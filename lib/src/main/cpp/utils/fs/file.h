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
#ifndef UTILS_FS_FILE_H_
#define UTILS_FS_FILE_H_

#include <memory>
#include <sstream>
#include <string>

#include "utils/fs/path.h"

namespace profiler {

class FileSystem;

// A handle to a file location. The file may or may not exist; use |Exists| to
// check and |Create| to actually create it.
class File final : public Path {
  friend FileSystem;  // So FileSystem can create File

 public:
  virtual ~File();

  bool Exists() const override;

  // Update the timestamp of this file. If the file does not exist, this method
  // does NOT create it. Instead, use |Create| if that's your intention.
  void Touch();

  // If this file exists, return its contents (else, the empty string). This
  // method will return the empty string if the file is currently open for
  // write.
  std::string Contents() const;

  // If this file exists and is not in write mode, and the target file doesn't
  // exist or it exists but isn't in write mode, move the contents of this file
  // to that file. After this operation is complete, this file will be deleted.
  //
  // After the move, this instance will still point at the old location (and
  // |Exists| will return false). Only its contents have moved.
  //
  // Note: Technically, instead of moving, you could copy:
  //  dest->Create();
  //  dest->OpenForWrite();
  //  dest->Append(src->Contents());
  //  dest->Close();
  //  src->Delete();
  // but the move operation is provided as a separate method since it can be
  // much more lightweight.
  bool MoveContentsTo(std::shared_ptr<File> dest);

  // Return true if this file is opened for write, at which point you can
  // safely append more data. You cannot read from a file that is open for
  // write.
  bool IsOpenForWrite() const;

  // Put this file into write mode, during which time you can append to its
  // contents. When you are done writing, you should call |Close|.
  void OpenForWrite();

  // If this file exists, append the input string
  void Append(const std::string &str);

  // Close a file that is currently in write mode.
  void Close();

  // Support the << operator for cout style |Append|ing
  // e.g. file << "This appends to the end of the file";
  template <typename T>
  File &operator<<(T &&value) {
    std::ostringstream oss;
    oss << value;
    Append(oss.str());
    return *this;
  }

 protected:
  // Don't create directly. Use |Dir::GetFile| or |Dir::NewFile| from a parent
  // directory instead.
  File(FileSystem *fs, const std::string &abs_path);

  bool HandleCreate() override;
  bool HandleDelete() override;
};

}  // namespace profiler

#endif  // UTILS_FS_FILE_SYSTEM_H_
