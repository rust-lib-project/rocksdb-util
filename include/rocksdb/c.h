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

typedef struct rocksdb_env_t             rocksdb_env_t;
typedef struct rocksdb_filterpolicy_t    rocksdb_filterpolicy_t;

/* Filter policy */
extern ROCKSDB_LIBRARY_API void rocksdb_filterpolicy_destroy(
    rocksdb_filterpolicy_t*);

extern ROCKSDB_LIBRARY_API rocksdb_filterpolicy_t*
rocksdb_filterpolicy_create_bloom(double bits_per_key);
extern ROCKSDB_LIBRARY_API rocksdb_filterpolicy_t*
rocksdb_filterpolicy_create_bloom_full(double bits_per_key);
extern ROCKSDB_LIBRARY_API rocksdb_filterpolicy_t*
rocksdb_filterpolicy_create_ribbon(double bloom_equivalent_bits_per_key);
extern ROCKSDB_LIBRARY_API rocksdb_filterpolicy_t*
rocksdb_filterpolicy_create_ribbon_hybrid(double bloom_equivalent_bits_per_key,
                                          int bloom_before_level);
#ifdef __cplusplus
}  /* end extern "C" */
#endif
