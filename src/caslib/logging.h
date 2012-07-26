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

#ifndef __LOCASBEROS_LOGGING__
#define __LOCASBEROS_LOGGING__

#include <stdlib.h>

typedef struct logger_t {
  void (*debug_f)(void *data, const char *file, int line, const char *fmt, ...);
  void (*info_f)(void *data, const char *file, int line, const char *fmt, ...);
  void (*warn_f)(void *data, const char *file, int line, const char *fmt, ...);
  void (*error_f)(void *data, const char *file, int line, const char *fmt, ...);
  void *data;
} logger_t;

#define CASLIB_DEBUG_(logger, fmt) if (logger.debug_f != NULL) { logger.debug_f(logger.data, __FILE__, __LINE__, fmt); }
#define CASLIB_DEBUG(logger, fmt, ...) if (logger.debug_f != NULL) { logger.debug_f(logger.data, __FILE__, __LINE__, fmt, __VA_ARGS__); }

#define CASLIB_INFO_(logger, fmt)  if (logger.info_f != NULL) { logger.info_f(logger.data, __FILE__, __LINE__, fmt); }
#define CASLIB_INFO(logger, fmt, ...)  if (logger.info_f != NULL) { logger.info_f(logger.data, __FILE__, __LINE__, fmt, __VA_ARGS__); }

#define CASLIB_WARN_(logger, fmt)  if (logger.warn_f != NULL) { logger.warn_f(logger.data, __FILE__, __LINE__, fmt); }
#define CASLIB_WARN(logger, fmt, ...)  if (logger.warn_f != NULL) { logger.warn_f(logger.data, __FILE__, __LINE__, fmt, __VA_ARGS__); }

#define CASLIB_ERROR_(logger, fmt) if (logger.error_f != NULL) { logger.error_f(logger.data, __FILE__, __LINE__, fmt); }
#define CASLIB_ERROR(logger, fmt, ...) if (logger.error_f != NULL) { logger.error_f(logger.data, __FILE__, __LINE__, fmt, __VA_ARGS__); }

/*! Uses STDERR as the logging medium
 */
extern logger_t default_logger;

#endif
