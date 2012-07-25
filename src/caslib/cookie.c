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

#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>
#include "caslib/misc.h"
#include "caslib/cookie.h"

struct caslib_cookie_t {
  char *username;
  char *signature;
  uint64_t timestamp;
};

caslib_cookie_t *cookie_init(const caslib_t *cas, const caslib_rsp_t *rsp) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  caslib_cookie_t *cookie = caslib_alloca_alloc(cas, sizeof(caslib_cookie_t));
  CASLIB_GOTOIF(cookie==NULL, failure);

  size_t sz         = (size_t) caslib_rsp_auth_username(rsp, NULL, 0);
  cookie->username  = caslib_alloca_alloc(cas, sz);
  CASLIB_GOTOIF(cookie->username==NULL, failure);
  cookie->signature = NULL; // TODO:fixme
  cookie->timestamp = (uint64_t) tv.tv_sec;
  caslib_rsp_auth_username(rsp, cookie->username, sz);

  return(cookie);

 failure:
  caslib_alloca_destroy(cas, cookie);
  return(NULL);
}

void cookie_destroy(const caslib_t *cas, caslib_cookie_t *cookie) {
  if (cookie != NULL) {
    caslib_alloca_destroy(cas, cookie->signature);
    caslib_alloca_destroy(cas, cookie->username);
  }
  caslib_alloca_destroy(cas, cookie);
}
