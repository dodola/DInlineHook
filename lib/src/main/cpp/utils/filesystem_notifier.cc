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

#include <limits.h>
#include <poll.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace profiler {

// This array must be kep in sync with notifier.h enum "events".
int event_translation_table[] = {
    IN_CLOSE, // CLOSE
};

FileSystemNotifier::FileSystemNotifier(const std::string &path, uint32_t event_mask)
    : path_(path), file_descriptor_(-1), watch_descriptor_(-1) {
  file_descriptor_ = inotify_init();

  // Transform events to inotify events.
  event_mask_ = TranslateMask(event_mask);

  // Start watching for events contained in the event_mask_.
  watch_descriptor_ = inotify_add_watch(file_descriptor_, path_.c_str(), event_mask_ );
}

uint32_t FileSystemNotifier::TranslateMask(uint32_t event_mask) {
  uint64_t dst = 0;
  int bit = 1;
  for (int i = 0; i < sizeof(event_mask) * CHAR_BIT; i++) {
    if (bit & event_mask) {
      dst |= GetDstValue(static_cast<Event>(bit));
    }
    bit <<= 1;
  }
  return dst;
}

uint32_t FileSystemNotifier::GetDstValue(Event e) {
  switch (e) {
    case CLOSE:
      return IN_CLOSE;
  }
  return 0;
}

bool FileSystemNotifier::IsReadyToNotify() {
  return watch_descriptor_ != -1;
}

FileSystemNotifier::~FileSystemNotifier() {
  inotify_rm_watch(file_descriptor_, watch_descriptor_);
  close(file_descriptor_);
}

bool FileSystemNotifier::WaitUntilEventOccurs(int64_t timeout_ms) {
  // Use a non-blocking poll system so it will timeout if the file never
  // received the expected events.
  struct pollfd pfd = {file_descriptor_, POLLIN, 0};
  int ret = poll(&pfd, 1, timeout_ms);
  if (ret < 0) {
    return false;
  }

  // The events have been received. Time to read them.
  unsigned int available_bytes;
  ioctl(file_descriptor_, FIONREAD, &available_bytes);
  int num_events = available_bytes / sizeof(struct inotify_event);

  struct inotify_event events[num_events];
  int length = read(file_descriptor_, reinterpret_cast<char*>(events), sizeof(events));

  if (length < 0) {
    return false;
  }

  // Parse events.
  for (int i=0 ; i < num_events ; i++) {
    if (events[i].mask & event_mask_) {
      // Found one of the expected events.
      return true;
    }
  }

  // This should never happen.
  return false;
}
}  // namespace profiler