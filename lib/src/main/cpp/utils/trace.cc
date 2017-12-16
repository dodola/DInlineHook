#include "utils/trace.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace profiler {

int Trace::trace_marker_fd;

Trace::Trace(const char *name) { Begin(name); }

Trace::Trace(const std::string &name) { Begin(name.c_str()); }

Trace::~Trace() { End(); }

#if TRACE_OUTPUT == 1
void Trace::Init() {
  if (trace_marker_fd == 0) {
    trace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    // TODO: Error handling
    if (trace_marker_fd == -1) {
    }
  }
}

void Trace::Begin(const char *name) {
  char buf[kTraceMessageLen];
  int len = snprintf(buf, kTraceMessageLen, "B|%d|%s", getpid(), name);
  write(trace_marker_fd, buf, len);
}

void Trace::End() {
  char c = 'E';
  write(trace_marker_fd, &c, 1);
}
#else
void Trace::Init() {}
void Trace::Begin(const char *name) {}
void Trace::End() {}
#endif
}  // namespace profiler
