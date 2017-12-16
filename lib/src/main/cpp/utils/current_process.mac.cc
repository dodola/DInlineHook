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

#include <limits.h>
#include <mach-o/dyld.h>

using std::string;

namespace profiler {

string CurrentProcess::GetExeDir() {
  // It's understood that PATH_MAX doesn't guarantee that the path will fit the
  // buffer. However, it's a faily high value. The buffer can be increased in
  // the future if it's necessary .
  char path[PATH_MAX];
  uint32_t size = sizeof(path);

  int ret = _NSGetExecutablePath(path, &size);
  if (ret == -1) {
    return string{}; // Returns an empty string on failure.
  }
  return CurrentProcess::GetResolvedPath(path);
}

}  // namespace profiler
