#include "package_manager.h"

#include "utils/log.h"
#include "utils/trace.h"

#include <cstring>
#include <sstream>

using std::string;

namespace {
const char *kPackagePrefix = "package:";
const char *kPM_EXEC = "/system/bin/pm";
}

namespace profiler {
PackageManager::PackageManager() : BashCommandRunner(kPM_EXEC) {}

bool PackageManager::GetAppBaseFolder(const string &package_name, string *path,
                                      string *error_string) const {
  Trace trace("PackageManager::GetAppBaseFolder");
  string parameters;
  parameters.append("path ");
  parameters.append(package_name);
  bool success = Run(parameters, error_string);
  if (!success) {
    return false;
  }

  // pm returns the path to the apk. We need to parse the response:
  // package:/data/app/net.fabiensanglard.shmup-1/base.apk
  // into
  // /data/app/net.fabiensanglard.shmup-1

  //  Make sure input is well-formed.
  if (path->find(kPackagePrefix) == string::npos) {
    *path = "";
    *error_string =
        "Unable to retrieve app base folder for '" + package_name + ";";
    return false;
  }

  // Remove prefix and prefix.
  *path = path->substr(strlen(kPackagePrefix), path->find_last_of("/"));
  return true;
}

bool PackageManager::GetAppDataPath(const string &package_name, string *path,
                                    string *error_string) const {
  Trace trace("PackageManager::GetAppDataPath");

  BashCommandRunner pwd("pwd");
  string parameters;
  std::string output;
  bool success = pwd.RunAs(parameters, package_name, &output);
  if (!success) {
    string msg = "Unable to retrieve App Data Path";
    *error_string = msg;
    Log::E("%s", msg.c_str());
    return false;
  }

  // Remove the CR at the end of the line.
  output.pop_back();

  *path = output;
  Log::D("GetAddDataPath %s", output.c_str());
  return true;
}
}  // namespace profiler
