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
#ifndef UTILS_TIME_VALUE_BUFFER_H_
#define UTILS_TIME_VALUE_BUFFER_H_

#include <algorithm>
#include <mutex>
#include <vector>

#include "utils/circular_buffer.h"

namespace profiler {

// Data per sample. The time field indicates a independent time point when
// value is collected.
template <typename T>
struct TimeValue {
  int64_t time;
  T value;
};

// Data holder class of time sequential collected information. For example,
// traffic bytes sent and received information are repeated collected. It stores
// data and provides query functionality.
template <typename T>
class TimeValueBuffer {
 public:
  explicit TimeValueBuffer(size_t capacity, int pid = -1)
      : pid_(pid), values_(capacity) {}

  // Add sample value collected at a given time point.
  void Add(T value, const int64_t sample_time) {
    std::lock_guard<std::mutex> lock(values_mutex_);
    TimeValue<T> time_value{sample_time, value};
    values_.Add(time_value);
  }

  // Returns data only (without timestamps) for the given range
  // (time_from, time_to].
  std::vector<T> GetValues(const int64_t time_from,
                           const int64_t time_to) const {
    std::vector<TimeValue<T>> items = GetItems(time_from, time_to);
    std::vector<T> result(items.size());
    std::transform(items.begin(), items.end(), result.begin(),
                   [](const TimeValue<T>& t) { return t.value; });

    return result;
  }

  // Returns time values within the given range (time_from, time_to].
  std::vector<TimeValue<T>> GetItems(const int64_t time_from,
                                     const int64_t time_to) const {
    std::lock_guard<std::mutex> lock(values_mutex_);
    std::vector<TimeValue<T>> result;
    for (size_t i = 0; i < values_.size(); i++) {
      const auto& value = values_.Get(i);
      if (value.time > time_from && value.time <= time_to) {
        result.push_back(value);
      }
    }
    return result;
  }

  // Returns the number of samples stored.
  size_t GetSize() const {
    std::lock_guard<std::mutex> lock(values_mutex_);
    return values_.size();
  }

  // Returns collected sample data at given index.
  TimeValue<T> GetItem(size_t index) const {
    std::lock_guard<std::mutex> lock(values_mutex_);
    return values_.Get(index);
  }

  // Returns app id that the profiler data buffer is for.
  int pid() const { return pid_; }

  // Returns the maximum number of samples it can hold.
  size_t capacity() const { return values_.capacity(); }

 private:
  const int pid_;

  mutable std::mutex values_mutex_;
  CircularBuffer<TimeValue<T>> values_;
};

}  // namespace profiler

#endif  // UTILS_TIME_VALUE_BUFFER_H_
