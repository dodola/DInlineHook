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
#include "utils/fs/memory_file_system.h"

#include <algorithm>
#include <climits>
#include <vector>

namespace profiler {

using std::count;
using std::function;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

MemoryFileSystem::MemoryFileSystem()
    : MemoryFileSystem(make_shared<SteadyClock>()) {}

MemoryFileSystem::MemoryFileSystem(const shared_ptr<Clock> &clock)
    : clock_(clock) {}

bool MemoryFileSystem::HasDir(const string &dpath) const {
  return dirs_.find(dpath) != dirs_.end();
}

bool MemoryFileSystem::HasFile(const string &fpath) const {
  return files_.find(fpath) != files_.end();
}

bool MemoryFileSystem::CreateDir(const string &dpath) {
  dirs_.insert(dpath);
  Touch(dpath);
  return true;
}

bool MemoryFileSystem::CreateFile(const string &fpath) {
  files_[fpath] = FileData();
  Touch(fpath);
  return true;
}

int32_t MemoryFileSystem::GetModificationAge(const string &path) const {
  auto it = timestamps_.find(path);
  if (it != timestamps_.end()) {
    return clock_->GetCurrentTime() - it->second;
  } else {
    return 0;
  }
}

void MemoryFileSystem::Touch(const string &path) {
  timestamps_[path] = clock_->GetCurrentTime();
}

void MemoryFileSystem::WalkDir(const string &dpath,
                               function<void(const PathStat &)> callback,
                               int32_t max_depth) const {
  vector<string> paths;
  for (const auto &file_entry : files_) {
    if (file_entry.first.compare(0, dpath.length(), dpath) == 0) {
      paths.push_back(file_entry.first);
    }
  }
  for (const auto &dir : dirs_) {
    if (dir != dpath && dir.compare(0, dpath.length(), dpath) == 0) {
      paths.push_back(dir);
    }
  }

  for (const auto &full_path : paths) {
    int32_t size_b = GetFileSize(full_path);
    int32_t modification_age_s = GetModificationAge(full_path);
    PathStat::Type type =
        HasDir(full_path) ? PathStat::Type::DIR : PathStat::Type::FILE;

    PathStat pstat(type, dpath, full_path, size_b, modification_age_s);
    int32_t depth =
        count(pstat.rel_path().begin(), pstat.rel_path().end(), '/');
    if (depth < max_depth) {
      callback(pstat);
    }
  }
}

int32_t MemoryFileSystem::GetFileSize(const string &fpath) const {
  return GetFileContents(fpath).length();
}

string MemoryFileSystem::GetFileContents(const string &fpath) const {
  return files_[fpath].contents;
}

bool MemoryFileSystem::MoveFile(const string &fpath_from,
                                const string &fpath_to) {
  NewFile(fpath_to);
  OpenForWrite(fpath_to);
  Append(fpath_to, GetFileContents(fpath_from));
  Close(fpath_to);
  DeleteFile(fpath_from);
  return true;
}

bool MemoryFileSystem::IsOpenForWrite(const string &fpath) const {
  return files_[fpath].in_write_mode;
}

void MemoryFileSystem::OpenForWrite(const string &fpath) {
  files_[fpath].in_write_mode = true;
}

bool MemoryFileSystem::Append(const string &fpath, const string &str) {
  files_[fpath].contents += str;
  return true;
}

void MemoryFileSystem::Close(const string &fpath) {
  files_[fpath].in_write_mode = false;
}

bool MemoryFileSystem::DeleteDir(const string &dpath) {
  timestamps_.erase(dpath);
  if (dirs_.erase(dpath) == 1) {
    // Deleting a directory should also delete all contents
    {
      auto it = dirs_.begin();
      while (it != dirs_.end()) {
        if ((*it).compare(0, dpath.length(), dpath) == 0) {
          it = dirs_.erase(it);
        } else {
          it++;
        }
      }
    }

    {
      auto it = files_.begin();
      while (it != files_.end()) {
        if (it->first.compare(0, dpath.length(), dpath) == 0) {
          it = files_.erase(it);
        } else {
          it++;
        }
      }
    }

    return true;
  } else {
    return false;
  }
}

bool MemoryFileSystem::DeleteFile(const string &fpath) {
  timestamps_.erase(fpath);
  return files_.erase(fpath) == 1;
}

int64_t MemoryFileSystem::GetFreeSpace(const std::string &path) const {
  return LONG_MAX;
}

}  // namespace profiler
