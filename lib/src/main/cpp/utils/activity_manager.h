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

#ifndef UTILS_PROFILER_ACTIVITYMANAGER_H
#define UTILS_PROFILER_ACTIVITYMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "bash_command.h"

namespace profiler {

// Singleton wrapper around Android executable "am" (Activity Manager).
class ActivityManager {
 public:
  enum ProfilingMode { SAMPLING, INSTRUMENTED };

  static ActivityManager *Instance();

  // Starts profiling using ART runtime profiler either by sampling or
  // code instrumentation.
  // Returns true is profiling started successfully. Otherwise false
  // and populate error_string.
  // |trace_path| is populated with absolute path where trace is being recorded.
  // Calling start twice in a row (without a stop) will result in an error.
  bool StartProfiling(const ProfilingMode profiling_mode,
                      const std::string &app_package_name,
                      int sampling_interval, std::string *trace_path,
                      std::string *error_string);

  // Stops ongoing profiling. If no profiling was ongoing, this function is a
  // no-op.
  // Returns true is profiling stopped successfully. Otherwise false
  // and populate error_string.
  bool StopProfiling(const std::string &app_package_name,
                     std::string *error_string);

  bool TriggerHeapDump(int pid, const std::string &file_path,
                       std::string *error_string) const;

 protected:
  // A protected constructor designed for testing.
  ActivityManager(std::unique_ptr<BashCommandRunner> bash)
      : bash_(std::move(bash)) {}

 private:
  ActivityManager();
  // Entry storing all data related to an ongoing profiling.
  class ArtOnGoingProfiling {
   public:
    std::string trace_path;  // File path where trace will be made available.
    std::string app_pkg_name;
  };

  // This mapping keeps track of ongoing profiling done via ART. It associates
  // an app package name with the trace file being produced.
  std::map<std::string, ArtOnGoingProfiling> profiled_;
  std::mutex profiled_lock_;  // Protects |profiled_|

  // Returns the absolute path_ where a profiling trace file should be
  // saved for a given |app_package_name|. App does not have to be up
  // and running for this method to succeed.
  // Generated path is unique (based on app package name and timestamp).
  std::string GenerateTracePath(const std::string &app_package_name) const;

  // Returns true if app is being profiled.
  bool IsAppProfiled(const std::string &app_package_name) const;

  void AddProfiledApp(const std::string &app_package_name,
                      const std::string &trace_path);

  void RemoveProfiledApp(const std::string &app_package_name);
  // Only call this method if you are certain the app is being profiled.
  // Otherwise result is undefined.
  std::string GetProfiledAppTracePath(
      const std::string &app_package_name) const;

  std::unique_ptr<BashCommandRunner> bash_;
};
}  // namespace profiler

#endif  // UTILS_PROFILER_ACTIVITYMANAGER_H
