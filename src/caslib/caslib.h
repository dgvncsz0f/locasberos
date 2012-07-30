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

#ifndef __LOCASBEROS_CASLIB__
#define __LOCASBEROS_CASLIB__

#include <stdlib.h>
#include <stdbool.h>
#include "caslib/alloca.h"
#include "caslib/logging.h"

typedef struct caslib_t caslib_t;
typedef struct caslib_rsp_t caslib_rsp_t;

/*! Must be called once in the program
 * \return 0=Ok.
 */
int caslib_global_init();

/*! Should be called once per caslib_global_init.
 */
void caslib_global_destroy();

/*! Initializes a new caslib_t using a given endpoint. This is the
 *  dual function of caslib_destroy.
 *
 *  This in fact a wrapper around caslib_init2 using system's
 *  standard memory allocation.
 */
caslib_t *caslib_init(const char *endpoint);

/*! Initializes a new caslib_t using a given endpoint and a custom
 *  memory management. This is the dual function of caslib_destroy.
 */
caslib_t *caslib_init_with(const char *endpoint, const alloca_t *alloca);

/*! Defines the logger to use.
 */
void caslib_setopt_logging(caslib_t *cas, const logger_t *logger);

/*! Build up the URL to authenticate the user.
 *
 * \param out The variable that will store the output (this might be NULL);
 * 
 * \param s The size of the out variable;
 */
int caslib_login_url(const caslib_t *cas, char *out, size_t s, const char *service, bool renew, bool gateway);

/*! Checks the validity of a service ticket [1].
 *
 * \param service The identifier of the service for which the ticket
 * was issued.
 *
 * \param ticket The service ticket issued by /login.
 *
 * \param renew If this parameter is true, ticket validation will only
 *              succeed if the service ticket was issued from the
 *              presentation of the user's primary credentials. It
 *              will fail if the ticket was issued from a single
 *              sign-on session.
 *
 * [1] http://www.jasig.org/cas/protocol, Section 2.5 (Wed, 20 Jun 2012)
 */
caslib_rsp_t *caslib_service_validate(const caslib_t *, const char *service, const char *ticket, bool renew);

/*! The same as caslib_service_validate but allows you to provide a
 *  proxy callback url.
 *
 * \param service The identifier of the service for which the ticket
 * was issued.
 *
 * \param ticket The service ticket issued by /login.
 *
 * \param renew If this parameter is true, ticket validation will only
 *              succeed if the service ticket was issued from the
 *              presentation of the user's primary credentials. It
 *              will fail if the ticket was issued from a single
 *              sign-on session.
 *
 * \param pgtUrl The URL of the proxy callback.
 *
 * \return The response of the validation request. Remember to free
 *         memory using caslib_rsp_destroy, in order to make sure the
 *         proper function gets called.
 */
caslib_rsp_t *caslib_service_validate_pgt(const char *service, const char *ticket, bool renew, const char *pgturl);

/*! Returns true if the response is either an authentication success
 *  or an authentication failure.
 *
 * \return 0 Authentication success;
 *         1 Authentication failure;
 *        -1 Not an authentication response;
 */
int caslib_rsp_auth(const caslib_rsp_t *);

/*! Checks if a response contains an authentication success message.
 *
 * \return true means it is an authentication success message. false
 *         might means it is an authentication failure or not an
 *         authentication response at all.
 */
bool caslib_rsp_auth_success(const caslib_rsp_t *);

/*! Returns the username associated with a auth response. Obviously
 *  this is only available if you have an authentication success
 *  message.
 *
 *  \param u The variable to receive the username (might be null);
 *
 *  \param s The size of the u variable;
 *
 *  \return The s parameter or the minimum required size of u.
 */
int caslib_rsp_auth_username(const caslib_rsp_t *, char *u, size_t s);

/*! Free memory used by a given caslib_t structure. This is the
 *  dual function of caslib_init.
 */
void caslib_destroy(caslib_t *);

/*! Free memory used by a given response_t structure.
 */
void caslib_rsp_destroy(const caslib_t *, caslib_rsp_t *);

void *caslib_alloca_alloc(const caslib_t *, size_t);
void *caslib_alloca_realloc(const caslib_t *, void *, size_t);
void caslib_alloca_destroy(const caslib_t *, void *);

#endif
