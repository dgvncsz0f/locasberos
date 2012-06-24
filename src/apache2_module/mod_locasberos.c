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
#include "apr_buckets.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "util_filter.h"
#include "http_request.h"

#include <ctype.h>

static const char locasberos_module_name[]="Locasberos";
module AP_MODULE_DECLARE_DATA locasberos_module;

typedef struct {
  int locasberos_enabled;
  unsigned int merged;
  unsigned int cas_cookie_timeout;
  char *cas_cookie_name;
  char *cas_endpoint;
  char *cas_service;
  char *cas_renew;
  apr_uri_t cas_login_url;
  apr_uri_t cas_service;
} locasberos_config;

static void *locasberos_create_server_config(apr_pool_t *p, server_rec *s) {
  locasberos_config *ptr_config=apr_pcalloc(p, sizeof *ptr_config);
  ptr_config->locasberos_enabled = 0;
  ptr_config->cas_cookie_timeout = 3600;
  ptr_config->cas_cookie_name = "MOD_LOCASBEROS";
  ptr_config->cas_endpoint = NULL;
  ptr_config->cas_service = NULL;
  ptr_config->cas_renew = NULL;
  ptr_config->cas_login_url = NULL;
  ptr_config->cas_service = NULL;

  return ptr_config;
}

static void locasberos_insert_filter(request_rec *r) {
  locasberos_config *ptr_config=ap_get_module_config(r->server->module_config,
      &locasberos_module);

  if(!ptr_config->locasberos_enabled)
    return;

  ap_add_input_filter(locasberos_module_name, NULL, r, r->connection);
}

static apr_status_t locasberos_input_filter(ap_filter_t *f, apr_bucket_brigade *pbbIn) {
  request_rec *r = f->r;
  conn_rec *c = r->connection;
  apr_bucket *pbktIn;
  apr_bucket_brigade *pbbOut;

  pbbOut=apr_brigade_create(r->pool, c->bucket_alloc);
  for (pbktIn = APR_BRIGADE_FIRST(pbbIn); pbktIn != APR_BRIGADE_SENTINEL(pbbIn); pbktIn = APR_BUCKET_NEXT(pbktIn)) {
    const char *data;
    apr_size_t len;
    char *buf;
    apr_size_t n;
    apr_bucket *pbktOut;

    if(APR_BUCKET_IS_EOS(pbktIn)) {
      apr_bucket *pbktEOS=apr_bucket_eos_create(c->bucket_alloc);
      APR_BRIGADE_INSERT_TAIL(pbbOut, pbktEOS);
      continue;
    }

    /* read */
    apr_bucket_read(pbktIn, &data, &len, APR_BLOCK_READ);

    /* write */
    buf = apr_bucket_alloc(len, c->bucket_alloc);
    for(n=0 ; n < len ; ++n)
      buf[n] = apr_toupper(data[n]);

    pbktOut = apr_bucket_heap_create(buf, len, apr_bucket_free,
        c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(pbbOut, pbktOut);
  }

  apr_brigade_cleanup(pbbIn);
  return ap_pass_brigade(f->next, pbbOut);
}

static const char *locasberos_enable(cmd_parms *cmd, void *dummy, int arg) {
  locasberos_config *ptr_config=ap_get_module_config(cmd->server->module_config, &locasberos_module);
  ptr_config->locasberos_enabled=arg;

  return NULL;
}

static const command_rec locasberos_cmds[] = {
  AP_INIT_FLAG(locasberos_module_name, locasberos_enable, NULL, RSRC_CONF, "Validate CAS authentication for this host"),
  { NULL }
};

static void locasberos_register_hooks(apr_pool_t *p) {
  ap_hook_insert_filter(locasberos_insert_filter, NULL, NULL, APR_HOOK_MIDDLE);
  ap_register_input_filter(locasberos_module_name, locasberos_input_filter, NULL, AP_FTYPE_RESOURCE);
}

AP_DECLARE_MODULE(locasberos) = {
  STANDARD20_MODULE_STUFF,
  NULL,
  NULL,
  LocasberosCreateServerConfig,
  NULL,
  locasberos_cmds,
  locasberos_register_hooks
};
