// vim: et:ts=8:sw=2:sts=2

// Copyright (c) 2012 dgvncsz0f
// Copyright (c) 2012 pothix
// Copyright (c) 2012 morellon
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
#include "apr_lib.h"

#include "mod_locasberos.h"
#include "caslib/misc.h"
#include "caslib/caslib.h"

#define ML_SELECT_IF_PTRDEF(a, b, m) (a->m==NULL ? b->m : a->m)
#define ML_SELECT_IF_INTDEF(a, b, m) (a->m==-1 ? b->m : a ->m)

#define ML_GET_PTRVAL(v, d) (v==NULL ? d : v)
#define ML_GET_INTVAL(v, d) (v==-1 ? d : v)

#define ML_LOGDEBUG_(s, fmt) ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, fmt)
#define ML_LOGDEBUG(s, fmt, ...) ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, fmt, __VA_ARGS__)
#define ML_LOGINFO_(s, fmt) ap_log_error(APLOG_MARK, APLOG_INFO, 0, s, fmt)
#define ML_LOGINFO(s, fmt, ...) ap_log_error(APLOG_MARK, APLOG_INFO, 0, s, fmt, __VA_ARGS__)
#define ML_LOGWARN_(s, fmt) ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, fmt)
#define ML_LOGWARN(s, fmt, ...) ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, fmt, __VA_ARGS__)
#define ML_LOGERROR_(s, fmt) ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, fmt)
#define ML_LOGERROR(s, fmt, ...) ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, fmt, __VA_ARGS__)

typedef struct {
  int enabled;
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

static
void __caslib_logger_func(void *data, const char *file, int line, const char *fmt, ...) {
  char msg[LOCASBEROS_MAXLOGSZ];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, LOCASBEROS_MAXLOGSZ, fmt, args);
  va_end(args);
  ML_LOGDEBUG_((server_rec *) data, msg);
}

static inline
void __locasberos_init_cfg(mod_locasberos_t *cfg) {
  cfg->enabled             = -1;
  cfg->cas_endpoint        = NULL;
  cfg->cas_service         = NULL;
  cfg->cas_renew           = 0;
  cfg->cas_gateway         = 0;
  cfg->cas_login_url       = "/login";
  cfg->cas_srvvalidate_url = "/serviceValidate";
  cfg->cookie_name         = "locasberos";
  cfg->cookie_timeout      = 60;
  cfg->cookie_path         = "/";
}

static
void *__locasberos_cfg_new_dir(apr_pool_t *pool, char *d) {
  mod_locasberos_t *cfg = apr_palloc(pool, sizeof(mod_locasberos_t));
  if (cfg != NULL)
    __locasberos_init_cfg(cfg);
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

static
char *__request_uri(request_rec *r) {
  return(apr_pstrcat(r->pool,
                     ap_http_scheme(r),
                     "://",
                     r->server->server_hostname,
                     apr_psprintf(r->pool, ":%u", r->server->port),
                     r->uri,
                     (r->args != NULL ? "?" : ""),
                     r->args,
                     NULL));
}

static
int __locasberos_authenticate(request_rec *r) {
  if (! ap_is_initial_req(r))
    return(DECLINED);

  mod_locasberos_t *cfg = (mod_locasberos_t *) ap_get_module_config(r->per_dir_config, &locasberos_module);
  const char *auth_type = ap_auth_type(r);
  apr_hash_t *args      = (r->args==NULL ? NULL : __parse_query_string(r->pool, r->args));
  const char *ticket    = (args==NULL ? NULL : apr_hash_get(args, "ticket", APR_HASH_KEY_STRING));
  const char *service   = ML_GET_PTRVAL(cfg->cas_service, __request_uri(r));
  caslib_t *cas         = NULL;
  caslib_rsp_t *rsp     = NULL;
  logger_t logger;
  int status            = DECLINED;
  int usersz            = -1;

  logger.debug_f = __caslib_logger_func;
  logger.info_f  = __caslib_logger_func;
  logger.warn_f  = __caslib_logger_func;
  logger.error_f = __caslib_logger_func;
  logger.data    = r->server;

  if (auth_type==NULL || apr_strnatcasecmp(auth_type, "locasberos")) {
    ML_LOGDEBUG(r->server, "ap_auth_type is not set to locasberos, declining: %s", r->uri);
    return(DECLINED);
  }

  if (! cfg->enabled) {
    ML_LOGDEBUG(r->server, "LocasberosEnabled off, declining: %s", r->uri);
    return(DECLINED);
  }

  if (cfg->cas_endpoint == NULL) {
    ML_LOGERROR(r->server, "CASEndpoint missing: %s", r->uri);
    return(HTTP_INTERNAL_SERVER_ERROR);
  }

  if (ticket != NULL) {
    cas = caslib_init(cfg->cas_endpoint);
    CASLIB_GOTOIF(cas==NULL, failure);
    caslib_setopt_logging(cas, &logger);
    rsp = caslib_service_validate(cas, service, ticket, cfg->cas_renew);
  } else {
    ML_LOGDEBUG(r->server, "no ticket found: %s", r->uri);
  }

  if (rsp==NULL || !caslib_rsp_auth_success(rsp)) {
    ML_LOGDEBUG(r->server, "cas authentication failure: %s", r->uri);
    status = HTTP_FORBIDDEN;
  } else {
    status  = OK;
    usersz  = caslib_rsp_auth_username(rsp, NULL, 0);
    r->user = apr_palloc(r->pool, usersz);
    caslib_rsp_auth_username(rsp, r->user, usersz);
    ML_LOGDEBUG(r->server, "cas authentication success: user=%s, %s", r->user, r->uri);
  }

  if (cas != NULL)
    caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);

  return(status);

 failure:
  ML_LOGERROR(r->server, "internal server error, aborting: %s", r->uri);
  return(HTTP_INTERNAL_SERVER_ERROR);
}

static
const command_rec locasberos_cmds[] = {
  AP_INIT_FLAG("LocasberosEnabled",
               ap_set_flag_slot,
               (void *) APR_OFFSETOF(mod_locasberos_t, enabled),
               OR_AUTHCFG,
               "Enable/Disable the module"),
  AP_INIT_TAKE1("CasEndpoint",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_endpoint),
                OR_AUTHCFG,
                "Define the CAS endpoint to use"),
  AP_INIT_TAKE1("CasLoginUrl",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_login_url),
                OR_AUTHCFG,
                "The CAS service validate URL to use"),
  AP_INIT_TAKE1("CasServiceValidateUrl",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_srvvalidate_url),
                OR_AUTHCFG,
                "The CAS service validate URL to use"),
  AP_INIT_TAKE1("CasCookieName",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_name),
                OR_AUTHCFG,
                "Define the cookie name to use"),
  AP_INIT_TAKE1("CasCookieTimeout",
                ap_set_int_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_timeout),
                OR_AUTHCFG,
                "Define the cookie expiration (in seconds)"),
  AP_INIT_TAKE1("CasCookiePath",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cookie_path),
                OR_AUTHCFG,
                "Define the cookie path to use when issuing/validating the cookie"),
  AP_INIT_TAKE1("CasService",
                ap_set_string_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_service),
                OR_AUTHCFG,
                "Define the CAS service to use when validating the service ticket"),
  AP_INIT_FLAG("CasRenew",
                ap_set_flag_slot,
                (void *) APR_OFFSETOF(mod_locasberos_t, cas_renew),
                OR_AUTHCFG,
                "Define whether or not setting the renew flag when redirecting to loginUrl or validating the service ticket"),
  AP_INIT_FLAG("CasGateway",
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
