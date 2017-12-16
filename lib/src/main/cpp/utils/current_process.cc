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

#include "utils/current_process.h"

#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <string>

using std::string;

namespace profiler {

CurrentProcess::CurrentProcess() { dir_ = GetExeDir(); }

CurrentProcess* CurrentProcess::Instance() {
  static CurrentProcess* instance = new CurrentProcess();
  return instance;
}

string CurrentProcess::GetResolvedPath(const char *unresolved_path) {
  // It's understood that PATH_MAX doesn't guarantee that the path will fit the
  // buffer. However, it's a fairly high value. The buffer can be increased in
  // the future if it's necessary .
  char buffer[PATH_MAX];
  // realpath() returns the canonicalized absolute pathname.
  char* real = realpath(unresolved_path, buffer);
  if (real == nullptr) {
    return std::string{}; // Returns an empty string on failure.
  }
  return std::string{dirname(buffer)} + "/";
}

}  // namespace profiler
