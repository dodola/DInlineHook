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
#include "circular_buffer.h"

#include <gtest/gtest.h>

using profiler::CircularBuffer;

TEST(CircularBuffer, AddDataMoreThanCapacity) {
  CircularBuffer<int> buffer(2);
  EXPECT_TRUE(buffer.empty());
  EXPECT_FALSE(buffer.full());
  EXPECT_EQ(0u, buffer.size());
  EXPECT_EQ(2u, buffer.capacity());

  buffer.Add(10);
  EXPECT_FALSE(buffer.empty());
  EXPECT_FALSE(buffer.full());
  EXPECT_EQ(1u, buffer.size());
  EXPECT_EQ(10, buffer.Get(0));
  EXPECT_EQ(10, buffer.back());

  buffer.Add(20);
  EXPECT_TRUE(buffer.full());
  EXPECT_EQ(20, buffer.Get(1));
  EXPECT_EQ(20, buffer.back());

  buffer.Add(30);
  EXPECT_TRUE(buffer.full());
  EXPECT_EQ(2u, buffer.size());
  EXPECT_EQ(20, buffer.Get(0));
  EXPECT_EQ(30, buffer.Get(1));
  EXPECT_EQ(30, buffer.back());
}
