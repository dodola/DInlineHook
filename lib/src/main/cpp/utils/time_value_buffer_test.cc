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
#include "time_value_buffer.h"

#include <gtest/gtest.h>

#include "utils/clock.h"

using profiler::Clock;
using profiler::TimeValue;
using profiler::TimeValueBuffer;

const int64_t t0 = Clock::s_to_ns(0);
const int64_t t1 = Clock::s_to_ns(1);
const int64_t t2 = Clock::s_to_ns(2);
const int64_t t3 = Clock::s_to_ns(3);
const int64_t t4 = Clock::s_to_ns(4);

typedef TimeValue<float> TimeFloat;
typedef TimeValueBuffer<float> TimeFloatBuffer;

TEST(TimeValueBuffer, AddDataMoreThanCapacity) {
  TimeFloatBuffer buffer(2);
  EXPECT_EQ(0u, buffer.GetSize());
  EXPECT_EQ(2u, buffer.capacity());

  buffer.Add(10, t1);
  EXPECT_EQ(1u, buffer.GetSize());
  EXPECT_EQ(t1, buffer.GetItem(0).time);
  EXPECT_EQ(10, buffer.GetItem(0).value);

  buffer.Add(20, t2);
  EXPECT_EQ(2u, buffer.GetSize());
  EXPECT_EQ(t2, buffer.GetItem(1).time);
  EXPECT_EQ(20, buffer.GetItem(1).value);

  buffer.Add(30, t3);
  EXPECT_EQ(2u, buffer.GetSize());
  EXPECT_EQ(t2, buffer.GetItem(0).time);
  EXPECT_EQ(20, buffer.GetItem(0).value);
  EXPECT_EQ(t3, buffer.GetItem(1).time);
  EXPECT_EQ(30, buffer.GetItem(1).value);
}

TEST(TimeValueBuffer, Empty) {
  TimeFloatBuffer buffer(3);
  std::vector<TimeFloat> items = buffer.GetItems(t1, t2);
  EXPECT_EQ(0u, items.size());
}

TEST(TimeValueBuffer, DataForQueryTimeRange) {
  TimeFloatBuffer buffer(2);
  buffer.Add(10, t1);
  buffer.Add(20, t2);

  std::vector<TimeFloat> items = buffer.GetItems(t1, t2);
  EXPECT_EQ(1u, items.size());
  EXPECT_EQ(t2, items.at(0).time);
  EXPECT_EQ(20, items.at(0).value);

  items.clear();
  items = buffer.GetItems(t0, t2);
  EXPECT_EQ(2u, items.size());
  EXPECT_EQ(t1, items.at(0).time);
  EXPECT_EQ(10, items.at(0).value);
  EXPECT_EQ(t2, items.at(1).time);
  EXPECT_EQ(20, items.at(1).value);
}

TEST(TimeValueBuffer, NoDataForQueryTimeRange) {
  TimeFloatBuffer buffer(2);
  buffer.Add(10, t1);
  buffer.Add(20, t2);

  std::vector<TimeFloat> items = buffer.GetItems(t3, t4);
  EXPECT_EQ(0u, items.size());
}

TEST(TimeValueBuffer, GetValuesRetrievesDataDirectly) {
  TimeFloatBuffer buffer(2);
  buffer.Add(10, t1);
  buffer.Add(20, t2);

  std::vector<float> values = buffer.GetValues(t1, t2);
  EXPECT_EQ(1u, values.size());
  EXPECT_EQ(20, values.at(0));

  values.clear();
  values = buffer.GetValues(t0, t2);
  EXPECT_EQ(2u, values.size());
  EXPECT_EQ(10, values.at(0));
  EXPECT_EQ(20, values.at(1));
}
