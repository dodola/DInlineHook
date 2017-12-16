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
#ifndef UTILS_CLOCK_H_
#define UTILS_CLOCK_H_

#include <cstdint>

namespace profiler {

// A mockable clock class for getting the current time, in nanoseconds.
// Example:
//  SteadyClock clock;
//  Log(clock.GetCurrentTime());
//
// * The meaning of what "now" is relative to is left as an implementation
//   detail for subclasses to define and document.
// * If you are more interested in the amount of time an operation took,
//   rather than absolute time, use Stopwatch instead.
class Clock {
 public:
  static constexpr int64_t ns_to_us(int64_t ns) { return ns / 1000; }
  static constexpr int64_t ns_to_ms(int64_t ns) { return ns_to_us(ns) / 1000; }
  static constexpr int64_t ns_to_s(int64_t ns) { return ns_to_ms(ns) / 1000; }
  static constexpr int64_t ns_to_m(int64_t ns) { return ns_to_s(ns) / 60; }
  static constexpr int64_t ns_to_h(int64_t ns) { return ns_to_m(ns) / 60; }
  static constexpr int64_t us_to_ns(int64_t us) { return us * 1000; }
  static constexpr int64_t us_to_ms(int64_t us) { return us / 1000; }
  static constexpr int64_t us_to_s(int64_t us) { return us_to_ms(us) / 1000; }
  static constexpr int64_t us_to_m(int64_t us) { return us_to_s(us) / 60; }
  static constexpr int64_t us_to_h(int64_t us) { return us_to_m(us) / 60; }
  static constexpr int64_t ms_to_ns(int64_t ms) { return ms_to_us(ms) * 1000; }
  static constexpr int64_t ms_to_us(int64_t ms) { return ms * 1000; }
  static constexpr int64_t ms_to_s(int64_t ms) { return ms / 1000; }
  static constexpr int64_t ms_to_m(int64_t ms) { return ms_to_s(ms) / 60; }
  static constexpr int64_t ms_to_h(int64_t ms) { return ms_to_m(ms) / 60; }
  static constexpr int64_t s_to_ns(int64_t s) { return s_to_us(s) * 1000; }
  static constexpr int64_t s_to_us(int64_t s) { return s_to_ms(s) * 1000; }
  static constexpr int64_t s_to_ms(int64_t s) { return s * 1000; }
  static constexpr int64_t s_to_m(int64_t s) { return s / 60; }
  static constexpr int64_t s_to_h(int64_t s) { return s_to_m(s) / 60; }
  static constexpr int64_t m_to_ns(int64_t m) { return m_to_us(m) * 1000; }
  static constexpr int64_t m_to_us(int64_t m) { return m_to_ms(m) * 1000; }
  static constexpr int64_t m_to_ms(int64_t m) { return m_to_s(m) * 1000; }
  static constexpr int64_t m_to_s(int64_t m) { return m * 60; }
  static constexpr int64_t m_to_h(int64_t m) { return m / 60; }
  static constexpr int64_t h_to_ns(int64_t h) { return h_to_us(h) * 1000; }
  static constexpr int64_t h_to_us(int64_t h) { return h_to_ms(h) * 1000; }
  static constexpr int64_t h_to_ms(int64_t h) { return h_to_s(h) * 1000; }
  static constexpr int64_t h_to_s(int64_t h) { return h_to_m(h) * 60; }
  static constexpr int64_t h_to_m(int64_t h) { return h * 60; }

  // Returns a monotonically increasing value. This value is meant for
  // comparing two relative times, as the time represented by time=0 is not
  // explicitly defined.
  virtual int64_t GetCurrentTime() const = 0;
  virtual ~Clock() = default;
};

// A Clock implementation based on clock_gettime(CLOCK_MONOTONIC).
//
// Note: we choose to rely on our own class instead of <chrono> because our most
// important use-case is profiling on Android, and this approach lets us use an
// API which:
// - has satisfactory precision, granularity, and reliability
// - is also accessible from Java via System.nanoTime.
// - is used by the Linux kernel to timestamp events (like in perfs)
class SteadyClock final : public Clock {
 public:
  // It is safe to be called from multiple threads simultaneously.
  virtual int64_t GetCurrentTime() const override;
};

}  // namespace profiler

#endif  // UTILS_CLOCK_H_
