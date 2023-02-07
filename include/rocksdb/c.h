//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

/* Copyright (c) 2011 The LevelDB Authors. All rights reserved.
  Use of this source code is governed by a BSD-style license that can be
  found in the LICENSE file. See the AUTHORS file for names of contributors.

  C bindings for rocksdb.  May be useful as a stable ABI that can be
  used by programs that keep rocksdb in a shared library, or for
  a JNI api.

  Does not support:
  . getters for the option types
  . custom comparators that implement key shortening
  . capturing post-write-snapshot
  . custom iter, db, env, cache implementations using just the C bindings

  Some conventions:

  (1) We expose just opaque struct pointers and functions to clients.
  This allows us to change internal representations without having to
  recompile clients.

  (2) For simplicity, there is no equivalent to the Slice type.  Instead,
  the caller has to pass the pointer and length as separate
  arguments.

  (3) Errors are represented by a null-terminated c string.  NULL
  means no error.  All operations that can raise an error are passed
  a "char** errptr" as the last argument.  One of the following must
  be true on entry:
     *errptr == NULL
     *errptr points to a malloc()ed null-terminated error message
  On success, a leveldb routine leaves *errptr unchanged.
  On failure, leveldb frees the old value of *errptr and
  set *errptr to a malloc()ed error message.

  (4) Bools have the type unsigned char (0 == false; rest == true)

  (5) All of the pointer arguments must be non-NULL.
*/

#pragma once

#ifdef _WIN32
#ifdef ROCKSDB_DLL
#ifdef ROCKSDB_LIBRARY_EXPORTS
#define ROCKSDB_LIBRARY_API __declspec(dllexport)
#else
#define ROCKSDB_LIBRARY_API __declspec(dllimport)
#endif
#else
#define ROCKSDB_LIBRARY_API
#endif
#else
#define ROCKSDB_LIBRARY_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Exported types */

typedef struct rocksdb_filterbits_builder_t rocksdb_filterbits_builder_t;
typedef struct rocksdb_filterbits_reader_t rocksdb_filterbits_reader_t;

/* Filter policy */
extern ROCKSDB_LIBRARY_API void rocksdb_filterbits_builder_destroy(
        rocksdb_filterbits_builder_t*);

extern ROCKSDB_LIBRARY_API rocksdb_filterbits_builder_t*
rocksdb_ribbon_filterbits_builder_create(double bits_per_key);
extern ROCKSDB_LIBRARY_API rocksdb_filterbits_builder_t*
rocksdb_fast_bloom_filter_builder_create(double bits_per_key);
extern ROCKSDB_LIBRARY_API void rocksdb_filterbits_builder_add_key(rocksdb_filterbits_builder_t*, const char*, size_t);
extern ROCKSDB_LIBRARY_API void rocksdb_filterbits_builder_add_key_hash(rocksdb_filterbits_builder_t* filter, uint64_t);
extern ROCKSDB_LIBRARY_API const char* rocksdb_filterbits_builder_finish(rocksdb_filterbits_builder_t*, size_t*);
extern ROCKSDB_LIBRARY_API void rocksdb_filterbits_reader_destroy(rocksdb_filterbits_reader_t* filter);
extern ROCKSDB_LIBRARY_API rocksdb_filterbits_reader_t*
rocksdb_filterbits_reader_create(const char* data, size_t data_len);
extern ROCKSDB_LIBRARY_API bool rocksdb_filterbits_reader_may_match(rocksdb_filterbits_reader_t*, const char*, size_t);
extern ROCKSDB_LIBRARY_API bool
rocksdb_filterbits_reader_may_match_hash(rocksdb_filterbits_reader_t *,
                                         uint64_t hash);

#ifdef __cplusplus
}  /* end extern "C" */
#endif
