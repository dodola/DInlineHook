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
#ifndef UTILS_BACKGROUND_QUEUE_H
#define UTILS_BACKGROUND_QUEUE_H

#include <atomic>
#include <string>
#include <thread>

#include "producer_consumer_queue.h"

namespace profiler {

// A thread-safe queue of tasks which will be run sequentially on a background
// thread. The queue can also be reset, which will clear it and remove any
// enqueued tasks that haven't run yet.
//
// Example:
//   {
//     BackgroundQueue bq("LongTasks");
//     bq.EnqueueTask([]() { ... long operation #1 ... })
//     bq.EnqueueTask([]() { ... long operation #2 ... })
//   } // bq will not be destroyed until tasks finish running
class BackgroundQueue {
 public:
  // Create a background queue with an optional max length. If |max_length| is
  // not specified, the queue can grow unbounded, constrained only by memory.
  // Otherwise, the number of simultaneously enqueued background tasks will be
  // limited, with older tasks removed to make way for newly enqueued ones. The
  // task that is currently running does not count against this length.
  //
  // Note that a max length of 0 is not supported and treated as an error. This
  // should either be set to a positive integer or left unset.
  explicit BackgroundQueue(std::string thread_name, int32_t max_length = -1);
  ~BackgroundQueue();

  // Add a task to the end of the queue. It will automatically be run after all
  // prior tasks finish; in other words, tasks are not run simultaneously.
  // If a max number of tasks has already been enqueued, the oldest task will be
  // removed without ever being run.
  void EnqueueTask(std::function<void()> task);

 private:
  // The background method responsible for pulling the next task out of the
  // queue and running it.
  void TaskThread();

  ProducerConsumerQueue<std::function<void()>> task_queue_;
  std::thread task_thread_;
  std::string task_thread_name_;
};

}  // namespace profiler

#endif  // UTILS_BACKGROUND_QUEUE_H
