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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <libxml/xmlstring.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "misc.h"
#include "caslib.h"

struct caslib_t {
  char *endpoint;
  char *service_validate_url;
  int timeout;
  alloca_t alloca;
  logger_t logger;
};

struct caslib_rsp_t {
  long status;
  xmlDocPtr xml;
};

static
void __libxml_errfunc(void * data, xmlErrorPtr e) {
  const caslib_t *cas   = (const caslib_t *) data;

  switch (e->level) {
  case XML_ERR_NONE:
    CASLIB_INFO(cas->logger,
                 "%s:%d:%d %s",
                 e->file,
                 e->line,
                 e->int1,
                 e->message)
    break;
  case XML_ERR_WARNING:
    CASLIB_WARN(cas->logger,
                 "%s:%d:%d %s",
                 e->file,
                 e->line,
                 e->int1,
                 e->message)
    break;
  case XML_ERR_ERROR:
  case XML_ERR_FATAL:
    CASLIB_ERROR(cas->logger,
                 "%s:%d:%d %s",
                 e->file,
                 e->line,
                 e->int1,
                 e->message)
    break;
  }
}


static
size_t __write_cc(void *payload, size_t usize, size_t nmemb, void *arg) {
  xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) arg;

  size_t size = usize * nmemb;
  int rc      = xmlParseChunk(ctxt, payload, (int) size, 0);

  return(rc==0 ? size : 0);
}

static inline
void __curl_set_common_opt(const caslib_t *cas, CURL *curl) {
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "caslib/v0.0.0");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, cas->timeout);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __write_cc);
}

static
int __joinparams(const caslib_t *cas, char *dest, size_t sz, int argv, ...) {
  int rc       = (int) sz,
      k        = 0;
  char *fmtstr = NULL;
  va_list args;

  va_start(args, argv);
  if (dest != NULL) {
    fmtstr = CASLIB_ALLOC_F(cas->alloca, (size_t) (3*argv));
    for (k=0; k<argv; k+=1) {
      if (k == 0)
        strncpy(fmtstr, "%s", 2);
      else
        strncpy(fmtstr + 3*k-1, "&%s", 3);
    }
    fmtstr[3*k-1] = '\0';
    rc = vsnprintf(dest, sz, fmtstr, args);
  } else {
    rc = argv;
    for (k=0; k<argv; k+=1)
      rc += (int) strlen(va_arg(args, char *));
  }
  va_end(args);

  CASLIB_DESTROY_F(cas->alloca, fmtstr);
  return(rc);
}

static
int __uencode(CURL *curl, char *dest, size_t sz, const char *key, const char *val) {
  char *e_key = NULL;
  char *e_val = NULL;
  int rc      = -1;

  e_key = curl_easy_escape(curl, key, 0);
  e_val = curl_easy_escape(curl, val, 0);
  CASLIB_GOTOIF(e_key==NULL || e_val==NULL, release);

  if (dest != NULL) {
    snprintf(dest, sz, "%s=%s", e_key, e_val);
    rc = (int) strlen(dest);
  } else {
    rc = (int) (2 + strlen(e_key) + strlen(e_val));
  }

 release:
  curl_free(e_key);
  curl_free(e_val);
  return(rc);
}

static
char *__uencode_r(const caslib_t *cas, CURL *curl, const char *key, const char *val) {
  int rc      = -1;
  char *param = NULL;
  int sz      = __uencode(curl, NULL, 0, key, val);

  if (sz != -1) {
    param = CASLIB_ALLOC_F(cas->alloca, (size_t) sz);
    rc    = __uencode(curl, param, (size_t) sz, key, val);
  }

  if (rc == -1) {
    CASLIB_DESTROY_F(cas->alloca, param);
    param = NULL;
  }

  return(param);
}

int caslib_global_init() {
  xmlInitParser();
  return(curl_global_init(CURL_GLOBAL_ALL));
}

void caslib_global_destroy() {
  curl_global_cleanup();
  xmlCleanupParser();
}

caslib_t *caslib_init(const char *endpoint) {
  alloca_t alloca;
  alloca_stdlib(&alloca);
  return(caslib_init_with(endpoint, &alloca));
}

caslib_t *caslib_init_with(const char *endpoint, const alloca_t *ptr) {
  caslib_t *p = CASLIB_ALLOC_F_PTR(ptr, sizeof(caslib_t));
  CASLIB_GOTOIF(p==NULL, failure);
  p->endpoint             = NULL;
  p->service_validate_url = NULL;
  p->timeout              = 60;
  p->alloca.alloc_f       = ptr->alloc_f;
  p->alloca.destroy_f     = ptr->destroy_f;
  p->alloca.realloc_f     = ptr->realloc_f;
  p->logger.debug_f       = NULL;
  p->logger.info_f        = NULL;
  p->logger.warn_f        = NULL;
  p->logger.error_f       = NULL;
  p->logger.data          = NULL;

  p->endpoint = CASLIB_ALLOC_F_PTR(ptr, strlen(endpoint) + 1);
  CASLIB_GOTOIF(p->endpoint==NULL, failure);
  strcpy(p->endpoint, endpoint);

  p->service_validate_url = CASLIB_ALLOC_F_PTR(ptr, strlen("/serviceValidate") + 1);
  CASLIB_GOTOIF(p->service_validate_url==NULL, failure);
  strcpy(p->service_validate_url, "/serviceValidate");

  return(p);

 failure:
  caslib_destroy(p);
  return(NULL);
}

void caslib_setopt_logging(caslib_t *cas, const logger_t *logger) {
  cas->logger.debug_f = logger->debug_f;
  cas->logger.info_f  = logger->info_f;
  cas->logger.warn_f  = logger->warn_f;
  cas->logger.error_f = logger->error_f;
  cas->logger.data    = logger->data;
}

caslib_rsp_t *caslib_service_validate(const caslib_t *cas, const char *service, const char *ticket, bool renew) {
  int rc;
  long status;
  size_t sz;
  char *url                  = NULL,
       *eservice             = NULL,
       *eticket              = NULL,
       *erenew               = NULL,
       *reqbdy               = NULL;
  CURL *curl                 = NULL;
  struct curl_slist *headers = NULL;
  xmlParserCtxtPtr ctxt      = NULL;
  caslib_rsp_t *rsp          = NULL;

  CASLIB_DEBUG_(cas->logger, "enter: caslib_service_validate");
  CASLIB_DEBUG_(cas->logger, " creating xml context...");
  ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, "caslib_service_validate.xml");
  CASLIB_GOTOIF(ctxt==NULL, failure);
  xmlSetStructuredErrorFunc((void *) cas, __libxml_errfunc);

  CASLIB_DEBUG_(cas->logger, " creating curl handle...");
  curl = curl_easy_init();
  CASLIB_GOTOIF(curl==NULL, failure);

  sz  = strlen(cas->endpoint) + strlen(cas->service_validate_url);
  url = CASLIB_ALLOC_F(cas->alloca, sz+1);
  CASLIB_GOTOIF(url==NULL, failure);
  rc  = snprintf(url, sz+1, "%s%s", cas->endpoint, cas->service_validate_url);
  CASLIB_GOTOIF(rc<0, failure);
  CASLIB_DEBUG(cas->logger, " using `%s' to validate service ticket", url);


  eservice = __uencode_r(cas, curl, "service", service);
  eticket  = __uencode_r(cas, curl, "ticket", ticket);
  erenew   = __uencode_r(cas, curl, "renew", (renew ? "true" : "false"));
  rc       = __joinparams(cas, NULL, 0, 3, eservice, eticket, erenew);
  CASLIB_GOTOIF(rc<0, failure);
  reqbdy   = CASLIB_ALLOC_F(cas->alloca, (size_t) rc);
  CASLIB_GOTOIF(reqbdy==NULL, failure);
  rc       = __joinparams(cas, reqbdy, (size_t) rc, 3, eservice, eticket, erenew);
  CASLIB_GOTOIF(rc<0, failure);
  CASLIB_DEBUG(cas->logger, " along with the post fields: %s", reqbdy);

  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
  CASLIB_GOTOIF(headers==NULL, failure);

  __curl_set_common_opt(cas, curl);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(reqbdy));
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqbdy);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctxt);
  CURLcode curl_rc = curl_easy_perform(curl);
  CASLIB_DEBUG(cas->logger, " request made, curl_rc: %d", curl_rc);
  CASLIB_GOTOIF(curl_rc!=0, failure);

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  CASLIB_DEBUG(cas->logger, " http status code: %ld", status);

  xmlParseChunk(ctxt, NULL, 0, 1);
  CASLIB_DEBUG(cas->logger, " response xml well formed: %d", ctxt->wellFormed);
  CASLIB_GOTOIF(! ctxt->wellFormed, failure);

  rsp = CASLIB_ALLOC_F(cas->alloca, sizeof(caslib_rsp_t));
  CASLIB_GOTOIF(rsp==NULL, failure);
  rsp->xml    = ctxt->myDoc;
  rsp->status = status;
  CASLIB_GOTOIF(true, release);

 failure:
  CASLIB_DESTROY_F(cas->alloca, rsp);
  rsp = NULL;
  if (ctxt != NULL)
    xmlFreeDoc(ctxt->myDoc);

 release:
  xmlFreeParserCtxt(ctxt);
  CASLIB_DESTROY_F(cas->alloca, reqbdy);
  CASLIB_DESTROY_F(cas->alloca, erenew);
  CASLIB_DESTROY_F(cas->alloca, eticket);
  CASLIB_DESTROY_F(cas->alloca, eservice);
  CASLIB_DESTROY_F(cas->alloca, url);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  CASLIB_DEBUG_(cas->logger, "exit: caslib_service_validate");
  return(rsp);
}

void caslib_destroy(caslib_t *p) {
  if (p == NULL)
    return;
  CASLIB_DESTROY_F(p->alloca, p->endpoint);
  CASLIB_DESTROY_F(p->alloca, p->service_validate_url);
  CASLIB_DESTROY_F(p->alloca, p);
}

void caslib_rsp_destroy(caslib_t *c, caslib_rsp_t *p) {
  if (p == NULL)
    return;
  xmlFreeDoc(p->xml);
  CASLIB_DESTROY_F(c->alloca, p);
}

int caslib_rsp_auth(const caslib_rsp_t *p) {
  if (p->xml == NULL)
    return(-1);

  xmlNodePtr node = xmlDocGetRootElement(p->xml)->children;
  while (node != NULL && (node = node->next)) {
    if (node->type != XML_ELEMENT_NODE
        || node->ns == NULL
        || xmlStrcmp(node->ns->href, (xmlChar *) "http://www.yale.edu/tp/cas") != 0)
      continue;
    if (xmlStrcmp(node->name, (xmlChar *) "authenticationFailure") == 0)
      return(1);
    if (xmlStrcmp(node->name, (xmlChar *) "authenticationSuccess") == 0)
      return(0);
  }

  return(-1);
}

bool caslib_rsp_auth_success(const caslib_rsp_t *p) {
  return(caslib_rsp_auth(p) == 0);
}
