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
#include "file_system.h"

namespace profiler {

using std::shared_ptr;
using std::string;

shared_ptr<Dir> FileSystem::GetDir(const string &abs_path) {
  // Can't use make_shared; Dir constructor is protected
  return shared_ptr<Dir>(new Dir(this, abs_path));
}

shared_ptr<Dir> FileSystem::NewDir(const string &abs_path) {
  auto dir = GetDir(abs_path);
  dir->Delete();
  dir->Create();
  return dir;
}

shared_ptr<Dir> FileSystem::GetOrNewDir(const string &abs_path) {
  auto dir = GetDir(abs_path);
  dir->Create();
  return dir;
}

shared_ptr<File> FileSystem::GetFile(const string &abs_path) {
  // Can't use make_shared; File constructor is protected
  return shared_ptr<File>(new File(this, abs_path));
}

shared_ptr<File> FileSystem::NewFile(const string &abs_path) {
  auto file = GetFile(abs_path);
  file->Delete();
  file->Create();
  return file;
}

shared_ptr<File> FileSystem::GetOrNewFile(const string &abs_path) {
  auto file = GetFile(abs_path);
  file->Create();
  return file;
}

}  // namespace profiler
