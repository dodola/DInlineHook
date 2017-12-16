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
#ifndef UTILS_UID_FETCHER_H_
#define UTILS_UID_FETCHER_H_

#include <cstdlib>
#include <string>

#include "file_reader.h"
#include "tokenizer.h"

namespace profiler {

// Contains utils for fetching a process's uid from it's pid.
class UidFetcher {
 public:
  static bool GetUidStringFromPidFile(std::string file_path,
                                      std::string *uid_result) {
    std::string content;
    FileReader::Read(file_path, &content);

    // Find the uid value start position. It's supposed to be after the prefix,
    // also after empty spaces on the same line.
    static const char *const kUidPrefix = "Uid:";
    size_t start = content.find(kUidPrefix);
    if (start != std::string::npos) {
      // Find the uid end position, which should be empty space or new line,
      // and check the uid value contains 0-9 only.
      Tokenizer tokenizer(content, " \t");
      tokenizer.set_index(start + strlen(kUidPrefix));
      tokenizer.SkipDelimiters();
      std::string uid;
      char next_char;
      if (tokenizer.GetNextToken(Tokenizer::IsDigit, &uid) &&
          tokenizer.GetNextChar(&next_char) &&
          Tokenizer::IsWhitespace(next_char)) {
        uid_result->assign(uid);
        return true;
      }
    }
    return false;
  }

  static bool GetUidString(int pid, std::string *uid_result) {
    return GetUidStringFromPidFile(GetPidStatusFilePath(pid), uid_result);
  }

  // Returns -1 if the corresponding Uid can't be found. Note that this does a
  // file read operation to get the uid, and should not be called too frequently
  // unless necessary.
  static int GetUid(int pid) {
    std::string uid_string;
    if (GetUidString(pid, &uid_string)) {
      return atoi(uid_string.c_str());
    }
    return -1;
  }

 private:
  // Returns path of pid status file.
  // TODO Move this to a "constants.h" or something similar when it exists.
  static std::string GetPidStatusFilePath(const int pid) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/proc/%d/status", pid);
    return buffer;
  }

  UidFetcher(){};
};

}  // namespace profiler

#endif  // UTILS_UID_FETCHER_H_
