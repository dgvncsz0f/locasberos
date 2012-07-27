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

#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <sys/time.h>
#include "caslib/misc.h"
#include "caslib/cookie.h"

#define CASLIB_COOKIE_VER 1

struct caslib_cookie_t {
  char *username;
  uint64_t timestamp;
};

static inline
int __serialize_uint8(uint8_t *out, int out_sz, uint8_t n) {
  if (out_sz < 1)
    return(-1);
  out[0] = n;
  return(out_sz - 1);
}

static inline
int __serialize_uint16(uint8_t *out, int out_sz, uint16_t n) {
  out_sz = __serialize_uint8(out, out_sz, (uint8_t) (n & 0xFF));
  out_sz = __serialize_uint8(out+1, out_sz, (uint8_t) ((n >> 8) & 0xFF));
  return(out_sz);
}

static inline
int __serialize_uint32(uint8_t *out, int out_sz, uint32_t n) {
  out_sz = __serialize_uint16(out, out_sz, (uint16_t) (n & 0xFFFF));
  out_sz = __serialize_uint16(out+2, out_sz, (uint16_t) ((n >> 16) & 0xFFFF));
  return(out_sz);
}

static inline
int __serialize_uint64(uint8_t *out, int out_sz, uint64_t n) {
  out_sz = __serialize_uint32(out, out_sz, (uint32_t) (n & 0xFFFFFFFF));
  out_sz = __serialize_uint32(out+4, out_sz, (uint32_t) ((n >> 32) & 0xFFFFFFFF));
  return(out_sz);
}

static inline
int __serialize_string(uint8_t *out, int out_sz, const char *s) {
  int l = (int) (strlen(s) + 1);
  if (out_sz < l)
    return(-1);
  strcpy((char *) out, s);
  return(out_sz - l);
}

caslib_cookie_t *caslib_cookie_init(const caslib_t *cas, const caslib_rsp_t *rsp) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  caslib_cookie_t *cookie = caslib_alloca_alloc(cas, sizeof(caslib_cookie_t));
  CASLIB_GOTOIF(cookie==NULL, failure);

  size_t sz         = (size_t) caslib_rsp_auth_username(rsp, NULL, 0);
  cookie->username  = caslib_alloca_alloc(cas, sz);
  CASLIB_GOTOIF(cookie->username==NULL, failure);
  cookie->timestamp = (uint64_t) tv.tv_sec;
  caslib_rsp_auth_username(rsp, cookie->username, sz);

  return(cookie);

 failure:
  caslib_alloca_destroy(cas, cookie);
  return(NULL);
}

int caslib_cookie_serialize(const caslib_cookie_t *c, const char *sec, uint8_t *o, size_t s) {
  CASLIB_UNUSED(sec);

  char username[COOKIE_USR_MAXSZ];
  size_t ulen = strlen(c->username) + 1;
  int usz     = CASLIB_MIN(COOKIE_USR_MAXSZ - 1, (int) ulen);
  int sz      = 1 + 1 + usz + 8;
  int tmp;

  if (o == NULL)
    return(sz);
  else if (((int) s) >= sz) {
    strncpy(username, c->username, (size_t) (usz - 1));
    username[usz] = '\0';
    tmp = __serialize_uint8(o, (int) s, CASLIB_COOKIE_VER); // + 1
    tmp = __serialize_string(o+1, tmp, username);           // + 1 + usz
    tmp = __serialize_uint64(o+1+usz, tmp, c->timestamp);   // + 8
    tmp = ((int) s) - tmp;
    assert(sz == tmp);
    return(sz);
  } else {
    return(sz);
  }
}

void caslib_cookie_destroy(const caslib_t *cas, caslib_cookie_t *cookie) {
  if (cookie != NULL) {
    caslib_alloca_destroy(cas, cookie->username);
  }
  caslib_alloca_destroy(cas, cookie);
}
