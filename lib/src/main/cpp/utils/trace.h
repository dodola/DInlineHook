#ifndef UTILS_TRACE_H
#define UTILS_TRACE_H

#include <string>

namespace profiler {
class Trace {
 public:
  explicit Trace(const char *name);
  explicit Trace(const std::string &name);
  Trace(const Trace&) = delete;
  Trace(Trace&&) = delete;
  ~Trace();
  static void Init();
  static void Begin(const char *name);
  static void End();
 private:
  static int trace_marker_fd;
  static const size_t kTraceMessageLen = 256;
};
}  // namespace profiler

#endif  // UTILS_TRACE_H
