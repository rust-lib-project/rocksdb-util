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
#include "rocksdb/filter_policy.h"

using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::SliceParts;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::FilterBitsBuilder;
using ROCKSDB_NAMESPACE::CreateStandard128RibbonBitsBuilder;
using ROCKSDB_NAMESPACE::CreateFastLocalBloomBitsBuilder;

using std::vector;
using std::unordered_set;

extern "C" {

struct rocksdb_filterbits_builder_t {
    FilterBitsBuilder* builder;
};

void rocksdb_filterbits_builder_destroy(rocksdb_filterbits_builder_t* filter) {
  if (filter->builder != nullptr) {
      delete filter->builder;
  }
  delete filter;
}

rocksdb_filterbits_builder_t* rocksdb_ribbon_filterbits_builder_create(double bits_per_key) {
    rocksdb_filterbits_builder_t* wrapper = new rocksdb_filterbits_builder_t;
    wrapper->builder = CreateStandard128RibbonBitsBuilder(bits_per_key);
    return wrapper;
}

rocksdb_filterbits_builder_t* rocksdb_fast_bloom_filter_builder_create(double bits_per_key) {
    rocksdb_filterbits_builder_t* wrapper = new rocksdb_filterbits_builder_t;
    wrapper->builder = CreateStandard128RibbonBitsBuilder(bits_per_key);
    return wrapper;
}

}  // end extern "C"

#endif  // !ROCKSDB_LITE
