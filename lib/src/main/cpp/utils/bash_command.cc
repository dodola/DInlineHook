#include "bash_command.h"

#include "sys/wait.h"
#include "utils/fs/disk_file_system.h"
#include "utils/log.h"
#include "utils/trace.h"

using std::string;

namespace profiler {
BashCommandRunner::BashCommandRunner(const string &executable_path)
    : executable_path_(executable_path) {}

bool BashCommandRunner::RunAs(const string &parameters,
                              const string &package_name,
                              string *output) const {
  string cmd;
  cmd.append(kRunAsExecutable);
  cmd.append(" ");
  cmd.append(package_name);
  // TODO: The single quote can interfer with parameters. Disregarding
  // this potential issue for now.
  cmd.append(" sh -c '");
  cmd.append(executable_path_);
  cmd.append(" ");
  cmd.append(parameters);
  cmd.append("'");
  return RunAndReadOutput(cmd, output);
}

bool BashCommandRunner::Run(const string &parameters, string *output) const {
  string cmd;
  cmd.append(executable_path_);
  if (!parameters.empty()) {
    cmd.append(" ");
    cmd.append(parameters);
  }
  return RunAndReadOutput(cmd, output);
}

bool BashCommandRunner::RunAndReadOutput(const string &cmd,
                                         string *output) const {
  Trace trace(executable_path_);
  char buffer[1024];
  FILE *pipe = popen(cmd.c_str(), "r");
  if (pipe == nullptr) {
    return false;
  }

  while (!feof(pipe)) {
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      if (output != nullptr) {
        output->append(buffer);
      }
    }
  }
  int ret = pclose(pipe);
  return WEXITSTATUS(ret) == 0;
}

bool BashCommandRunner::IsRunAsCapable() {
  DiskFileSystem fs;
  auto run_as = fs.GetFile(kRunAsExecutable);
  // Checking for run-as existance is not enough: We also need to
  // check capabilities.
  // TODO: Use listxattr (as in
  // https://groups.google.com/forum/#!topic/android-kernel/iYakEvY24n4)
  // to makes sure run-as has CAP_SETUID and CAP_SETGID capability.
  // See bug report: https://code.google.com/p/android/issues/detail?id=187955
  return run_as->Exists();
}
}  // namespace profiler
