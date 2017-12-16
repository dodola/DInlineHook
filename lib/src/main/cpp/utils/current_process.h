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

#ifndef UTILS_CURRENT_PROCESS_H_
#define UTILS_CURRENT_PROCESS_H_

#include <string>

namespace profiler {

// A singleton class containing information about the running process.
class CurrentProcess {
 public:
  // Returns the absolute path of the directory containing the process's
  // executable file. Ends with a '/'.
  static const std::string& dir() { return Instance()->dir_; }

 private:
  CurrentProcess();

  // Returns a singleton instance.
  static CurrentProcess* Instance();

  // Returns a resolved path using given an unresolved (e.g. containing path
  // modifiers such as '..') one. Ends with a '/'. Returns an empty string on
  // failure.
  std::string GetResolvedPath(const char *unresolved_path);

  // Returns the absolute path of the calling process. Ends with a '/'. Returns
  // an empty string on failure.
  std::string GetExeDir();

  // Absolute path of the directory containing the process's executable. Ends
  // with a '/'.
  std::string dir_;
};

}  // namespace profiler
#endif  // UTILS_CURRENT_PROCESS_H_
