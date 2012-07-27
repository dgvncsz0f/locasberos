// vim: et:ts=8:sw=2:sts=2

// Copyright (c) dsouza
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

#include <UnitTest++.h>
#include <string>
#include "fixtures.hh"
extern "C" {
#include "caslib/misc.h"
#include "caslib/alloca.h"
#include "caslib/caslib.h"
#include "caslib/cookie.h"
}

#define test_cookie_cas_endpoint ("file://" + fixture_path("/auth_success"))

static inline
caslib_rsp_t *__cas_response(const caslib_t *cas) {
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  return(rsp);
}

TEST(cookie_username_must_return_the_same_username_that_is_on_response) {
  caslib_t *cas           = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp       = __cas_response(cas);
  caslib_cookie_t *cookie = caslib_cookie_init(cas, rsp);
  char username[1024];

  caslib_rsp_auth_username(rsp, username, 1024);
  CHECK_EQUAL(username, caslib_cookie_username(cookie));

  caslib_cookie_destroy(cas, cookie);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(cookie_serialize_should_allow_NULL_pointers) {
  caslib_t *cas           = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp       = __cas_response(cas);
  caslib_cookie_t *cookie = caslib_cookie_init(cas, rsp);

  int rc = caslib_cookie_serialize(cookie, "secret", NULL, 0);
  CHECK(rc > 0);

  caslib_cookie_destroy(cas, cookie);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(cookie_serialize_should_fail_if_buffer_is_not_large_enough) {
  caslib_t *cas           = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp       = __cas_response(cas);
  caslib_cookie_t *cookie = caslib_cookie_init(cas, rsp);
  uint8_t data[1024];

  int rc = caslib_cookie_serialize(cookie, "secret", NULL, 0);
  rc = caslib_cookie_serialize(cookie, "secret", data, rc-1);
  CHECK_EQUAL(-1, rc);

  caslib_cookie_destroy(cas, cookie);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(cookie_serialize_unserialize_should_fail_if_buffer_is_not_large_enough) {
  caslib_t *cas            = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp        = __cas_response(cas);
  caslib_cookie_t *cookie0 = caslib_cookie_init(cas, rsp);
  caslib_cookie_t *cookie1 = NULL;
  uint8_t data[1024];
  int rc;

  rc      = caslib_cookie_serialize(cookie0, "secret", data, 1024);
  cookie1 = caslib_cookie_unserialize(cas, "secret", data, rc-1);

  CHECK(NULL == cookie1);

  caslib_cookie_destroy(cas, cookie0);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(cookie_serialize_unserialize_should_be_noop) {
  caslib_t *cas            = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp        = __cas_response(cas);
  caslib_cookie_t *cookie0 = caslib_cookie_init(cas, rsp);
  caslib_cookie_t *cookie1 = NULL;
  uint8_t data[1024];

  caslib_cookie_serialize(cookie0, "secret", data, 1024);
  cookie1 = caslib_cookie_unserialize(cas, "secret", data, 1024);

  CHECK_EQUAL(caslib_cookie_timestamp(cookie0), caslib_cookie_timestamp(cookie1));
  CHECK_EQUAL(caslib_cookie_username(cookie0), caslib_cookie_username(cookie1));

  caslib_cookie_destroy(cas, cookie0);
  caslib_cookie_destroy(cas, cookie1);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(cookie_unserialize_should_fail_if_secret_is_different) {
  caslib_t *cas            = caslib_init(test_cookie_cas_endpoint.c_str());
  caslib_rsp_t *rsp        = __cas_response(cas);
  caslib_cookie_t *cookie0 = caslib_cookie_init(cas, rsp);
  caslib_cookie_t *cookie1 = NULL;
  uint8_t data[1024];

  caslib_cookie_serialize(cookie0, "secretA", data, 1024);
  cookie1 = caslib_cookie_unserialize(cas, "secretO", data, 1024);

  CHECK(NULL != cookie0);
  CHECK(NULL == cookie1);

  caslib_cookie_destroy(cas, cookie0);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}
