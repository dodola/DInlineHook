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

#ifndef UTILS_NOTIFIER_H
#define UTILS_NOTIFIER_H

#include <string>

namespace profiler {
class FileSystemNotifier {
 public:
  // IMPORTANT: These values are used to build a bit mask:
  // Only power of two values are allowed: 3 doesn't work !
  enum Event {
    CLOSE = 1
  };

  // A monitor on file |path| listening for |event_mask_| events.
  FileSystemNotifier(const std::string &path, uint32_t event_mask);
  ~FileSystemNotifier();

  // Returns true is constructor was succesful in initializing the watch
  // for the file being monitored.
  bool IsReadyToNotify();

  // Wait until one of the events specified in |event_mask| (via constructor)
  // occurs OR at least |timeout_ms| has elapsed. Returns false if timeout or
  // if poll failed.
  bool WaitUntilEventOccurs(int64_t timeout_ms = 5000);

 private:
  // Transforme a bit mask expressed in Event space to the underlying
  // notification framework space.
  uint32_t TranslateMask(uint32_t event_mask);

  // Convert one Event value to the matching
  // underlying notification framework value.
  uint32_t GetDstValue(Event e);

  // The bitmap representing events monitored.
  uint32_t event_mask_;
  // The path_ of the file monitored.
  std::string path_;
  // The file descriptor associated with the monitor implementation.
  int file_descriptor_;
  // The file descriptor associated with the event stream: Events are read
  // from this.
  int watch_descriptor_;
};
}  // namespace profiler

#endif  // UTILS_NOTIFIER_H
