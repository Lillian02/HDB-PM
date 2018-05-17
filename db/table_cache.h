// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Thread-safe (provides internal synchronization)

#ifndef STORAGE_LEVELDB_DB_TABLE_CACHE_H_
#define STORAGE_LEVELDB_DB_TABLE_CACHE_H_

#include <string>
#include <stdint.h>
#include "db/dbformat.h"
#include "hyperleveldb/cache.h"
#include "hyperleveldb/table.h"
#include "port/port.h"

namespace leveldb {

class Env;

class TableCache {
 public:
  TableCache(const std::string& dbname, const std::string& ddir, const Options* options, int entries);
  ~TableCache();

  // Return an iterator for the specified file number (the corresponding
  // file length must be exactly "file_size" bytes).  If "tableptr" is
  // non-NULL, also sets "*tableptr" to point to the Table object
  // underlying the returned iterator, or NULL if no Table object underlies
  // the returned iterator.  The returned "*tableptr" object is owned by
  // the cache and should not be deleted, and is valid for as long as the
  // returned iterator is live.
  Iterator* NewIterator(const ReadOptions& options,
                        uint64_t file_number,
                        uint64_t file_size,
                        size_t level,     //5.6 增加一个参数
                        Table** tableptr = NULL);

  // If a seek to internal key "k" in specified file finds an entry,
  // call (*handle_result)(arg, found_key, found_value).
  Status Get(const ReadOptions& options,
             size_t level,     //5.6 增加一个参数level
             uint64_t file_number,
             uint64_t file_size,
             const Slice& k,
             void* arg,
             void (*handle_result)(void*, const Slice&, const Slice&));

  // Evict any entry for the specified file number
  void Evict(uint64_t file_number);

 private:
  TableCache(const TableCache&);
  TableCache& operator = (const TableCache&);
  Env* const env_;
  const std::string dbname_;
  const std::string data_dir_;   //5.7
  const Options* options_;
  Cache* cache_;

  //5.6 增加一个参数level
  Status FindTable(size_t level, uint64_t file_number, uint64_t file_size, Cache::Handle**);
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_TABLE_CACHE_H_
