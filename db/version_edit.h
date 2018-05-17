// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_VERSION_EDIT_H_
#define STORAGE_LEVELDB_DB_VERSION_EDIT_H_

#include <set>
#include <utility>
#include <vector>
#include "db/dbformat.h"

namespace leveldb {

class VersionSet;

struct FileMetaData {
  int refs;
  int allowed_seeks;          // Seeks allowed until compaction
  uint64_t number;
  uint64_t file_size;         // File size in bytes
  InternalKey smallest;       // Smallest internal key served by table
  InternalKey largest;        // Largest internal key served by table

  FileMetaData() : refs(0), allowed_seeks(1 << 30), number(0), file_size(0), smallest(), largest() { }
};

class VersionEdit {
 public:
  VersionEdit()
    : comparator_(),
      log_number_(),
      prev_log_number_(),
      next_file_number_(),
      last_sequence_(),
      has_comparator_(),
      has_log_number_(),
      has_prev_log_number_(),
      has_next_file_number_(),
      has_last_sequence_(),
      level(),   //5.9
      compact_pointers_(),
      deleted_files_(),
      deleted_p2files_(),   //5.10
      new_files_(),
      new_l0_files_() {    //3.26 初始化
    Clear();
  }
  ~VersionEdit() { }

  void Clear();

  void SetComparatorName(const Slice& name) {
    has_comparator_ = true;
    comparator_ = name.ToString();
  }
  void SetLogNumber(uint64_t num) {
    has_log_number_ = true;
    log_number_ = num;
  }
  void SetPrevLogNumber(uint64_t num) {
    has_prev_log_number_ = true;
    prev_log_number_ = num;
  }
  void SetNextFile(uint64_t num) {
    has_next_file_number_ = true;
    next_file_number_ = num;
  }
  void SetLastSequence(SequenceNumber seq) {
    has_last_sequence_ = true;
    last_sequence_ = seq;
  }
  void SetCompactPointer(int level, const InternalKey& key) {
    compact_pointers_.push_back(std::make_pair(level, key));
  }

  // Add the specified file at the specified number.
  // REQUIRES: This version has not been saved (see VersionSet::SaveTo)
  // REQUIRES: "smallest" and "largest" are smallest and largest keys in file
  void AddFile(int level, uint64_t file,
               uint64_t file_size,
               const InternalKey& smallest,
               const InternalKey& largest) {
    FileMetaData f;
    f.number = file;
    f.file_size = file_size;
    f.smallest = smallest;
    f.largest = largest;
    new_files_.push_back(std::make_pair(level, f));
  }

  //3.26 增加Level0分区文件
  void AddL0File(int par, uint64_t file,
        uint64_t file_size,
        const InternalKey& smallest,
        const InternalKey& largest) {
    FileMetaData f;
    f.number = file;
    f.file_size = file_size;
    f.smallest = smallest;
    f.largest = largest;
    new_l0_files_.push_back(std::make_pair(par, f));
    }

  // Delete the specified "file" from the specified "level".
  void DeleteFile(int par, uint64_t file) {
    deleted_files_.insert(std::make_pair(par, file));
  }

  //5.10 添加第二层有序文件的删除方法
  void DeleteP2File(int par, uint64_t file) {
    deleted_p2files_.insert(std::make_pair(par, file));
  }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(const Slice& src);

  std::string DebugString() const;

 private:
  friend class VersionSet;

  typedef std::set< std::pair<int, uint64_t> > DeletedFileSet;

  std::string comparator_;
  uint64_t log_number_;
  uint64_t prev_log_number_;
  uint64_t next_file_number_;
  SequenceNumber last_sequence_;
  bool has_comparator_;
  bool has_log_number_;
  bool has_prev_log_number_;
  bool has_next_file_number_;
  bool has_last_sequence_;

  //5.9 增加level，因为多了个par
  size_t level;

  std::vector< std::pair<int, InternalKey> > compact_pointers_;
  DeletedFileSet deleted_files_;
  DeletedFileSet deleted_p2files_;    //5.10 为第二层分区内的有序文件提供一个删除列表
  std::vector< std::pair<int, FileMetaData> > new_files_;
  std::vector< std::pair<int, FileMetaData> > new_l0_files_;  //3.26用于临时存放新的L0分区文件
  
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_VERSION_EDIT_H_
