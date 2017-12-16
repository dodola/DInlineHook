/*
 * Copyright (C) 2017 The Android Open Source Project
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
#ifndef UTILS_COUNT_DOWN_LATCH_H_
#define UTILS_COUNT_DOWN_LATCH_H_

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace profiler {

// A thread-safe class which can be used to wait on a count being decremented to
// 0. This is a useful way for background threads to signal a parent thread that
// they're all finished.
//
// Example:
//   CountDownLatch latch(10);
//   vector<thread> threads;
//
//   for (int i = 0; i < 10; i++) {
//     threads.push_back(thread([&latch]() {
//       ... long operation ...
//       latch.CountDown();
//     }));
//   }
//
//   latch.Await(); // Blocks until all 10 threads are finished with their work
class CountDownLatch {
 public:
  explicit CountDownLatch(int32_t count)
      : original_count_(count), count_(count) {}

  // Decrement the latch value by 1. When it hits 0, any threads that called
  // |Await| will be allowed to move forward. If the count is already 0, this
  // operation is a no-op.
  void CountDown() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
      return;
    }

    count_--;
    if (count_ == 0) {
      count_down_complete_.notify_all();
    }
  }

  // Wait for the current latch count to hit 0. If the count is already 0, this
  // operation is a no-op.
  void Await() const {
    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ != 0) {
      // Conditional variables can be woken up sporadically, so we check count
      // to verify the wakeup was triggered by |CountDown|.
      count_down_complete_.wait(lock);
    }
  }

  // Resets the latch back to the count it was initialized with.
  void Reset() {
    std::unique_lock<std::mutex> lock(mutex_);
    count_ = original_count_;
  }

  int32_t count() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return count_;
  }

 private:
  int32_t original_count_;
  int32_t count_;

  mutable std::condition_variable count_down_complete_;
  mutable std::mutex mutex_;
};

}  // namespace profiler

#endif  // UTILS_COUNT_DOWN_LATCH_H_
