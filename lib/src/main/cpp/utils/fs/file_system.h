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
#ifndef UTILS_FS_FILE_SYSTEM_H_
#define UTILS_FS_FILE_SYSTEM_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "utils/fs/dir.h"
#include "utils/fs/file.h"

namespace profiler {

// Data class which holds lightweight metadata for a target file or directory
// path, which can be used to construct a |File| or |Dir| object if needed.
class PathStat {
 public:
  enum class Type {
    FILE,
    DIR,
  };

  PathStat(Type type, const std::string &root, const std::string &full_path,
           int32_t size_b, int32_t modification_age_s)
      : type_(type),
        full_path_(full_path),
        size_b_(size_b),
        modification_age_s_(modification_age_s) {
    rel_path_ = full_path.substr(root.length() + 1);
  }

  Type type() const { return type_; }

  // Returns the full path of this file.
  // e.g. if walking /root/dir/ and coming across /root/dir/subdir/file.txt,
  // full_path will be "/root/dir/subdir/file.txt"
  const std::string &full_path() const { return full_path_; }

  // Returns the path of this file, relative to the directory being walked.
  // e.g. if walking /root/dir/ and coming across /root/dir/subdir/file.txt,
  // rel_path will be "subdir/file.txt"
  const std::string &rel_path() const { return rel_path_; }

  // Returns the size, in bytes, of this file system entry
  int32_t size() const { return size_b_; }

  // Returns the time, in seconds, since this file was last modified
  int32_t modification_age() const { return modification_age_s_; }

 private:
  Type type_;
  std::string full_path_;
  std::string rel_path_;
  int32_t size_b_;
  int32_t modification_age_s_;
};

// A mockable file system providing basic file operations
// Example:
//  // Declaring a file system instance
//  DiskFileSystem fs;
//  auto root = fs.GetOrNewDir("/path/to/app");
//
//  // Reading files
//  auto settings = root->GetOrNewFile(".appsettings");
//  string contents = settings->Contents();
//  ...
//
//  // Working with directories
//  auto cache = root->GetDir("cache")->Delete(); // Delete old cache
//  root->NewDir("cache/images");
//  root->NewDir("cache/movies");
//  // Creating subdirs should automatically create parent dir
//  assert(cache->Exists());
//
//  // Editing files
//  auto log_file = root->GetOrNewFile("logs/cache.log");
//  *log_file << "Cache modified at " << clock.GetCurrentTime() << endl;
//
// The FileSystem class is NOT thread safe so be careful when modifying
// directories and files across threads.
class FileSystem {
  // Expose access to protected utility methods
  friend File;
  friend Dir;
  friend Path;

 public:
  virtual ~FileSystem() = default;

  // Fetch a directory handle for the specified path.
  std::shared_ptr<Dir> GetDir(const std::string &abs_path);

  // Fetch a directory handle for the specified path, ensuring it is created.
  // If a directory already exists here it will be overwritten.
  std::shared_ptr<Dir> NewDir(const std::string &abs_path);

  // Fetch a directory handle for the specified path, creating it only if it
  // does not already exist.
  std::shared_ptr<Dir> GetOrNewDir(const std::string &abs_path);

  // Fetch a file handle for the specified path.
  std::shared_ptr<File> GetFile(const std::string &abs_path);

  // Fetch a file handle for the specified path, ensuring it is created. If a
  // file already exists here it will be overwritten.
  std::shared_ptr<File> NewFile(const std::string &abs_path);

  // Fetch a file handle for the specified path, creating it only if it does not
  // already exist.
  std::shared_ptr<File> GetOrNewFile(const std::string &abs_path);

 protected:
  virtual bool HasDir(const std::string &dpath) const = 0;

  virtual bool HasFile(const std::string &fpath) const = 0;

  // Create a new directory. A directory should not already exist at this
  // location when this method is called.
  //
  // This method will fail if the necessary parent directories to create this
  // directory don't already exist; the caller should ensure they do.
  virtual bool CreateDir(const std::string &dpath) = 0;

  // Create a new file. A file should not already exist at this location when
  // this method is called.
  //
  // This method will fail if the necessary parent directories to create this
  // directory don't already exist; the caller should ensure they do.
  virtual bool CreateFile(const std::string &fpath) = 0;

  // Return the time passed, in seconds, since the target path was modified.
  virtual int32_t GetModificationAge(const std::string &path) const = 0;

  // Update the target file's modified timestamp. Caller should ensure the file
  // exists. This method does NOT create a file if it doesn't already exist.
  virtual void Touch(const std::string &fpath) = 0;

  // Given a path to a directory, walk over its contents, triggering the
  // callback for each file. The callback will be triggered in an order where
  // the paths can be safely deleted (i.e. children first).
  //
  // This method also takes a depth parameter, where a |max_depth| of 1 walks
  // contents of this directory, |max_depth| of 2 also includes contents of its
  // subdirectories, |max_depth| of 3 also includes the contents of those
  // subdirectories' subdirectories, etc. A |max_depth| of 0 does nothing. Pass
  // in |INT32_MAX| to walk all children.
  virtual void WalkDir(const std::string &dpath,
                       std::function<void(const PathStat &)> callback,
                       int32_t max_depth) const = 0;

  // Return a file's size, in bytes.
  virtual int32_t GetFileSize(const std::string &fpath) const = 0;

  // Read a file's contents all in one pass. This will return the empty string
  // if the file at the target path is in write mode.
  virtual std::string GetFileContents(const std::string &fpath) const = 0;

  // Move the file from the first path to the second path. The caller should
  // ensure the first file is not in write mode and that the second file either
  // doesn't exist or is also not in write mode. The caller should also not
  // call this method with the same path for both arguments.
  virtual bool MoveFile(const std::string &fpath_from,
                        const std::string &fpath_to) = 0;

  // Returns true if the file is in write mode. See also |OpenWriteMode| and
  // |Close|
  virtual bool IsOpenForWrite(const std::string &fpath) const = 0;

  // Put a file into write mode. The file stays in write mode until |Close| is
  // called.
  virtual void OpenForWrite(const std::string &fpath) = 0;

  // Append text to the end of the file at the specified path. This should not
  // be called if the file is not already in write mode.
  virtual bool Append(const std::string &fpath, const std::string &str) = 0;

  // Indication that user is done writing to a file after calling
  // |OpenWriteMode|.
  virtual void Close(const std::string &fpath) = 0;

  // Remove a directory and all of its contents recursively.
  virtual bool DeleteDir(const std::string &dpath) = 0;

  // Remove a file.
  virtual bool DeleteFile(const std::string &fpath) = 0;

  // Return the free space available on the disk which contains the target path.
  // The path must exist, or this will return 0.
  virtual int64_t GetFreeSpace(const std::string &path) const = 0;
};

}  // namespace profiler

#endif  // UTILS_FS_FILE_SYSTEM_H_
