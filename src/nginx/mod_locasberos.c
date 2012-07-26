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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
  ngx_flag_t locasberos_enabled;
  ngx_str_t cas_endpoint;
  ngx_str_t cas_service;
} ngx_http_locasberos_loc_conf_t;

static ngx_command_t locasberos_cmds[] = {
  {
    ngx_string("locasberos_enabled"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_locasberos_loc_conf_t, locasberos_enabled),
    NULL
  },
  {
    ngx_string("cas_endpoint"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_locasberos_loc_conf_t, cas_endpoint),
    NULL
  },
  {
    ngx_string("cas_service"),
    NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_locasberos_loc_conf_t, cas_service),
    NULL
  },
  ngx_null_command
};

static ngx_int_t ngx_http_locasberos_init(ngx_conf_t *conf) {
  return NGX_DECLINED;
};

static void * ngx_http_locasberos_create_loc_conf(ngx_conf_t *cf) {
  ngx_http_locasberos_loc_conf_t *conf;
  conf = ngx_palloc(cf->pool, sizeof(ngx_http_locasberos_loc_conf_t));
  if (conf == NULL) {
    return NGX_CONF_ERROR;
  }

  ngx_str_null(&conf->cas_endpoint);
  ngx_str_null(&conf->cas_service);
  conf->locasberos_enabled = NGX_CONF_UNSET;

  return conf;
};

static char * ngx_http_locasberos_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
  return NGX_CONF_OK;
};

static ngx_http_module_t locasberos_ctx = {
  NULL,
  ngx_http_locasberos_init,
  NULL,
  NULL,
  NULL,
  NULL,
  ngx_http_locasberos_create_loc_conf,
  ngx_http_locasberos_merge_loc_conf
};

ngx_module_t mod_locasberos = {
  NGX_MODULE_V1,
  &locasberos_ctx,
  locasberos_cmds,
  NGX_HTTP_MODULE,
  NULL, NULL, NULL, NULL, /* inits */
  NULL, NULL, NULL,  /* exits */
  NGX_MODULE_V1_PADDING
};
