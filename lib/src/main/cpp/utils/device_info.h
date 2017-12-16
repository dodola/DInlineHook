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

#ifndef UTILS_DEVICE_INFO_H_
#define UTILS_DEVICE_INFO_H_

#include <string>

#include "utils/bash_command.h"

namespace profiler {

// A singleton class containing information about the running device.
class DeviceInfo {
 public:
  static const std::string& code_name() { return Instance()->code_name_; }
  static const std::string& release() { return Instance()->release_; }
  static const int sdk() { return Instance()->sdk_; }
  // Alias of sdk().
  static const int api_level() { return Instance()->sdk_; }
  static const int feature_level() { return Instance()->feature_level_; }

 private:
  DeviceInfo();

  // Returns a singleton instance.
  static DeviceInfo* Instance();

  std::string GetSystemProperty(const std::string& property_name) const;

  // 'getprop' command line is far more portable than NDK APIs such as
  // __system_property_get() across API levels.
  const BashCommandRunner getprop_;
  const std::string code_name_;
  const std::string release_;
  const int sdk_;
  const int feature_level_;
};

}  // namespace profiler
#endif  // UTILS_DEVICE_INFO_H_
