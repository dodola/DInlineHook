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
#ifndef UTILS_FS_DIR_H_
#define UTILS_FS_DIR_H_

#include <functional>
#include <memory>
#include <string>

#include "utils/fs/path.h"

namespace profiler {

class File;
class FileSystem;
class PathStat;

// A handle to a directory location. The directory may or may not exist; use
// |Exists| to check and |Create| to actually create it.
class Dir final : public Path {
  friend FileSystem;  // So FileSystem can create Dir
  friend Path;        // So Path can call HandleCreate directly on any Dir

 public:
  virtual ~Dir() = default;

  // Check to see if this directory already exists
  bool Exists() const override;

  // Fetch a directory handle for the specified path.
  std::shared_ptr<Dir> GetDir(const std::string &rel_path);
  const std::shared_ptr<Dir> GetDir(const std::string &rel_path) const;

  // Fetch a directory handle for the specified path, ensuring it is created.
  // If a directory already exists in this location, it will be overwritten.
  std::shared_ptr<Dir> NewDir(const std::string &rel_path);

  // Fetch a directory handle for the specified path. If one does not already
  // exist, it will be created.
  std::shared_ptr<Dir> GetOrNewDir(const std::string &rel_path);

  // Fetch a file handle for the specified path.
  std::shared_ptr<File> GetFile(const std::string &rel_path);
  const std::shared_ptr<File> GetFile(const std::string &rel_path) const;

  // Fetch a file handle for the specified path, ensuring it is created. If a
  // file already exists in this location, it will be overwritten.
  std::shared_ptr<File> NewFile(const std::string &rel_path);

  // Fetch a file handle for the specified path. If one does not already exist,
  // it will be created.
  std::shared_ptr<File> GetOrNewFile(const std::string &rel_path);

  // Walk each file in this directory, triggering a callback for each file
  // visited. The callback will be triggered in an order where the paths can
  // safely be deleted (i.e. children first).
  //
  // An optional depth can be passed in, where a |max_depth| of 1 walks
  // contents of this directory, |max_depth| of 2 also includes contents of its
  // subdirectories, |max_depth| of 3 also includes the contents of those
  // subdirectories' subdirectories, etc. A |max_depth| of 0 does nothing. If
  // left unset, all children are walked.
  void Walk(std::function<void(const PathStat &)>,
            int32_t max_depth = INT32_MAX) const;

 protected:
  // Don't create directly. Use |GetDir| or |NewDir| from a parent directory
  // instead.
  Dir(FileSystem *fs, const std::string &abs_path);

  bool HandleCreate() override;
  bool HandleDelete() override;

 private:
  std::shared_ptr<Dir> DoGetDir(const std::string &rel_path) const;
  std::shared_ptr<File> DoGetFile(const std::string &rel_path) const;
};

}  // namespace profiler

#endif  // UTILS_FS_DIR_H_
