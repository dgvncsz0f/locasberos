// vim: et:ts=8:sw=2:sts=2

// Copyright (c) 2012 dgvncsz0f
// Copyright (c) 2012 pothix
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the <ORGANIZATION> nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <ctype.h>
#include <stdbool.h>

#include "ap_config.h"
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_protocol.h"
#include "http_core.h"
#include "http_log.h"

#include "apr_general.h"
#include "apr_strings.h"
#include "apr_pools.h"
#include "apr_hash.h"
#include "apr_tables.h"
#include "apr_lib.h"
#include "apr_base64.h"

#include "mod_authn_locasberos.h"
#include "caslib/misc.h"
#include "caslib/caslib.h"
#include "caslib/cookie.h"

#define ML_SELECT_IF_PTRDEF(a, b, m) (a->m==NULL ? b->m : a->m)
#define ML_SELECT_IF_INTDEF(a, b, m) (a->m==-1 ? b->m : a ->m)

#define ML_GET_PTRVAL(v, d) (v==NULL ? d : v)
#define ML_GET_INTVAL(v, d) (v==-1 ? d : v)

#define ML_LOGDEBUG_(s, fmt) ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, s, fmt)
#define ML_LOGDEBUG(s, fmt, ...) ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, s, fmt, __VA_ARGS__)
#define ML_LOGINFO_(s, fmt) ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, s, fmt)
#define ML_LOGINFO(s, fmt, ...) ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, s, fmt, __VA_ARGS__)
#define ML_LOGWARN_(s, fmt) ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, s, fmt)
#define ML_LOGWARN(s, fmt, ...) ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, s, fmt, __VA_ARGS__)
#define ML_LOGERROR_(s, fmt) ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, s, fmt)
#define ML_LOGERROR(s, fmt, ...) ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, s, fmt, __VA_ARGS__)

typedef struct {
  int enabled;
  int authoritative;
  char *cas_endpoint;
  char *cas_login_url;
  char *cas_srvvalidate_url;
  char *cookie_name;
  unsigned int cookie_timeout;
  char *cookie_path;
  char *cas_service;
  int cas_renew;
  int cas_gateway;
} mod_locasberos_t;

#ifdef LOCASBEROS_NOTES_COOKIE_KEY
#undef LOCASBEROS_NOTES_COOKIE_KEY
#endif

#define LOCASBEROS_NOTES_COOKIE_KEY "locasbers_cookie"

static
void __caslib_logger_func(void *data, const char *file, int line, const char *fmt, ...) {
  CASLIB_UNUSED(file);
  CASLIB_UNUSED(line);

  char msg[LOCASBEROS_MAXLOGSZ];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, LOCASBEROS_MAXLOGSZ, fmt, args);
  va_end(args);
  ML_LOGDEBUG_((request_rec *) data, msg);
}

static inline
void __locasberos_init_cfg(apr_pool_t *pool, mod_locasberos_t *cfg) {
  cfg->enabled             = -1;
  cfg->authoritative       = 1;
  cfg->cas_endpoint        = NULL;
  cfg->cas_service         = NULL;
  cfg->cas_renew           = 0;
  cfg->cas_gateway         = 0;
  cfg->cas_login_url       = apr_pstrdup(pool, "/login");
  cfg->cas_srvvalidate_url = apr_pstrdup(pool, "/serviceValidate");
  cfg->cookie_name         = apr_pstrdup(pool, "locasberos");
  cfg->cookie_timeout      = 28800;
  cfg->cookie_path         = apr_pstrdup(pool, "/");
}

static
void *__locasberos_cfg_new_dir(apr_pool_t *pool, char *d) {
  CASLIB_UNUSED(d);
  mod_locasberos_t *cfg = apr_palloc(pool, sizeof(mod_locasberos_t));
  if (cfg != NULL)
    __locasberos_init_cfg(pool, cfg);
  return(cfg);
}

static
apr_hash_t *__parse_query_string(apr_pool_t *pool, const char *args0) {
    apr_hash_t *query = apr_hash_make(pool);
    CASLIB_GOTOIF(args0==NULL, terminate);

    char *args  = apr_pstrdup(pool, args0);
    char *val   = NULL;
    char *ctx   = NULL;
    char *token = apr_strtok(args, "&", &ctx);

    for (; token!=NULL; token=apr_strtok(NULL, "&", &ctx)) {
      val   = strchr(token, '=');
      if (val != NULL) {
        *val  = '\0';
        val  += 1;
        ap_unescape_url(token);
        ap_unescape_url(val);
      } else {
        val = "";
        ap_unescape_url(token);
      }

      apr_hash_set(query, token, APR_HASH_KEY_STRING, apr_pstrdup(pool, val));
    }

 terminate:
    return(query);
}

static inline
char *__destroy_ticket_param(char *uri) {
  size_t len;
  char *sep    = NULL;
  char *ticket = strstr(uri, "?ticket=");
  if (ticket == NULL)
    ticket = strstr(uri, "&ticket=");
  if (ticket == NULL)
    return(uri);

  sep = strchr(ticket+1, '&');
  len = (sep == NULL ? 0 : strlen(sep));
  if (len > 0)
    memmove(ticket+1, sep+1, len);
  ticket[len] = '\0';
  return(uri);
}

static
char *__request_uri(request_rec *r) {
  return(apr_pstrcat(r->pool,
                     ap_http_scheme(r),
                     "://",
                     r->server->server_hostname,
                     apr_psprintf(r->pool, ":%u", r->server->port),
                     r->uri,
                     (r->args == NULL ? NULL : "?"),
                     r->args,
                     NULL));
}

static inline
int __handle_auth_failure(request_rec *r) {
  mod_locasberos_t *cfg = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  if (cfg->authoritative) {
    return(HTTP_FORBIDDEN);
  } else {
    ML_LOGDEBUG(r, "we are not the authoritative one, declining: %s", r->uri);
    return(DECLINED);
  }
}

static inline
const char *__find_cas_cookie_in_notes(request_rec *r) {
  const char *cookie = NULL;
  if (r->main != NULL)
    cookie = apr_table_get(r->main->notes, LOCASBEROS_NOTES_COOKIE_KEY);
  if (cookie == NULL)
    cookie = apr_table_get(r->notes, LOCASBEROS_NOTES_COOKIE_KEY);
  return(cookie);
}

static
const char *__find_cas_cookie(request_rec *r, const char *name) {
  const char *cookie = __find_cas_cookie_in_notes(r);
  CASLIB_GOTOIF(cookie!=NULL, terminate);
  char *cookies0 = (char *) apr_table_get(r->headers_in, "Cookie");
  char *cookies  = apr_pstrdup(r->pool, (cookies0!=NULL ? cookies0 : ""));
  CASLIB_GOTOIF(cookies==NULL, terminate);
  char *ctx      = NULL;
  char *val      = NULL;
  char *token    = apr_strtok(cookies, ";", &ctx);

  for (; token!=NULL; token=apr_strtok(NULL, "&", &ctx)) {
    val = strchr(token, '=');
    if (val != NULL) {
      *val  = '\0';
      val  += 1;
    }

    if (apr_strnatcasecmp(token, name) == 0) {
      cookie = val;
      break;
    }
  }

 terminate:
  return(cookie);
}

static
caslib_cookie_t *__decode_cookie(request_rec *r, const caslib_t *cas, const char *e_cookie) {
  char secret[]            = "TODO:fixme";
  caslib_cookie_t *cookie  = NULL;
  int datasz               = apr_base64_decode_len(e_cookie);
  uint8_t *data            = apr_palloc(r->pool, datasz);
  CASLIB_GOTOIF(data==NULL, failure);
  
  apr_base64_decode_binary(data, e_cookie);
  cookie = caslib_cookie_unserialize(cas, secret, data, datasz);

 failure:
  return(cookie);
}

static
char *__encode_cookie(request_rec *r, const caslib_t *cas, const caslib_rsp_t *rsp) {
  char secret[]           = "TODO:fixme";
  char *b64cookie         = NULL;
  caslib_cookie_t *cookie = caslib_cookie_init(cas, rsp);
  CASLIB_GOTOIF(cookie==NULL, failure);
  int bincookiesz         = caslib_cookie_serialize(cookie, secret, NULL, 0);
  uint8_t *bincookie      = apr_palloc(r->pool, bincookiesz);
  CASLIB_GOTOIF(bincookie==NULL, failure);

  b64cookie = apr_palloc(r->pool, apr_base64_encode_len(bincookiesz));
  CASLIB_GOTOIF(b64cookie==NULL, failure);

  caslib_cookie_serialize(cookie, secret, bincookie, bincookiesz);
  apr_base64_encode_binary(b64cookie, bincookie, bincookiesz);

 failure:
  caslib_cookie_destroy(cas, cookie);
  return(b64cookie);
}

static
int __handle_auth_success(request_rec *r, const caslib_t *cas, const caslib_rsp_t *rsp) {
  int status              = HTTP_INTERNAL_SERVER_ERROR;
  mod_locasberos_t *cfg   = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  char *headerstr         = NULL;
  char *cookie            = __encode_cookie(r, cas, rsp);
  CASLIB_GOTOIF(cookie==NULL, failure);

  headerstr = apr_psprintf(r->pool, 
                           "%s=%s; Path=%s; HttpOnly",
                           cfg->cookie_name,
                           cookie,
                           cfg->cookie_path);

  ML_LOGDEBUG(r, "emitting new locasberos cookie: %s", r->uri);
  apr_table_add(r->headers_out, "Set-Cookie", headerstr);
  apr_table_add(r->err_headers_out, "Set-Cookie", headerstr);
  apr_table_set(r->notes, LOCASBEROS_NOTES_COOKIE_KEY, cookie);
  status = OK;

 failure:
  return(status);
}

static
int __perform_cookie_authentication(request_rec *r, const caslib_t *cas) {
  int status              = DECLINED;
  mod_locasberos_t *cfg   = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  const char *e_cookie    = NULL;
  caslib_cookie_t *cookie = NULL;

  e_cookie = __find_cas_cookie(r, cfg->cookie_name);
  CASLIB_GOTOIF(e_cookie==NULL, failure);
  cookie = __decode_cookie(r, cas, e_cookie);
  CASLIB_GOTOIF(cookie==NULL, failure);

  if (caslib_cookie_check_timestamp(cookie, cfg->cookie_timeout)) {
    r->user = apr_pstrdup(r->pool, caslib_cookie_username(cookie));
    status  = OK;
    ML_LOGINFO(r, "cas authentication success (cookie): user=%s, %s", r->user, r->uri);
  }

  if (status != OK) {
    ML_LOGINFO(r, "cas authentication failure (cookie): %s", r->uri);
  }

 failure:
  caslib_cookie_destroy(cas, cookie);
  return(status);
}

static
int __login_redirect(request_rec *r, const caslib_t *cas) {
  int status            = HTTP_INTERNAL_SERVER_ERROR;
  mod_locasberos_t *cfg = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  const char *service   = ML_GET_PTRVAL(cfg->cas_service, __request_uri(r));
  int sz                = caslib_login_url(cas, NULL, 0, service, cfg->cas_renew, cfg->cas_gateway);
  CASLIB_GOTOIF(sz<0, failure);
  char *login_url       = apr_palloc(r->pool, sz);
  CASLIB_GOTOIF(login_url==NULL, failure);

  caslib_login_url(cas, login_url, sz, service, cfg->cas_renew, cfg->cas_gateway);
  apr_table_add(r->headers_out, "Location", login_url);
  status = HTTP_MOVED_TEMPORARILY;
  ML_LOGDEBUG(r, "sending redirect: %s: %s", login_url, service);

 failure:
  return(status);
}

static
int __perform_cas_authentication(request_rec *r, const caslib_t *cas) {
  mod_locasberos_t *cfg = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  apr_hash_t *args      = (r->args==NULL ? NULL : __parse_query_string(r->pool, r->args));
  const char *ticket    = (args==NULL ? NULL : apr_hash_get(args, "ticket", APR_HASH_KEY_STRING));
  const char *service   = ML_GET_PTRVAL(cfg->cas_service, __request_uri(r));
  char *rwservice       = NULL;
  caslib_rsp_t *rsp     = NULL;
  int status            = HTTP_INTERNAL_SERVER_ERROR;
  int usersz            = -1;

  if (ticket != NULL) {
    rwservice = apr_pstrdup(r->pool, service);
    __destroy_ticket_param(rwservice);
    rsp = caslib_service_validate(cas, rwservice, ticket, cfg->cas_renew);
    CASLIB_GOTOIF(rsp==NULL, failure);
  } else {
    ML_LOGDEBUG(r, "no ticket found: %s", r->uri);
  }

  if (rsp==NULL) {
    ML_LOGINFO(r, "no ticket found, redirecting to login url: %s", r->uri);
    status = __login_redirect(r, cas);
  } else if (caslib_rsp_auth_success(rsp)) {
    status  = __handle_auth_success(r, cas, rsp);
    usersz  = caslib_rsp_auth_username(rsp, NULL, 0);
    r->user = apr_palloc(r->pool, usersz);
    caslib_rsp_auth_username(rsp, r->user, usersz);
    ML_LOGINFO(r, "cas authentication success (ticket): user=%s, %s", r->user, r->uri);
  } else {
    ML_LOGINFO(r, "cas authentication failure (ticket): %s", r->uri);
    status = __handle_auth_failure(r);
  }

 failure:
  caslib_rsp_destroy(cas, rsp);

  return(status);
}

static
int __locasberos_authenticate(request_rec *r) {
  mod_locasberos_t *cfg   = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  const char *auth_type   = ap_auth_type(r);
  caslib_t *cas           = NULL;
  int status              = DECLINED;
  logger_t logger;

  logger.debug_f = __caslib_logger_func;
  logger.info_f  = __caslib_logger_func;
  logger.warn_f  = __caslib_logger_func;
  logger.error_f = __caslib_logger_func;
  logger.data    = r;

  if (auth_type==NULL || apr_strnatcasecmp(auth_type, "locasberos")) {
    ML_LOGDEBUG(r, "ap_auth_type is not set to locasberos, declining: %s", r->uri);
    return(DECLINED);
  }

  if (! cfg->enabled) {
    ML_LOGDEBUG(r, "LocasberosEnabled off, declining: %s", r->uri);
    return(DECLINED);
  }

  if (cfg->cas_endpoint == NULL) {
    ML_LOGERROR(r, "LocasberosEndpoint missing: %s", r->uri);
    return(HTTP_INTERNAL_SERVER_ERROR);
  }

  cas = caslib_init(cfg->cas_endpoint);
  CASLIB_GOTOIF(cas==NULL, failure);
  caslib_setopt_logging(cas, &logger);

  status = __perform_cookie_authentication(r, cas);
  if (status == DECLINED)
    status = __perform_cas_authentication(r, cas);

  caslib_destroy(cas);
  return(status);

 failure:
  return(HTTP_INTERNAL_SERVER_ERROR);
}

static
const command_rec locasberos_cmds[] = {
  AP_INIT_FLAG("LocasberosEnabled",
               ap_set_flag_slot,
               (void *) APR_OFFSETOF(mod_locasberos_t, enabled),
               OR_AUTHCFG,
               "Enable/Disable the module"),
  AP_INIT_FLAG("LocasberosAuthoritative",
               ap_set_flag_slot,
               (void *) APR_OFFSETOF(mod_locasberos_t, enabled),
               OR_AUTHCFG,
               "Authoritative mode will deny access when authentication fails"),
  AP_INIT_TAKE1("LocasberosEndpoint",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_endpoint),
                OR_AUTHCFG,
                "Define the CAS endpoint to use"),
  AP_INIT_TAKE1("LocasberosLoginUrl",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, authoritative),
                OR_AUTHCFG,
                "The CAS login URL to use"),
  AP_INIT_TAKE1("LocasberosServiceValidateUrl",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_srvvalidate_url),
                OR_AUTHCFG,
                "The CAS service validate URL to use"),
  AP_INIT_TAKE1("LocasberosCookieName",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_name),
                OR_AUTHCFG,
                "Define the cookie name to use"),
  AP_INIT_TAKE1("LocasberosCookieTimeout",
                ap_set_int_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_timeout),
                OR_AUTHCFG,
                "Define the cookie expiration (in seconds)"),
  AP_INIT_TAKE1("LocasberosCookiePath",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_path),
                OR_AUTHCFG,
                "Define the cookie path to use when issuing/validating the cookie"),
  AP_INIT_TAKE1("LocasberosService",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_service),
                OR_AUTHCFG,
                "Define the CAS service to use when validating the service ticket"),
  AP_INIT_FLAG("LocasberosRenew",
                ap_set_flag_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_renew),
                OR_AUTHCFG,
                "Define whether or not setting the renew flag when redirecting to loginUrl or validating the service ticket"),
  AP_INIT_FLAG("LocasberosGateway",
               ap_set_flag_slot,
               (void *) APR_OFFSETOF(mod_locasberos_t, cas_gateway),
               OR_AUTHCFG,
               "Define whether or not setting the gateway flag when redirecting to loginUrl"),
  { NULL }
};

static
apr_status_t __caslib_global_destroy(void *_) {
  CASLIB_UNUSED(_);
  caslib_global_destroy();
  return(OK);
}

static
void __locasberos_hooks(apr_pool_t *pool) {
  caslib_global_init();
  apr_pool_cleanup_register(pool, NULL, __caslib_global_destroy, apr_pool_cleanup_null);
  ap_hook_check_user_id(__locasberos_authenticate, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_DECLARE_DATA locasberos_module = {
  STANDARD20_MODULE_STUFF,
  __locasberos_cfg_new_dir,
  NULL,
  NULL,
  NULL,
  locasberos_cmds,
  __locasberos_hooks
};
