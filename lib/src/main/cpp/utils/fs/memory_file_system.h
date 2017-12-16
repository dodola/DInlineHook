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
#include "utils/fs/file_system.h"

#include <set>
#include <unordered_map>

#include "utils/clock.h"

namespace profiler {

// A FileSystem implementation that stores files and directories in memory only.
// This class can be useful when you want a lightweight file system that doesn't
// persist across application runs. It is also helpful for unit tests.
//
// Note: This class is only used for unit tests at the moment. If this ever
// needs to be used in production, consider changing the data structures from
// flat maps to a more appropriate tree structure.
class MemoryFileSystem final : public FileSystem {
 public:
  MemoryFileSystem();
  explicit MemoryFileSystem(const std::shared_ptr<Clock> &clock);

  ~MemoryFileSystem() override = default;

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
  class FileData final {
   public:
    std::string contents;
    bool in_write_mode;
  };

  mutable std::set<std::string> dirs_;
  mutable std::unordered_map<std::string, FileData> files_;
  mutable std::unordered_map<std::string, int32_t> timestamps_;
  std::shared_ptr<Clock> clock_;
};
}  // namespace profiler
