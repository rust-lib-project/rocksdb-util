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

using ROCKSDB_NAMESPACE::BuiltinFilterBitsReader;
using ROCKSDB_NAMESPACE::CreateFastLocalBloomBitsBuilder;
using ROCKSDB_NAMESPACE::CreateStandard128RibbonBitsBuilder;
using ROCKSDB_NAMESPACE::FilterBitsBuilder;
using ROCKSDB_NAMESPACE::FilterBitsReader;
using ROCKSDB_NAMESPACE::GetBuiltinFilterBitsReader;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::SliceParts;
using ROCKSDB_NAMESPACE::Status;

using std::vector;
using std::unordered_set;

extern "C" {

struct rocksdb_filterbits_builder_t {
    FilterBitsBuilder* builder;
};

struct rocksdb_filterbits_reader_t {
  BuiltinFilterBitsReader *reader;
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
    wrapper->builder = CreateFastLocalBloomBitsBuilder(bits_per_key);
    return wrapper;
}

void rocksdb_filterbits_builder_add_key(rocksdb_filterbits_builder_t* filter, const char* key, size_t keylen) {
    filter->builder->AddKey(Slice(key, keylen));
}

void rocksdb_filterbits_builder_add_key_hash(rocksdb_filterbits_builder_t* filter, uint64_t hash) {
    filter->builder->AddKeyHash(hash);
}

const char* rocksdb_filterbits_builder_finish(rocksdb_filterbits_builder_t* filter, size_t* buf_len) {
    std::unique_ptr<const char []> buf;
    Slice ret = filter->builder->Finish(&buf);
    *buf_len = ret.size();
    return buf.release();
}

void rocksdb_filterbits_reader_destroy(rocksdb_filterbits_reader_t* filter) {
    if (filter->reader != nullptr) {
        delete filter->reader;
    }
    delete filter;
}

rocksdb_filterbits_reader_t* rocksdb_filterbits_reader_create(const char* data, size_t data_len) {
    rocksdb_filterbits_reader_t* wrapper = new rocksdb_filterbits_reader_t;
    wrapper->reader = GetBuiltinFilterBitsReader(Slice(data, data_len));
    if (wrapper->reader == nullptr) {
        delete wrapper;
        return nullptr;
    }
    return wrapper;
}

bool rocksdb_filterbits_reader_may_match(rocksdb_filterbits_reader_t* filter, const char* key, size_t key_len) {
    return filter->reader->MayMatch(Slice(key, key_len));
}

bool rocksdb_filterbits_reader_may_match_hash(
    rocksdb_filterbits_reader_t *filter, uint64_t hash) {
  return filter->reader->HashMayMatch(hash);
}

}  // end extern "C"

#endif  // !ROCKSDB_LITE
