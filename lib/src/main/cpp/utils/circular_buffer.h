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
#ifndef UTILS_CIRCULAR_BUFFER_H_
#define UTILS_CIRCULAR_BUFFER_H_

#include <cstddef>
#include <memory>

namespace profiler {

// A circular buffer. This class is not thread safe.
template <typename T>
class CircularBuffer {
 public:
  explicit CircularBuffer(size_t capacity)
      : capacity_(capacity), values_(new T[capacity_]) {}

  // Add a copy of |value| and return a pointer to our copy.
  T* Add(T value) {
    size_t index = size_ < capacity_ ? size_ : start_;
    values_[index] = value;
    if (size_ < capacity_) {
      size_++;
    } else {
      start_ = (start_ + 1) % capacity_;
    }
    return &values_[index];
  }

  // Get the item at the specified |index|. This method will return an undefined
  // value if the index is out of bounds.
  T& Get(size_t index) {
    return values_[(start_ + index) % capacity_];
  }
  const T& Get(size_t index) const {
    return values_[(start_ + index) % capacity_];
  }

  // Get the last item in the buffer. This method will return an undefined value
  // if this buffer is empty.
  T& back() {
    return Get(size() - 1);
  }
  const T& back() const {
    return Get(size() - 1);
  }

  size_t size() const { return size_; }

  // Returns the maximum number of items this buffer can hold.
  size_t capacity() const { return capacity_; }

  // Convenience method for checking if the buffer is at max capacity. If |true|
  // then the next item added will overwrite the oldest item.
  bool full() const { return size_ == capacity_; }

  // Convenience method for checking if the buffer has no items added yet.
  bool empty() const { return size_ == 0; }

 private:
  const size_t capacity_;

  std::unique_ptr<T[]> values_;
  size_t size_ = 0;
  size_t start_ = 0;
};

}  // namespace profiler

#endif  // UTILS_CIRCULAR_BUFFER_H_
