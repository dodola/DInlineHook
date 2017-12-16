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
#include "trie.h"

#include <vector>

#include <gtest/gtest.h>

using profiler::Trie;

TEST(Trie, IncrementalAdd) {
  Trie<int> trie;

  std::vector<int> values;
  for (int i = 1; i < 6; i++) {
    values.push_back(i);
    auto result = trie.insert(values);
    EXPECT_TRUE(result.second);
    EXPECT_EQ(result.first, i);
    EXPECT_EQ(trie.getValue(result.first), i);
    EXPECT_EQ(trie.getParent(result.first), i - 1);
    EXPECT_EQ(trie.length(), i + 1);
  }
}

TEST(Trie, MultiRootMultiChildren) {
  Trie<int> trie;

  std::vector<int> values1{1, 2};
  std::vector<int> values2{1, 3};
  std::vector<int> values3{11, 12};
  std::vector<int> values4{11, 13};

  auto result = trie.insert(values1);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 2);
  EXPECT_EQ(trie.getValue(result.first), 2);
  EXPECT_EQ(trie.getParent(result.first), 1);
  EXPECT_EQ(trie.length(), 3);

  result = trie.insert(values2);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 3);
  EXPECT_EQ(trie.getValue(result.first), 3);
  EXPECT_EQ(trie.getParent(result.first), 1);
  EXPECT_EQ(trie.length(), 4);

  result = trie.insert(values3);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 5);
  EXPECT_EQ(trie.getValue(result.first), 12);
  EXPECT_EQ(trie.getParent(result.first), 4);
  EXPECT_EQ(trie.length(), 6);

  result = trie.insert(values4);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 6);
  EXPECT_EQ(trie.getValue(result.first), 13);
  EXPECT_EQ(trie.getParent(result.first), 4);
  EXPECT_EQ(trie.length(), 7);
}

TEST(Trie, AddSubPath) {
  Trie<int> trie;

  std::vector<int> values1{1, 2, 3, 4};
  std::vector<int> values2{1, 2};

  auto result = trie.insert(values1);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 4);
  EXPECT_EQ(trie.getValue(result.first), 4);
  EXPECT_EQ(trie.getParent(result.first), 3);
  EXPECT_EQ(trie.length(), 5);

  result = trie.insert(values2);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 2);
  EXPECT_EQ(trie.getValue(result.first), 2);
  EXPECT_EQ(trie.getParent(result.first), 1);
  EXPECT_EQ(trie.length(), 5);

  // Check that readding the sub-path does nothing
  result = trie.insert(values2);
  EXPECT_FALSE(result.second);
  EXPECT_EQ(result.first, 2);
  EXPECT_EQ(trie.getValue(result.first), 2);
  EXPECT_EQ(trie.getParent(result.first), 1);
  EXPECT_EQ(trie.length(), 5);
}

TEST(Trie, AddDuplicates) {
  Trie<int> trie;

  std::vector<int> values{1, 2, 3, 4, 5};
  auto result = trie.insert(values);
  EXPECT_TRUE(result.second);
  EXPECT_EQ(result.first, 5);
  EXPECT_EQ(trie.getValue(result.first), 5);
  EXPECT_EQ(trie.getParent(result.first), 4);
  EXPECT_EQ(trie.length(), 6);

  result = trie.insert(values);
  EXPECT_FALSE(result.second);
  EXPECT_EQ(result.first, 5);
  EXPECT_EQ(trie.getValue(result.first), 5);
  EXPECT_EQ(trie.getParent(result.first), 4);
  EXPECT_EQ(trie.length(), 6);
}
