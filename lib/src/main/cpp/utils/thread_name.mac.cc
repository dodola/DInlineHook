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
#include "utils/thread_name.h"

#include <pthread.h>

namespace profiler {

// On OS X, pthread_setname_np() takes 1 arugment, which means a thread's name
// can be set only by the thread itself.
bool SetThreadName(const std::string& name) {
  std::string name_used{name};
  if (name.length() > 15) name_used = name.substr(0, 15);
  return (pthread_setname_np(name_used.c_str()) == 0);
}

}  // namespace profiler
