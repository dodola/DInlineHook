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

#include "filesystem_notifier.h"

namespace profiler {
FileSystemNotifier::FileSystemNotifier(const std::string &path, uint32_t event_mask)
    : event_mask_(event_mask), path_(path), file_descriptor_(-1), watch_descriptor_(-1) {
}

FileSystemNotifier::~FileSystemNotifier() {
}

// On Mac OS X, this always succeeds.
// TODO: Implement this with FSEvents from File System Events API.
bool FileSystemNotifier::IsReadyToNotify() {
  // The following uses are fake to silence 'unused-private-field' warnings.
  event_mask_ = 0;
  file_descriptor_ = 0;
  watch_descriptor_ = 0;
  return true;
}

// On Mac OS X, this always succeeds and never waits.
// TODO: Implement this with FSEvents from File System Events API.
bool FileSystemNotifier::WaitUntilEventOccurs(int64_t timeout_ms) {
  return true;
}

uint32_t FileSystemNotifier::TranslateMask(uint32_t event_mask) {
  return 0;
}

uint32_t FileSystemNotifier::GetDstValue(Event e) {
  return 0;
}
}  // namespace profiler
