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

#ifndef UTILS_PROCESS_MANAGER_H
#define UTILS_PROCESS_MANAGER_H

#include <sys/types.h>
#include <string>
#include <vector>

namespace profiler {

// Record storing all information retrieved from /proc/pids folders
struct Process {
 public:
  Process(pid_t pid, const std::string &cmdline,
          const std::string &binary_name);
  pid_t pid;
  std::string cmdline;
  std::string binary_name;
};

class ProcessManager {
 public:
  // Search running process started with arg[0] == app_pkg_name and returns its
  // pid
  // This method purpose is to match an app with a process id and the
  // expectation is that only one app with this package name will be running.
  // Therefore, it returns the first match.
  int GetPidForBinary(const std::string &binary_name) const;

  // Return true is process |pid| is currently running (present in /proc).
  bool IsPidAlive(int pid) const;

  static std::string GetCmdlineForPid(int pid);

  // Get the package name associate with the application name. If the
  // application of interest is a service running as its own process, its'
  // app_name can be of the format PACKAGE_NAME:SERVICE_NAME. We need
  // to extract the package name for operations like run-as and data folder path
  // retrieval, which works on the package instead of the app.
  static std::string GetPackageNameFromAppName(const std::string &app_name);

 private:
  std::vector<Process> GetAllProcesses() const;
};
}
#endif  // UTILS_PROCESS_MANAGER_H
