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
#include "activity_manager.h"

#include <sys/stat.h>
#include <iostream>
#include <sstream>

#include "utils/clock.h"
#include "utils/current_process.h"
#include "utils/device_info.h"
#include "utils/filesystem_notifier.h"
#include "utils/trace.h"

using std::string;

namespace {
const char *const kAmExecutable = "/system/bin/am";
}

namespace profiler {

ActivityManager::ActivityManager()
    : bash_(new BashCommandRunner(kAmExecutable)) {}

bool ActivityManager::StartProfiling(const ProfilingMode profiling_mode,
                                     const string &app_package_name,
                                     int sampling_interval_us,
                                     string *trace_path, string *error_string) {
  Trace trace("CPU:StartProfiling ART");
  std::lock_guard<std::mutex> lock(profiled_lock_);

  if (IsAppProfiled(app_package_name)) {
    *error_string = "App is already being profiled with ART";
    return false;
  }
  *trace_path = this->GenerateTracePath(app_package_name);

  // Run command via actual am.
  std::ostringstream parameters;
  parameters << "profile start ";
  if (profiling_mode == ActivityManager::SAMPLING) {
    // A sample interval in microseconds is required after '--sampling'.
    // Note that '--sampling 0' would direct ART into instrumentation mode.
    // If there's no '--sampling X', instrumentation is used.
    parameters << "--sampling " << sampling_interval_us << " ";
  }
  if (DeviceInfo::feature_level() >= 26) {
    // Use streaming output mode on O or greater.
    parameters << "--streaming ";
  }
  parameters << app_package_name << " " << *trace_path;
  if (!bash_->Run(parameters.str(), error_string)) {
    *error_string = "Unable to run profile start command";
    return false;
  }
  AddProfiledApp(app_package_name, *trace_path);
  return true;
}

bool ActivityManager::StopProfiling(const string &app_package_name,
                                    string *error_string) {
  Trace trace("CPU:StopProfiling ART");
  std::lock_guard<std::mutex> lock(profiled_lock_);

  // Start monitoring trace events (to catch close) so this method only returns
  // when the generation of the trace file is finished.
  FileSystemNotifier notifier(GetProfiledAppTracePath(app_package_name),
                              FileSystemNotifier::CLOSE);

  RemoveProfiledApp(app_package_name);

  if (!notifier.IsReadyToNotify()) {
    *error_string = "Unable to monitor trace file for completion";
    return false;
  }

  // Run stop command via actual am.
  string parameters;
  parameters.append("profile stop ");
  parameters.append(app_package_name);
  if (!bash_->Run(parameters, error_string)) {
    *error_string = "Unable to run profile stop command";
    return false;
  }

  // Wait until ART has finished writing the trace to the file and closed the
  // file.
  if (!notifier.WaitUntilEventOccurs()) {
    *error_string = "Wait for ART trace file failed.";
    return false;
  }

  return true;
}

bool ActivityManager::TriggerHeapDump(int pid, const std::string &file_path,
                                      std::string *error_string) const {
  std::stringstream ss;
  ss << "dumpheap " << pid << " " << file_path;
  return bash_->Run(ss.str(), error_string);
}

std::string ActivityManager::GenerateTracePath(
    const std::string &app_package_name) const {
  // TODO: The activity manager should be a component of the daemon.
  // And it should use the daemon's steady clock.
  SteadyClock clock;
  std::stringstream path;
  path << CurrentProcess::dir();
  path << app_package_name;
  path << "-";
  path << clock.GetCurrentTime();
  path << ".art_trace";
  return path.str();
}

ActivityManager *ActivityManager::Instance() {
  static ActivityManager *instance = new ActivityManager();
  return instance;
}

bool ActivityManager::IsAppProfiled(const std::string &app_package_name) const {
  return profiled_.find(app_package_name) != profiled_.end();
}

void ActivityManager::AddProfiledApp(const std::string &app_package_name,
                                     const std::string &trace_path) {
  ArtOnGoingProfiling profilingEntry;
  profilingEntry.trace_path = trace_path;
  profilingEntry.app_pkg_name = app_package_name;
  profiled_[app_package_name] = profilingEntry;
}

void ActivityManager::RemoveProfiledApp(const std::string &app_package_name) {
  profiled_.erase(app_package_name);
}

string ActivityManager::GetProfiledAppTracePath(
    const std::string &app_package_name) const {
  auto it = profiled_.find(app_package_name);
  return it->second.trace_path;
}

}  // namespace profiler
