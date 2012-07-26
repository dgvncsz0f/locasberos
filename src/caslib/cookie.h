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

#ifndef __LOCASBEROS_COOKIE__
#define __LOCASBEROS_COOKIE__

#include "caslib/alloca.h"
#include "caslib/caslib.h"

typedef struct caslib_cookie_t caslib_cookie_t;

// TODO: cookie_check_signature
// TODO: cookie_check_timestamp

/*! Creates a new cookie from a successfull caslib response. It is
 *  currently undefined to invoke this function with a message other
 *  than success.
 *
 * \param r A successful CAS response;
 * 
 * \return The cookie struct or NULL if something goes wrong.
 */
caslib_cookie_t *cookie_init(const caslib_t *, const caslib_rsp_t *r);

/*! Unserializes the cookie that has been written by the
 *  cookie_serialize function.
 * 
 * \param s The cookie string.
 *
 * \return The cookie struct or NULL if something goes wrong.
 */
caslib_cookie_t *cookie_unserialize(const caslib_t *, const char *s);

/*! Serializes the cookie in a 7-bit ascii format.
 *
 *  \param u The cookie you want to serialize.
 *
 *  \param o The variable that will receive the cookie.
 *
 *  \param s The size of the o variable;
 *
 *  \return The s parameter or the minimum required size of o.
 */
int cookie_serialize(caslib_cookie_t *c, char *o, size_t s);

/*! Frees all memory associated with the cookie structure.
 */
void cookie_destroy(const caslib_t *, caslib_cookie_t *);

#endif
