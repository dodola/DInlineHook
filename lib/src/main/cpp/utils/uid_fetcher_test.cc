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
#include "uid_fetcher.h"
#include "test/utils.h"

#include <gtest/gtest.h>

using profiler::UidFetcher;
using profiler::TestUtils;

TEST(GetUidStringFromPidFile, UidFoundAfterPrefix) {
  std::string file_name(
    TestUtils::getUtilsTestData("uid_found_after_prefix.txt"));
  std::string content;
  EXPECT_TRUE(UidFetcher::GetUidStringFromPidFile(file_name, &content));
  EXPECT_EQ("10023", content);
}

TEST(GetUidStringFromPidFile, UidFoundAfterPrefixAndEmptySpaces) {
  std::string file_name(
    TestUtils::getUtilsTestData("uid_found_after_prefix_and_spaces.txt"));
  std::string content;
  EXPECT_TRUE(UidFetcher::GetUidStringFromPidFile(file_name, &content));
  EXPECT_EQ("10023", content);
}

TEST(GetUidStringFromPidFile, UidNotFoundAsPrefixIsMissing) {
  std::string file_name(TestUtils::getUtilsTestData("uid_missing.txt"));
  std::string content;
  EXPECT_FALSE(UidFetcher::GetUidStringFromPidFile(file_name, &content));
}

TEST(GetUidStringFromPidFile, UidNotFoundAsNegativeNumber) {
  std::string file_name(TestUtils::getUtilsTestData("uid_negative_number.txt"));
  std::string content;
  EXPECT_FALSE(UidFetcher::GetUidStringFromPidFile(file_name, &content));
}

TEST(GetUidStringFromPidFile, UidNotFoundAsLetters) {
  std::string file_name(TestUtils::getUtilsTestData("uid_invalid_letters.txt"));
  std::string content;
  EXPECT_FALSE(UidFetcher::GetUidStringFromPidFile(file_name, &content));
}
