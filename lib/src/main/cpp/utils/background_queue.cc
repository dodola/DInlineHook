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
#include "background_queue.h"

#include <unistd.h>

#include "utils/clock.h"
#include "utils/thread_name.h"

using profiler::Clock;
using std::function;
using std::lock_guard;
using std::mutex;
using std::thread;
using std::string;

namespace profiler {

BackgroundQueue::BackgroundQueue(string thread_name, int max_length)
    : task_queue_(max_length), task_thread_name_(thread_name) {
  task_thread_ = thread(&BackgroundQueue::TaskThread, this);
}

BackgroundQueue::~BackgroundQueue() {
  task_queue_.Finish();
  task_thread_.join();
}

void BackgroundQueue::EnqueueTask(function<void()> task) {
  task_queue_.Push(task);
}

void BackgroundQueue::TaskThread() {
  SetThreadName(task_thread_name_);

  function<void()> task;
  while (task_queue_.Pop(&task)) {
    task();
  }
}

}  // namespace profiler
