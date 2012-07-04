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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "misc.h"
#include "logging.h"

static
void __dolog(const char *file, int line, const char *fmt, va_list args) {
    fprintf(stderr, "%s:%d: ", file, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

static
void __debug(void *_, const char *file, int line, const char *fmt, ...) {
  CASLIB_UNUSED(_);
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "debug ");
  __dolog(file, line, fmt, args);
  va_end(args);
}

static
void __info(void *_, const char *file, int line, const char *fmt, ...) {
  CASLIB_UNUSED(_);
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "warn ");
  __dolog(file, line, fmt, args);
  va_end(args);
}

static
void __warn(void *_, const char *file, int line, const char *fmt, ...) {
  CASLIB_UNUSED(_);
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "info ");
  __dolog(file, line, fmt, args);
  va_end(args);
}

static
void __error(void *_, const char *file, int line, const char *fmt, ...) {
  CASLIB_UNUSED(_);
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "error ");
  __dolog(file, line, fmt, args);
  va_end(args);
}

logger_t default_logger = { __debug,
                            __info,
                            __warn,
                            __error,
                            NULL
                          };

