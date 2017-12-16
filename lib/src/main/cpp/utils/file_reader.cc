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
#include "file_reader.h"

#include <fcntl.h>
#include <unistd.h>
#include <sstream>

namespace profiler {

bool FileReader::Read(const std::string &file_path,
                      std::vector<std::string> *lines) {
  std::string content;
  if (Read(file_path, &content)) {
    std::stringstream stream(content);
    std::string line;
    // Check if the last line with no eol-char trailing works
    while (std::getline(stream, line)) {
      lines->push_back(line);
    }
    return true;
  }
  return false;
}

bool FileReader::Read(const std::string &file_path, std::string *content) {
  int file = open(file_path.c_str(), O_RDONLY);
  if (file == -1) {
    return false;
  }
  char buffer[kBufferSize_];
  off_t offset = 0;
  ssize_t read_size;
  content->erase();
  while ((read_size = pread(file, buffer, kBufferSize_, offset)) > 0) {
    offset += read_size;
    content->append(buffer, read_size);
  }
  close(file);
  return true;
}

}  // namespace profiler
