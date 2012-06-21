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

#ifndef __LOCASBEROS_CASLIB__
#define __LOCASBEROS_CASLIB__

#include <stdlib.h>
#include <stdbool.h>

typedef struct caslib_t caslib_t;
typedef struct casresponse_t casresponse_t;

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
casresponse_t *caslib_service_validate(const char *service, const char *ticket, bool renew);

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
 *         memory using casresponse_destroy, in order to make sure the
 *         proper function gets called.
 */
casresponse_t *caslib_service_validate_pgt(const char *service, const char *ticket, bool renew, const char *pgturl);

/*! Returns true if the response is either an authentication success
 *  or an authentication failure.
 */
bool casresponse_authentication(const casresponse_t *);

/*! Checks if a response contains an authentication success message.
 *
 * \return true means it is an authentication success message. false
 *         might means it is an authentication failure or not an
 *         authentication response at all.
 */
bool casresponse_authentication_success(const casresponse_t *);

/*! Free memory used by a given caslib_t structure. This is the
 *  dual function of caslib_init.
 */
void caslib_destroy(caslib_t *);

/*! Free memory used by a given response_t structure.
 */
void casresponse_destroy(caslib_t *, casresponse_t *);

#endif