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

#ifndef __LOCASBEROS_ALLOCA__
#define __LOCASBEROS_ALLOCA__

#define CASLIB_ALLOC_F(a, s) (a.alloc_f == NULL ? NULL : a.alloc_f(&a, s))
#define CASLIB_ALLOC_F_PTR(a, s) (a->alloc_f == NULL ? NULL : a->alloc_f(a, s))

#define CASLIB_REALLOC_F(a, p, s) (a.realloc_f == NULL ? NULL : a.realloc_f(&a, p, s))
#define CASLIB_REALLOC_F_PTR(a, p, s) (a->realloc_f == NULL ? NULL : a.realloc_f(a, p, s))

#define CASLIB_DESTROY_F(a, p) if (a.destroy_f != NULL) { a.destroy_f(&a, p); }
#define CASLIB_DESTROY_F_PTR(a, p) if (a->destroy_f != NULL) { a->destroy_f(a, p); }

#define ALLOCA_DUMMY_FREE NULL
#define ALLOCA_DUMMY_REALLOC NULL

typedef struct alloca_t {
  /*! This function should allocate size bytes and return a pointer to
   *  the allocated memory.
   *
   * \param size How many bytes to allocate;
   * \return The pointer to the allocated memory. It may also be NULL,
   *         which represents an error;
   */
  void *(*alloc_f)(const struct alloca_t *, size_t size);

  /*! Frees memory used by a given pointer, which was allocated by
   *  alloca_f. It is considered an error to invoke this function
   *  twice or more.
   *
   * \param ptr Pointer to a memory allocated by alloca_f (might be NULL);
   */
  void (*destroy_f)(const struct alloca_t *, void *ptr);

  /*! Changes the size of the memory block pointed to by ptr to size
   *  bytes.
   *
   * \param ptr Pointer to a memory allocated by alloca_f (might be NULL);
   * \param size The new size.
   */
  void *(*realloc_f)(const struct alloca_t *, void *ptr, size_t size);

  /*! Data-pointer that may be retrieved inside alloca_f or destroy_f functions.
   */
  void *data;

} alloca_t;

void alloca_stdlib(alloca_t *ptr);
void *alloca_stdlib_malloc(const alloca_t *, size_t);
void *alloca_stdlib_realloc(const alloca_t *, void *ptr, size_t);
void alloca_stdlib_free(const alloca_t *, void *);

#endif
