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

#include <stdlib.h>
#include <string.h>
#include "alloca.h"
#include "cas_client.h"

struct casclient_t
{
  /* The cas endpoint to use */
  char *endpoint;

  /* The timeout in seconds */
  int timeout;

  /* The memory allocation algorithm */
  alloca_t alloca;
};


casclient_t *casclient_init(const char *endpoint)
{
  alloca_t alloca;
  alloca_std(&alloca);
  return(casclient_init2(endpoint, &alloca));
}

casclient_t *casclient_init2(const char *endpoint, const alloca_t *ptr)
{
  casclient_t *p = ptr->alloca_f(sizeof(casclient_t));
  p->endpoint         = NULL;
  p->timeout          = 60;
  p->alloca.alloca_f  = ptr->alloca_f;
  p->alloca.destroy_f = ptr->destroy_f;
  CATCH(p==NULL, error_handler);

  p->endpoint = ptr->alloca_f(sizeof(char) * (strlen(endpoint) + 1));
  CATCH(p->endpoint==NULL, error_handler);
  strcpy(p->endpoint, endpoint);

  return(p);

 error_handler:
  casclient_destroy(p);
  return(NULL);
}

void casclient_destroy(casclient_t *p)
{
  if (p == NULL)
    return;
  p->alloca.destroy_f(p->endpoint);
  p->alloca.destroy_f(p);
}
