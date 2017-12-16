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
#ifndef UTILS_FS_DISK_FILE_SYSTEM_H_
#define UTILS_FS_DISK_FILE_SYSTEM_H_

#include "utils/fs/file_system.h"

#include <cstdio>
#include <functional>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace profiler {

// A FileSystem implementation that stores files and directories to disk. This
// class uses APIs that are guaranteed to be available on Android.
class DiskFileSystem final : public FileSystem {
 public:
  ~DiskFileSystem() override;

  bool HasDir(const std::string &path) const override;
  bool HasFile(const std::string &path) const override;
  bool CreateDir(const std::string &path) override;
  bool CreateFile(const std::string &path) override;
  int32_t GetModificationAge(const std::string &path) const override;
  void Touch(const std::string &path) override;
  void WalkDir(const std::string &dpath,
               std::function<void(const PathStat &)> callback,
               int32_t max_depth) const override;
  int32_t GetFileSize(const std::string &path) const override;
  std::string GetFileContents(const std::string &path) const override;
  bool MoveFile(const std::string &path_from,
                const std::string &path_to) override;
  bool IsOpenForWrite(const std::string &path) const override;
  void OpenForWrite(const std::string &path) override;
  bool Append(const std::string &path, const std::string &str) override;
  void Close(const std::string &path) override;
  bool DeleteDir(const std::string &path) override;
  bool DeleteFile(const std::string &path) override;
  int64_t GetFreeSpace(const std::string &path) const override;

 private:
  std::unordered_map<std::string, FILE *> open_files_;
};

}  // namespace profiler

#endif  // UTILS_FS_DISK_FILE_SYSTEM_H_
