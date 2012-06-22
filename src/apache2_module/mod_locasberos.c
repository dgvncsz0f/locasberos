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

static const char s_szLocasberosName[]="Locasberos";
module AP_MODULE_DECLARE_DATA locasberos_module;

typedef struct {
  int bEnabled;
} LocasberosConfig;

static void *LocasberosCreateServerConfig(apr_pool_t *p,server_rec *s) {
  LocasberosConfig *pConfig=apr_pcalloc(p,sizeof *pConfig);
  pConfig->bEnabled=0;

  return pConfig;
}

static void LocasberosInsertFilter(request_rec *r) {
  LocasberosConfig *pConfig=ap_get_module_config(r->server->module_config,
      &locasberos_module);

  if(!pConfig->bEnabled)
    return;

  ap_add_input_filter(s_szLocasberosName,NULL,r,r->connection);
}

static apr_status_t LocasberosOutFilter(ap_filter_t *f, apr_bucket_brigade *pbbIn) {
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

    if(APR_BUCKET_IS_EOS(pbktIn))
    {
      apr_bucket *pbktEOS=apr_bucket_eos_create(c->bucket_alloc);
      APR_BRIGADE_INSERT_TAIL(pbbOut,pbktEOS);
      continue;
    }

    /* read */
    apr_bucket_read(pbktIn,&data,&len,APR_BLOCK_READ);

    /* write */
    buf = apr_bucket_alloc(len, c->bucket_alloc);
    for(n=0 ; n < len ; ++n)
      buf[n] = apr_toupper(data[n]);

    pbktOut = apr_bucket_heap_create(buf, len, apr_bucket_free,
        c->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(pbbOut,pbktOut);
  }

  /* Q: is there any advantage to passing a brigade for each bucket?
   * A: obviously, it can cut down server resource consumption, if this
   * experimental module was fed a file of 4MB, it would be using 8MB for
   * the 'read' buckets and the 'write' buckets.
   *
   * Note it is more efficient to consume (destroy) each bucket as it's
   * processed above than to do a single cleanup down here.  In any case,
   * don't let our caller pass the same buckets to us, twice;
   */
  apr_brigade_cleanup(pbbIn);
  return ap_pass_brigade(f->next,pbbOut);
}

static const char *LocasberosEnable(cmd_parms *cmd, void *dummy, int arg) {
  LocasberosConfig *pConfig=ap_get_module_config(cmd->server->module_config,
      &locasberos_module);
  pConfig->bEnabled=arg;

  return NULL;
}

static const command_rec LocasberosCmds[] = {
  AP_INIT_FLAG("Locasberos", LocasberosEnable, NULL, RSRC_CONF,
      "Validate CAS authentication for this host"),
  { NULL }
};

static void LocasberosRegisterHooks(apr_pool_t *p) {
  ap_hook_insert_filter(LocasberosInsertFilter,NULL,NULL,APR_HOOK_MIDDLE);
  ap_register_input_filter(s_szLocasberosName,LocasberosOutFilter,NULL, AP_FTYPE_RESOURCE);
}

AP_DECLARE_MODULE(locasberos) = {
  STANDARD20_MODULE_STUFF,
  NULL,
  NULL,
  LocasberosCreateServerConfig,
  NULL,
  LocasberosCmds,
  LocasberosRegisterHooks
};
