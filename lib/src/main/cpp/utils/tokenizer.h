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
#ifndef UTILS_TOKENIZER_H_
#define UTILS_TOKENIZER_H_

#include <cctype>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace profiler {

// Class which helps break apart a string into tokens separated by delimiters.
// Example:
//    Tokenizer t("1 1 3 5 8 13"); // Delimiter defaults to whitespace
//    string token;
//    t.SkipTokens(4); // Skip over '1', '1', '3', and '5'
//    t.GetNextToken(&token); // token == "8"
//    t.GetNextToken(&token); // token == "13"
//    t.GetNextToken(&token); // Returns false, token still "13"
//
// Lambda support is provided for more refined parsing:
//    Tokenizer t("123+321=444");
//    string token;
//    t.GetNextToken(Tokenizer::IsDigit, &token); // 123
//    t.SkipNextChar();                           // Skip over +
//    t.GetNextToken(Tokenizer::IsDigit, &token); // 321
//    ...
//
// Like the string class, this class handles bytes independently of the encoding
// used. If the input string being tokenized contains variable-length, multi-
// byte characters, this class will be unaware, operating on one byte at a time.
class Tokenizer final {
 public:
  static bool IsAlpha(char c) { return isalpha(c) != 0; }
  static bool IsAlphaNum(char c) { return isalnum(c) != 0; }
  static bool IsDigit(char c) { return isdigit(c) != 0; }
  static bool IsLower(char c) { return islower(c) != 0; }
  static bool IsUpper(char c) { return isupper(c) != 0; }
  static bool IsWhitespace(char c) { return strchr(kWhitespace, c) != nullptr; }
  // Creates a testing function which can be used with |GetNextToken|
  // e.g. GetNextToken(Tokenizer::IsOneOf("abc"), &token);
  static std::function<bool(char)> IsOneOf(const char *chars) {
    return [chars](char c) { return strchr(chars, c) != nullptr; };
  }

  // Returns a list of tokens by splitting |input| by |delimiters|.
  //
  // When |start_token_index| is specified, we skip that many tokens. The final
  // results include only tokens after that index, if any; otherwise, no token
  // is skipped.
  //
  // For example, with |input| = "1 2 3", |start_token_index| = 1, and
  // |delimiter| = " ", then the result is {"2", "3"}.
  //
  // If |max_token_count| is specified, at most |max_token_count| tokens are
  // returned; otherwise, the results are not limited.
  //
  // For example, with |input| = "1 2 3", |start_token_index| = 1,
  // |delimiter| = " ", and |max_token_count| = 1, then the result is {"2"}.
  static std::vector<std::string> GetTokens(
      const std::string &input, const std::string &delimiters,
      const int32_t start_token_index = 0,
      const int32_t max_token_count = INT32_MAX);

  // Create a tokenizer that wraps an input string. By default, it will use
  // whitespace as a delimiter, but you can instead optionally specify a string
  // that contains one (or more) delimiters. If multiple delimiters are
  // specified, each one of them can separately indicate a token boundary.
  explicit Tokenizer(const std::string &input,
                     const std::string &delimiters = kWhitespace)
      : input_(input), delimiters_(delimiters), index_(0) {}

  // Get the next token in the input text, where a token is text surrounded by
  // delimiters. If found, return true and set |token| to its value. Otherwise,
  // |token| will be left unset.
  //
  // This method will skip over any leading delimiters first. If this causes the
  // index to move to the end of the input string, then this method returns
  // false and leaves |token| unset.
  //
  // After this method is called, the index will be positioned after the end of
  // the token.
  //
  // If you don't care about the token result, use |SkipNextToken| instead.
  bool GetNextToken(std::string *token);

  // Like |GetNextToken(token)|, but which uses a custom lambda method to
  // pull out the token (which, here, does not rely on delimiters, but is the
  // longest substring where the |is_valid_char| function returns true on each
  // character).
  bool GetNextToken(std::function<bool(char)> is_valid_char,
                    std::string *token);

  // Get the next character in the input text and return true, unless we're
  // already at the end of the text, at which point false will be returned and
  // |c| will be left unset.
  //
  // This method doesn't take delimiters into account and will return the next
  // character even if it is a delimiter.
  //
  // After this method is called, the index will be positioned one character
  // forward.
  //
  // If you don't care about the character result, use |SkipNextChar| instead.
  bool GetNextChar(char *c);

  // Convenience method for calling |GetNextToken| when you don't care about the
  // token result.
  bool SkipNextToken() { return GetNextToken(nullptr); }

  // Convenience method for calling |GetNextToken| when you don't care about the
  // token result.
  bool SkipNextToken(std::function<bool(char)> is_valid_char) {
    return GetNextToken(is_valid_char, nullptr);
  }

  // Convenience method for calling |GetNextChar| when you don't care about the
  // char result.
  bool SkipNextChar() { return GetNextChar(nullptr); }

  // Skip over |token_count| number of tokens, returning true if it could skip
  // over that many.
  //
  // After this method is called, the index will be positioned after the end of
  // the last token skipped.
  bool SkipTokens(int32_t token_count);

  // If the tokenizer is currently pointing at any character which is a
  // delimiter, keep skipping over until all are passed. If not pointing at a
  // delimiter, this method leaves the tokenizer at its current index.
  //
  // This method always returns true, so that it can be chained safely without
  // breaking flow, e.g. GetToken && SkipDelimeters && GetToken
  bool SkipDelimiters();

  // If the tokenizer is currently pointing at any character matched by
  // |should_skip|, keep skipping over until all are passed. If |should_skip|
  // returns false immediately, this method leaves the tokenizer at its current
  // index.
  //
  // This method always returns true, so that it can be chained safely without
  // breaking flow, e.g. GetToken && SkipWhile && GetToken
  bool SkipWhile(std::function<bool(char)> should_skip);

  // Set the tokenizer's index directly, although it will be clamped to the
  // length of the input text.
  //
  // This method is useful if you want to reset the tokenizer or start
  // tokenizing from the middle of a string.
  void set_index(size_t index) {
    index_ = index;
    if (index_ > input_.size()) {
      index_ = input_.size();
    }
  }

  size_t index() const { return index_; }
  bool done() const { return index_ == input_.size(); }

 private:
  static constexpr const char *const kWhitespace = " \t\r\n\f";

  const std::string input_;
  const std::string delimiters_;
  size_t index_;
};

}  // namespace profiler

#endif  // UTILS_TOKENIZER_H_
