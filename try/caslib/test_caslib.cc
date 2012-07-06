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

#include <UnitTest++.h>
#include "fixtures.hh"
extern "C" {
#include "caslib/misc.h"
#include "caslib/alloca.h"
#include "caslib/caslib.h"
}

size_t trace_alloc_calls;
size_t trace_destroy_calls;

static
void *trace_alloc(size_t s) {
  trace_alloc_calls += 1;
  return(malloc(s));
}

static
void trace_destroy(void *p) {
  trace_destroy_calls += 1;
  free(p);
}

TEST(caslib_init_with_should_cope_with_malloc_failure) {
  alloca_t alloca;
  alloca.alloc_f   = CASLIB_DUMMY_ALLOC;
  alloca.destroy_f = free;
  CHECK(NULL == caslib_init_with("", &alloca));
}

TEST(caslib_init_with_should_invoke_destroy_for_each_alloca) {
  trace_alloc_calls   = 0;
  trace_destroy_calls = 0;
  alloca_t alloca;
  alloca.alloc_f   = trace_alloc;
  alloca.destroy_f = trace_destroy;
  caslib_destroy(caslib_init_with("", &alloca));
  CHECK(trace_alloc_calls > 0);
  CHECK_EQUAL(trace_alloc_calls, trace_destroy_calls);
}

TEST(caslib_service_validate_should_not_return_NULL_for_auth_success_responses) {
  std::string url   = "file://" + fixture_path("/auth_success");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK(rsp != NULL);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_username_must_return_the_username_for_auth_success_responses) {
  char buffer[9];
  std::string url   = "file://" + fixture_path("/auth_success");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(9, caslib_rsp_auth_username(rsp, buffer, 9));
  CHECK_EQUAL(9, caslib_rsp_auth_username(rsp, NULL, 0));
  CHECK_EQUAL("username", buffer);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_username_must_return_minus_one_for_auth_success_responses) {
  char buffer[9];
  std::string url   = "file://" + fixture_path("/auth_failure");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(-1, caslib_rsp_auth_username(rsp, NULL, 0));
  CHECK_EQUAL(-1, caslib_rsp_auth_username(rsp, buffer, 9));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_username_must_return_the_strlen_for_auth_success_responses) {
  char buffer[9];
  std::string url   = "file://" + fixture_path("/auth_success");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(5, caslib_rsp_auth_username(rsp, buffer, 5));
  CHECK_EQUAL(9, caslib_rsp_auth_username(rsp, NULL, 0));
  CHECK_EQUAL("user", buffer);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_service_validate_should_not_return_NULL_for_auth_failure_responses) {
  std::string url   = "file://" + fixture_path("/auth_failure");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK(rsp != NULL);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_service_validate_should_return_NULL_for_garbage_responses) {
  std::string url   = "file://" + fixture_path("/auth_garbage");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK(rsp == NULL);
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_should_return_0_for_auth_success_responses) {
  std::string url   = "file://" + fixture_path("/auth_success");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(0, caslib_rsp_auth(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_should_return_1_for_auth_failure_responses) {
  std::string url   = "file://" + fixture_path("/auth_failure");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(1, caslib_rsp_auth(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_should_return_minus1_for_unknown_responses) {
  std::string url   = "file://" + fixture_path("/auth_bogus");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(-1, caslib_rsp_auth(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_should_return_true_for_auth_success_responses) {
  std::string url   = "file://" + fixture_path("/auth_success");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(true, caslib_rsp_auth_success(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}

TEST(caslib_rsp_auth_should_return_false_for_everything_else) {
  std::string url   = "file://" + fixture_path("/auth_failure");
  caslib_t *cas     = caslib_init(url.c_str());
  caslib_rsp_t *rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(false, caslib_rsp_auth_success(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);

  url = "file://" + fixture_path("/auth_bogus");
  cas = caslib_init(url.c_str());
  rsp = caslib_service_validate(cas, "service", "ticket", false);
  CHECK_EQUAL(false, caslib_rsp_auth_success(rsp));
  caslib_rsp_destroy(cas, rsp);
  caslib_destroy(cas);
}
