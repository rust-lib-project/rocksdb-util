//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef ROCKSDB_LITE

#include "rocksdb/c.h"

#include <cstdlib>
#include <map>
#include <unordered_set>
#include <vector>

#include "port/port.h"
#include "rocksdb/status.h"

using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::SliceParts;
using ROCKSDB_NAMESPACE::Status;

using std::vector;
using std::unordered_set;

extern "C" {

struct rocksdb_filterpolicy_t {
    int a;
};


void rocksdb_filterpolicy_destroy(rocksdb_filterpolicy_t* filter) {
  delete filter;
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_bloom_format() {
  return NULL;
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_bloom_full(
    double bits_per_key) {
    return NULL;
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_bloom(double bits_per_key) {
    return NULL;
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_ribbon_format(
    double bloom_equivalent_bits_per_key, int bloom_before_level) {
  // Make a rocksdb_filterpolicy_t, but override all of its methods so
  // they delegate to a NewRibbonFilterPolicy() instead of user
  return NULL;
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_ribbon(
    double bloom_equivalent_bits_per_key) {
  return rocksdb_filterpolicy_create_ribbon_format(
      bloom_equivalent_bits_per_key, /*bloom_before_level = disabled*/ -1);
}

rocksdb_filterpolicy_t* rocksdb_filterpolicy_create_ribbon_hybrid(
    double bloom_equivalent_bits_per_key, int bloom_before_level) {
  return rocksdb_filterpolicy_create_ribbon_format(
      bloom_equivalent_bits_per_key, bloom_before_level);
}

}  // end extern "C"

#endif  // !ROCKSDB_LITE
