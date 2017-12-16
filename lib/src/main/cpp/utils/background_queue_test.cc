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

#include <gtest/gtest.h>
#include "utils/count_down_latch.h"

using profiler::BackgroundQueue;
using profiler::CountDownLatch;

TEST(BackgroundQueue, EnqueuingTasksWorks) {
  CountDownLatch job_1_waiting(1);
  CountDownLatch job_1_finished(1);
  CountDownLatch job_2_waiting(1);
  CountDownLatch job_2_finished(1);

  int val = 0;
  BackgroundQueue bq("BQTestThread");
  bq.EnqueueTask([&] {
    job_1_waiting.Await();
    val = 1;
    job_1_finished.CountDown();
  });
  bq.EnqueueTask([&] {
    job_2_waiting.Await();
    val = 2;
    job_2_finished.CountDown();
  });

  EXPECT_EQ(0, val);
  job_1_waiting.CountDown();
  job_1_finished.Await();
  EXPECT_EQ(1, val);
  job_2_waiting.CountDown();
  job_2_finished.Await();
  EXPECT_EQ(2, val);
}

TEST(BackgroundQueue, DestructorBlocksUntilJobsFinish) {
  const int kNumJobs = 12345;
  CountDownLatch first_job_started(1);
  int num_jobs_run = 0;

  {
    BackgroundQueue bq("BQTestThread");
    bq.EnqueueTask([&] { first_job_started.Await(); });
    for (int i = 0; i < kNumJobs; ++i) {
      bq.EnqueueTask([&] { ++num_jobs_run; });
    }
    first_job_started.CountDown();
    EXPECT_NE(kNumJobs, num_jobs_run);
  }  // Blocked here until all enqueued tasks run
  EXPECT_EQ(kNumJobs, num_jobs_run);
}

TEST(BackgroundQueue, QueueMaxLengthIsRespected) {
  CountDownLatch job_1_started(1);
  CountDownLatch job_1_waiting(1);
  CountDownLatch job_2_finished(1);
  CountDownLatch job_3_finished(1);
  CountDownLatch job_4_finished(1);
  CountDownLatch job_5_finished(1);
  CountDownLatch job_6_finished(1);

  {
    // Queue has max length of 3. We'll add one task, run it, and block it,
    // leaving an empty queue again. At that point, we'll add 5 more tasks.
    // Since we can only have three enqueued tasks max, the first two should get
    // dropped.
    BackgroundQueue bq("BQTestThread", 3);
    bq.EnqueueTask([&] {
      job_1_started.CountDown();
      job_1_waiting.Await();
    });

    job_1_started.Await();
    bq.EnqueueTask([&] { job_2_finished.CountDown(); });  // Will be removed
    bq.EnqueueTask([&] { job_3_finished.CountDown(); });  // Will be removed
    bq.EnqueueTask([&] { job_4_finished.CountDown(); });
    bq.EnqueueTask([&] { job_5_finished.CountDown(); });
    bq.EnqueueTask([&] { job_6_finished.CountDown(); });

    job_1_waiting.CountDown();
  }

  EXPECT_FALSE(job_2_finished.count() == 0);
  EXPECT_FALSE(job_3_finished.count() == 0);
  EXPECT_TRUE(job_4_finished.count() == 0);
  EXPECT_TRUE(job_5_finished.count() == 0);
  EXPECT_TRUE(job_6_finished.count() == 0);
}
