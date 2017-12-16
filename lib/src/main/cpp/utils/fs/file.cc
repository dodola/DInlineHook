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
#include "file.h"

#include "utils/fs/file_system.h"

namespace profiler {

using std::shared_ptr;
using std::string;

File::File(FileSystem *fs, const string &abs_path) : Path(fs, abs_path) {}

File::~File() { Close(); }

bool File::Exists() const { return fs_->HasFile(path_); }

void File::Touch() {
  if (Exists()) {
    fs_->Touch(path_);
  }
}

string File::Contents() const {
  if (Exists() && !IsOpenForWrite()) {
    return fs_->GetFileContents(path_);
  } else {
    return "";
  }
}

bool File::MoveContentsTo(shared_ptr<File> dest) {
  if (!Exists()) {
    return false;
  }
  if (IsOpenForWrite()) {
    return false;
  }
  if (dest->IsOpenForWrite()) {
    return false;
  }
  if (path_ == dest->path()) {
    return true;
  }

  dest->Delete();
  return fs_->MoveFile(path_, dest->path());
}

bool File::IsOpenForWrite() const { return fs_->IsOpenForWrite(path_); }

void File::OpenForWrite() {
  if (Exists()) {
    fs_->OpenForWrite(path_);
  }
}

void File::Append(const string &str) {
  if (IsOpenForWrite()) {
    fs_->Append(path_, str);
  }
}

void File::Close() {
  if (IsOpenForWrite()) {
    fs_->Close(path_);
  }
}

bool File::HandleCreate() { return fs_->CreateFile(path_); }

bool File::HandleDelete() { return fs_->DeleteFile(path_); }
}  // namespace profiler
