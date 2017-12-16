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
#include "utils/current_process.h"
#include <gtest/gtest.h>
#include <string>

using profiler::CurrentProcess;
using std::string;

TEST(CurrentProcess, GetDir) {
  string output = CurrentProcess::dir();
  int length = output.length();
  // The output is expected to end with "/".
  EXPECT_EQ("/", output.substr(length - 1));
  // This test is under "utils/" directory. The test process's executable
  // is expected to be under a directory of the same name.
  const string kExpectedTail{"/utils/"};
  EXPECT_STREQ(kExpectedTail.c_str(),
               output.substr(length - kExpectedTail.length()).c_str());
}
