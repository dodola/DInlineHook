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
#include "producer_consumer_queue.h"

#include <deque>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "utils/count_down_latch.h"

using profiler::ProducerConsumerQueue;
using profiler::CountDownLatch;
using std::thread;
using std::vector;

TEST(ProducerConsumerQueue, CommunicatesAcrossThreads) {
  ProducerConsumerQueue<int32_t> q;
  auto producer = thread([&]() {
    q.Push(1);
    q.Push(2);
    q.Push(3);
    q.Push(4);
    q.Finish();
  });

  auto consumer = thread([&]() {
    int val;
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 4);
    EXPECT_FALSE(q.Pop(&val));
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(0, q.Drain().size());
}

TEST(ProducerConsumerQueue, CanConsumeFromMultipleThreads) {
  ProducerConsumerQueue<int32_t> q;
  auto producer = thread([&]() {
    q.Push(1);
    q.Push(2);
    q.Push(3);
    q.Push(4);
    q.Push(5);
    q.Push(6);
  });

  vector<thread> threads;
  for (int i = 0; i < 3; i++) {
    threads.push_back(thread([&]() {
      int val;
      EXPECT_TRUE(q.Pop(&val));
      EXPECT_TRUE(q.Pop(&val));
    }));
  }

  std::for_each(threads.begin(), threads.end(), [](thread &t) { t.join(); });
  producer.join();

  EXPECT_EQ(0, q.Drain().size());
}

TEST(ProducerConsumerQueue, CanProduceFromMultipleThreads) {
  ProducerConsumerQueue<int32_t> q;
  CountDownLatch done_producing_(3);

  auto consumer = thread([&]() {
    int val;
    EXPECT_TRUE(q.Pop(&val));  // 1
    EXPECT_TRUE(q.Pop(&val));  // 2
    EXPECT_TRUE(q.Pop(&val));  // 3
    EXPECT_TRUE(q.Pop(&val));  // 4
    EXPECT_TRUE(q.Pop(&val));  // 5
    EXPECT_TRUE(q.Pop(&val));  // 6
    EXPECT_FALSE(q.Pop(&val));
  });

  vector<thread> threads;
  for (int i = 0; i < 3; i++) {
    threads.push_back(thread([&]() {
      int thread_index = (2 * i) + 1;  // (1 and 2) or (3 and 4) or (5 and 6)
      q.Push(thread_index);
      q.Push(thread_index + 1);
      done_producing_.CountDown();
    }));
  }
  threads.push_back(thread([&]() {
    done_producing_.Await();
    q.Finish();
  }));

  std::for_each(threads.begin(), threads.end(), [](thread &t) { t.join(); });
  consumer.join();

  EXPECT_EQ(0, q.Drain().size());
}

TEST(ProducerConsumerQueue, ValuesAfterFinishAreIgnored) {
  ProducerConsumerQueue<int32_t> q;
  auto producer = thread([&]() {
    EXPECT_TRUE(q.Push(1));
    EXPECT_TRUE(q.Push(2));
    EXPECT_TRUE(q.Push(3));
    q.Finish();
    EXPECT_FALSE(q.Push(4));
    EXPECT_FALSE(q.Push(5));
    EXPECT_FALSE(q.Push(6));
  });

  auto consumer = thread([&]() {
    int val;
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, 3);
    EXPECT_FALSE(q.Pop(&val));
  });

  producer.join();
  consumer.join();

  EXPECT_EQ(0, q.Drain().size());
}

TEST(ProducerConsumerQueue, QueueIsBounded) {
  ProducerConsumerQueue<int32_t> q(5);
  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(q.Push(i));
  }

  int val;
  for (int i = 5; i < 10; i++) {
    EXPECT_TRUE(q.Pop(&val));
    EXPECT_EQ(val, i);
  }

  EXPECT_EQ(0, q.Drain().size());
}

TEST(ProducerConsumerQueue, Drain) {
  ProducerConsumerQueue<int32_t> q;
  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(q.Push(i));
  }

  std::deque<int32_t> tmp(q.Drain());
  EXPECT_EQ(10, tmp.size());
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(tmp.front(), i);
    tmp.pop_front();
  }

  EXPECT_EQ(0, q.Drain().size());
}
