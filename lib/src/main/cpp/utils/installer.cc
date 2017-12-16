#include "installer.h"

#include <unistd.h>
#include <memory>
#include <sstream>

#include "android_studio_version.h"
#include "bash_command.h"
#include "package_manager.h"
#include "utils/current_process.h"
#include "utils/fs/disk_file_system.h"
#include "utils/log.h"
#include "utils/trace.h"

using std::string;

namespace {
bool Exists(const string &who_runs_the_check, const string &path) {
  profiler::BashCommandRunner existsCmd("test");
  std::stringstream parameters;
  parameters << "-e ";
  parameters << path;
  string output;
  return existsCmd.RunAs(parameters.str(), who_runs_the_check, &output);
}
}
namespace profiler {
Installer::Installer(const char *app_package_name)
    : app_package_name_(app_package_name) {}

Installer::Installer(const std::string &app_package_name)
    : app_package_name_(app_package_name.c_str()) {}

bool Installer::Install(const string &binary_name, string *error_string) const {
  Trace trace("CPU:" + string("Install ") + binary_name.c_str());
  Log::I("Request to install sampler in app '%s'", app_package_name_.c_str());

  string src_path = CurrentProcess::dir() + binary_name;

  // Check if the sampler is already there.
  string dst_path;
  if (!GetInstallationPath(binary_name, &dst_path, error_string)) {
    error_string->append("\n");
    error_string->append("Unable to generate installation path");
    return false;
  }
  Log::I("Installing %s to %s", src_path.c_str(), dst_path.c_str());

  if (::Exists(app_package_name_, dst_path)) {
    Log::I("'%s' executable is already installed (found at '%s').\n",
           app_package_name_.c_str(), dst_path.c_str());
    return true;
  }

  Log::I("'%s' executable requires installation (missing from '%s').\n",
         app_package_name_.c_str(), dst_path.c_str());
  // We need to copy sampler to the app folder.

  DiskFileSystem fs;
  auto src = fs.GetFile(src_path);
  if (!src->Exists()) {
    error_string->append("\n");
    error_string->append("Source does not exists (" + src_path + ").");
    return false;
  }

  if (!BashCommandRunner::IsRunAsCapable()) {
    error_string->append("\n");
    error_string->append("System is not run-as capable");
    return false;
  }

  // This abomination spawns two process: A producer piping into a consumer.
  // sh -c 'cat SRC | run-as PKG_USER sh -c "cat > DST ; chmod 700 DST"'
  BashCommandRunner cmd("cat");

  std::stringstream copy_command;
  copy_command << src_path;
  copy_command << " | ";
  copy_command << "run-as ";
  copy_command << app_package_name_;
  copy_command << " sh -c \"cat > ";
  copy_command << dst_path;
  copy_command << "; chmod 700 ";
  copy_command << dst_path;
  copy_command << "\"";

  string out;
  bool success = cmd.Run(copy_command.str(), &out);
  if (!success || !Exists(app_package_name_, dst_path)) {
    error_string->append("\n");
    error_string->append("cmd:'" + copy_command.str() + "' failed:" + out);
    return false;
  }

  Log::I("%s %s %s", "Installation to'", dst_path.c_str(), "' succeeded.");
  return true;
}

bool Installer::Uninstall(const string &binary_path,
                          string *error_string) const {
  DiskFileSystem fs;
  auto target = fs.GetFile(binary_path);

  if (!target->Exists()) {
    error_string->append("\n");
    error_string->append("Cannot delete file '" + binary_path +
                         "': does not exist.");
    return false;
  }
  BashCommandRunner rm("rm");
  string parameters;
  parameters.append(target->path());
  bool success = rm.Run(parameters, error_string);
  if (!success || target->Exists()) {
    return false;
  }
  return true;
}

bool Installer::GetInstallationPath(const string &executable_path,
                                    string *install_path,
                                    string *error_string) const {
  string error_message;

  // Build the installation destination install_path:
  PackageManager pm;
  string app_base;
  bool ret = pm.GetAppDataPath(app_package_name_, &app_base, &error_message);
  if (!ret) {
    error_string->append(error_message);
    return false;
  }
  Log::I("App %s base is %s", app_package_name_.c_str(), app_base.c_str());

  DiskFileSystem fs;
  auto binary = fs.GetFile(executable_path);
  string binary_filename = binary->name();

  install_path->clear();
  install_path->append(app_base);
  install_path->append("/");
  install_path->append(GetBinaryNameForPackage(binary_filename));
  return true;
}

const string Installer::GetBinaryNameForPackage(
    const string &executable_filename) const {
  string binary_name;
  binary_name.append(executable_filename);
  binary_name.append("-v");
  binary_name.append(kAndroidStudioVersion);
  return binary_name;
}
}  // namespace profiler
