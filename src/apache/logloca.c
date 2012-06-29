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

#include <stdlib.h>
#include <stdarg.h>
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_log.h"
#include "caslib/misc.h"
#include "caslib/log.h"

void logger_ap_debug(void *_, const char *format, ...) {
    char *msg;
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);
    CASLIB_UNUSED(_);
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, msg);
}

void logger_ap_info(void *_, const char *format, ...) {
    char *msg;
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);
    CASLIB_UNUSED(_);
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, msg);
}

void logger_ap_warn(void *_, const char *format, ...) {
    char *msg;
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);
    CASLIB_UNUSED(_);
    ap_log_error(APLOG_MARK, APLOG_WARNING, 0, msg);
}

void logger_ap_error(void *_, const char *format, ...) {
    char *msg;
    va_list args;
    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);
    CASLIB_UNUSED(_);
    ap_log_error(APLOG_MARK, APLOG_ERR, 0, msg);
}

void logger_apache(logger_t *ptr) {
  ptr->debug_f = logger_ap_debug;
  ptr->info_f  = logger_ap_info;
  ptr->warn_f  = logger_ap_warn;
  ptr->error_f = logger_ap_error;
}
