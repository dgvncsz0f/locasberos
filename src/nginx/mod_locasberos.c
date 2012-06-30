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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_command_t locasberos_cmds[] = {
    {
      ngx_string("cas_enabled"),
      NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      0,
      0,
      NULL
    },
    ngx_null_command
};

static ngx_http_module_t locasberos_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
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
