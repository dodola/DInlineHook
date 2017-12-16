/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "dir.h"
#include "utils/fs/file_system.h"

namespace profiler {

using std::function;
using std::shared_ptr;
using std::string;

Dir::Dir(FileSystem *fs, const string &abs_path) : Path(fs, abs_path) {}

bool Dir::Exists() const { return fs_->HasDir(path_); }

shared_ptr<Dir> Dir::GetDir(const string &rel_path) {
  return DoGetDir(rel_path);
}

const shared_ptr<Dir> Dir::GetDir(const string &rel_path) const {
  return DoGetDir(rel_path);
}

shared_ptr<Dir> Dir::NewDir(const string &rel_path) {
  return fs_->NewDir(Path::Append(path_, rel_path));
}

shared_ptr<Dir> Dir::GetOrNewDir(const string &rel_path) {
  return fs_->GetOrNewDir(Path::Append(path_, rel_path));
}

shared_ptr<File> Dir::GetFile(const string &rel_path) {
  return DoGetFile(rel_path);
}

const shared_ptr<File> Dir::GetFile(const string &rel_path) const {
  return DoGetFile(rel_path);
}

shared_ptr<File> Dir::NewFile(const string &rel_path) {
  return fs_->NewFile(Path::Append(path_, rel_path));
}

shared_ptr<File> Dir::GetOrNewFile(const string &rel_path) {
  return fs_->GetOrNewFile(Path::Append(path_, rel_path));
}

shared_ptr<Dir> Dir::DoGetDir(const string &rel_path) const {
  return fs_->GetDir(Path::Append(path_, rel_path));
}

shared_ptr<File> Dir::DoGetFile(const string &rel_path) const {
  return fs_->GetFile(Path::Append(path_, rel_path));
}

void Dir::Walk(function<void(const PathStat &)> callback,
               int32_t max_depth) const {
  fs_->WalkDir(path_, callback, max_depth);
}

bool Dir::HandleCreate() { return fs_->CreateDir(path_); }

bool Dir::HandleDelete() { return fs_->DeleteDir(path_); }
}  // namespace profiler
