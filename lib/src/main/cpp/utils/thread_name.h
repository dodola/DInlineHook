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
#ifndef UTILS_THREAD_NAME_H_
#define UTILS_THREAD_NAME_H_

#include <string>

namespace profiler {

// Sets the calling thread's name to |name|. Returns true on success.
// On Android, the underlying thread name's length is restricted to 16
// characters, including the terminating null byte ('\0'). Therefore,
// if |name| is longer than 15 characters, only the first 15 will be used.
bool SetThreadName(const std::string& name);

}  // namespace profiler

#endif  // UTILS_THREAD_NAME_H_
