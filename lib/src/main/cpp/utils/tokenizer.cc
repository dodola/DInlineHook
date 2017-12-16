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
#include "utils/tokenizer.h"

using std::vector;
using std::string;

namespace profiler {

std::vector<std::string> Tokenizer::GetTokens(const std::string &input,
                                              const std::string &delimiters,
                                              const int32_t start_token_index,
                                              const int32_t max_token_count) {
  vector<string> tokens{};
  Tokenizer tokenizer(input, delimiters);
  if (tokenizer.SkipTokens(start_token_index)) {
    std::string token;
    int32_t count = 0;
    while (count < max_token_count && tokenizer.GetNextToken(&token)) {
      tokens.push_back(token);
      count++;
    }
  }
  return tokens;
}

bool Tokenizer::GetNextToken(std::string *token) {
  SkipDelimiters();
  return GetNextToken(
      [this](char c) { return delimiters_.find(c) == string::npos; }, token);
}

bool Tokenizer::SkipTokens(int32_t token_count) {
  int32_t remaining_count = token_count;
  while (remaining_count > 0) {
    if (!SkipNextToken()) {
      return false;
    }
    remaining_count--;
  }
  return true;
}

bool Tokenizer::GetNextToken(std::function<bool(char)> is_valid_char,
                             std::string *token) {
  size_t start = index_;
  while (index_ < input_.size() && is_valid_char(input_[index_])) {
    index_++;
  }
  size_t end = index_;

  if (end > start) {
    if (token != nullptr) {
      *token = input_.substr(start, end - start);
    }
    return true;
  } else {
    return false;
  }
}

// Get the next character in the input text, unless we're already at the end
// of the text.
bool Tokenizer::GetNextChar(char *c) {
  if (done()) {
    return false;
  }

  if (c != nullptr) {
    *c = input_[index_];
  }
  set_index(index_ + 1);

  return true;
}

bool Tokenizer::SkipDelimiters() {
  return SkipWhile(
      [this](char c) { return delimiters_.find(c) != string::npos; });
}

bool Tokenizer::SkipWhile(std::function<bool(char)> should_skip) {
  while (index_ < input_.size() && should_skip(input_[index_])) {
    index_++;
  }
  return true;
}

}  // namespace profiler
