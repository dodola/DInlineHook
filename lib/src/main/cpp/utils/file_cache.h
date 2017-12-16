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
#ifndef UTILS_FILE_CACHE_H_
#define UTILS_FILE_CACHE_H_

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "utils/fs/file_system.h"

namespace profiler {

// Class for storing files on disk into a cache with a shortish cleanup period
// (since disk space on an average mobile device is magnitudes smaller than your
// standard PC).
//
// The cache will get cleaned up periodically by a background thread and will
// not outlive separate runs of the application. In other words, creating a
// new cache will nuke any contents it may find from before.
//
// The cache will automatically create two subdirectores - "partial" and
// "complete". As bytes are streamed into the cache, a corresponding file is
// built up in the "partial" subfolder. When streaming is finished, the complete
// file is moved to the "complete" subfolder, at which points it becomes a
// candidate for eventual cache cleanup.
//
// Example:
//    // Streaming data into the cache
//    FileCache cache;
//    cache.AddChunk("id", "123");
//    cache.AddChunk("id", "456");
//    auto file = cache.Complete("id"); // file->Contents() == "123456"
//
// This class is not thread safe. You must be careful when modifying this cache
// across multiple threads.
class FileCache final {
 public:
  FileCache();
  FileCache(std::unique_ptr<FileSystem> fs);
  ~FileCache();

  FileCache(const FileCache &) = delete;
  FileCache &operator=(const FileCache &) = delete;

  // Repeatedly call this to add chunks of data to be appended to a file
  // named after the passed in |cache_id|. If no file exists yet, a new one will
  // be created. Call |Abort| to cancel or |Complete| when done.
  //
  // |cache_id| is case sensitive and must be a valid file name. For example,
  // "../../myid" could cause unexpected behavior.
  //
  // Finally, it's up to the caller to ensure they choose a unique name, or else
  // they will overwrite something else in the cache.
  void AddChunk(const std::string &cache_id, const std::string &chunk);

  // Delete the cached file associated with the |cache_id|, if you were
  // calling |AddChunk| but want to cancel.
  void Abort(const std::string &cache_id);

  // Notify the cache that the file associated with |cache_id| is now
  // complete, having called |AddChunk| enough times. This will put the
  // contents of the file into a final location, returning it.
  std::shared_ptr<File> Complete(const std::string &cache_id);

  // Return the cached file associated with the |cache_id|. While the pointer
  // returned will never be null, the file may not exist, if |Complete|
  // wasn't called first, or if the cleanup thread has since deleted the file.
  std::shared_ptr<File> GetFile(const std::string &cache_id);

 private:
  // While running, periodically walks the cache and removes old files
  void JanitorThread();

  // Max size we allow this cache to grow to, in bytes. Note: The cache may
  // temporarily go over this limit, but it will be trimmed down the next time
  // the |JanitorThread| runs a cleanup pass.
  int64_t size_limit_b;

  // File system for storing cached files
  std::unique_ptr<FileSystem> fs_;
  std::shared_ptr<Dir> cache_partial_;
  std::shared_ptr<Dir> cache_complete_;

  std::atomic_bool is_janitor_running_;
  std::thread janitor_thread_;
};

}  // namespace profiler

#endif  // UTILS_FILE_CACHE_H_
