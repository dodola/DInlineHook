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
#ifndef UTILS_FS_PATH_H_
#define UTILS_FS_PATH_H_

#include <memory>
#include <string>

namespace profiler {

class Dir;
class File;
class FileSystem;

// A file system entry, a common base class for files and directories.
// As a user, you should never need to interact with this clss directly.
class Path {
 public:
  // Paths should start with a slash, end without one, and no redundant slashes
  // e.g. /a///b/c// -> /a/b/c
  static std::string Standardize(const std::string &path);

  // Remove the last entry from a standardized path string
  // e.g. /a/b/c -> /a/b, /dir/file.txt -> /dir
  // This method expects properly formatted input as is not meant for being used
  // directly. It is exposed for testing.
  static std::string StripLast(const std::string &path);

  // Append two paths together into one, e.g. /a/b/c and d/e -> /a/b/c/d/e
  static std::string Append(const std::string &root,
                            const std::string &rel_path) {
    return root + "/" + rel_path;
  }

  virtual ~Path() = default;

  // You may hold onto a handle to an path but that doesn't mean its actually
  // present in the file system. Check for actual existance on disk using this
  // method.
  virtual bool Exists() const = 0;

  // Returns the absolute path to this file or directory
  const std::string &path() const { return path_; }

  // Returns the name of this directory or file (the tail end of the path)
  const std::string &name() const { return name_; }

  // Returns the time, in seconds, since this path was last modified
  int32_t GetModificationAge() const;

  // Create this file or directory, if a file or directory is not already there.
  // Any missing parent directories will automatically be created to parent this
  // entry.
  //
  // If you want to intentionally replace a file or directory, call |Delete|
  // first (or use |Dir::NewDir| or |Dir::NewFile|)
  bool Create();

  // Delete the file or directory from the file system. Any children files will
  // also be deleted so be careful!
  bool Delete();

  // Return the parent directory for this entry. If this is the root dir, this
  // returns itself.
  const std::shared_ptr<Dir> Up() const;
  std::shared_ptr<Dir> Up();

  // Return the free space available on the disk which contains this path (or,
  // if the path does not yet exist, its first valid ancestor).
  int64_t GetFreeSpace() const;

 protected:
  Path(FileSystem *fs, const std::string &abs_path);

  // Normally, directory creation is limited to within the file system under
  // root. However, this internal method works directly with the underlying
  // disk and ignores that restriction.
  bool CreateDirsRecursively(const std::string &abs_path);

  // Delegate dir/file creation to subclass
  virtual bool HandleCreate() = 0;

  // Delegate dir/file deletion to subclass
  virtual bool HandleDelete() = 0;

  FileSystem *fs_;
  std::string path_;
  std::string name_;

 private:
  std::shared_ptr<Dir> DoUp() const;
};

}  // namespace profiler

#endif  // UTILS_FS_PATH_H_
