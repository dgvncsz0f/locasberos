// vim: et:ts=8:sw=2:sts=2

// Copyright (c) 2012 dsouza
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

#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "apr_general.h"
#include "apr_strings.h"
#include "apr_tables.h"
#include "apr_hash.h"
#include "apr_lib.h"

#include "mod_locasberos.h"
#include "logloca.h"

#include "caslib/misc.h"
#include "caslib/caslib.h"

#include <ctype.h>
#include <stdbool.h>

#define ML_SELECT_IF_PTRDEF(a, b, m) (a->m==NULL ? b->m : a->m)
#define ML_SELECT_IF_INTDEF(a, b, m) (a->m==-1 ? b->m : a ->m)

#define ML_GET_PTRVAL(v, d) (v==NULL ? d : v)

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
} mod_locasberos_t;

static inline
void locasberos_init_cfg(mod_locasberos_t *cfg) {
  logloca_t *log;
  logloca_apache(log);
  log->info_f("OK");

  cfg->enabled             = -1;
  cfg->cas_endpoint        = NULL;
  cfg->cas_login_url       = NULL;
  cfg->cas_srvvalidate_url = NULL;
  cfg->cookie_name         = NULL;
  cfg->cookie_timeout      = -1;
  cfg->cookie_path         = NULL;
  cfg->cas_service         = NULL;
  cfg->cas_renew           = -1;
}

static
void *locasberos_cfg_new_srv(apr_pool_t *pool, server_rec *s) {
  mod_locasberos_t *cfg = apr_palloc(pool, sizeof(mod_locasberos_t));
  if (cfg != NULL)
    locasberos_init_cfg(cfg);
  return(cfg);
}

static
void *locasberos_cfg_new_dir(apr_pool_t *pool, char *d) {
  mod_locasberos_t *cfg = apr_palloc(pool, sizeof(mod_locasberos_t));
  if (cfg != NULL)
    locasberos_init_cfg(cfg);
  return(cfg);
}

static
void *locasberos_cfg_merge(apr_pool_t *pool, void *vbase, void *vadd) {
  mod_locasberos_t *base = (mod_locasberos_t *) vbase;
  mod_locasberos_t *add  = (mod_locasberos_t *) vadd;
  mod_locasberos_t *cfg  = (mod_locasberos_t *) apr_palloc(pool, sizeof(mod_locasberos_t));

  cfg->enabled             = ML_SELECT_IF_INTDEF(add, base, enabled);
  cfg->cas_endpoint        = ML_SELECT_IF_PTRDEF(add, base, cas_endpoint);
  cfg->cas_login_url       = ML_SELECT_IF_PTRDEF(add, base, cas_login_url);
  cfg->cas_srvvalidate_url = ML_SELECT_IF_PTRDEF(add, base, cas_srvvalidate_url);
  cfg->cookie_name         = ML_SELECT_IF_PTRDEF(add, base, cookie_name);
  cfg->cookie_path         = ML_SELECT_IF_PTRDEF(add, base, cookie_path);
  cfg->cookie_timeout      = ML_SELECT_IF_INTDEF(add, base, cookie_timeout);
  cfg->cas_service         = ML_SELECT_IF_PTRDEF(add, base, cas_service);
  cfg->cas_renew           = ML_SELECT_IF_INTDEF(add, base, cas_renew);

  return(cfg);
}

static
apr_hash_t *parse_query_string(apr_pool_t *pool, char *args) {
    apr_hash_t *query          = apr_hash_make(pool);
    CASLIB_GOTOIF(args==NULL, terminate);
    apr_array_header_t *values = NULL;
    char *val                  = NULL;
    char *ctx                  = NULL;
    char *token                = apr_strtok(args, "&", &ctx);

    while (token != NULL) {
      token = apr_strtok(NULL, "&", &ctx);
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

      // TODO: support multiple values?
      apr_hash_set(query, token, APR_HASH_KEY_STRING, apr_pstrdup(pool, val));
    }

 terminate:
    return(query);
}

static
int locasberos_authenticate(request_rec *r) {
  if (apr_strnatcasecmp(r->ap_auth_type, "locasberos"))
    return(DECLINED);

  mod_locasberos_t *cfg = (mod_locasberos_t *) r->per_dir_config;
  apr_hash_t *args      = parse_query_string(r->pool, r->args);
  char *ticket          = apr_hash_get(args, "ticket", APR_HASH_KEY_STRING);
  caslib_t *cas         = NULL;
  caslib_rsp_t *rsp     = NULL;
  int status            = DECLINED;

  cas = caslib_init(cfg->cas_endpoint);
  rsp = caslib_service_validate(cas, cfg->cas_service, ticket, false);
  if (rsp==NULL || !caslib_rsp_authentication_success(rsp))
    status = HTTP_FORBIDDEN;
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);

  return(status);
}

static
const command_rec locasberos_cmds[] = {
  // TODO: allow per-server config?
  AP_INIT_FLAG("CasEnabled",
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
                "Define whether or not setting the renew flag when validating the service ticket"),
  { NULL }
};

static
apr_status_t __caslib_global_destroy(void *_) {
  CASLIB_UNUSED(_);
  caslib_global_destroy();
  return(OK);
}

static void locasberos_hooks(apr_pool_t *pool) {
  caslib_global_init();
  apr_pool_cleanup_register(pool, NULL, __caslib_global_destroy, apr_pool_cleanup_null);
  ap_hook_check_user_id(locasberos_authenticate, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_DECLARE_DATA locasberos_module = {
  STANDARD20_MODULE_STUFF,
  locasberos_cfg_new_dir,
  locasberos_cfg_merge,
  NULL,
  NULL,
  locasberos_cmds,
  locasberos_hooks
};
