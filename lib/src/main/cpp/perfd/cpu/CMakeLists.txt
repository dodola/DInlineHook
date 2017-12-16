add_library(profiler_cpu
            cpu_cache.cc
            cpu_collector.cc
            cpu_service.cc
            cpu_usage_sampler.cc
            procfs_files.cc
            simpleperf_manager.cc
            thread_monitor.cc)

target_link_libraries(profiler_cpu)


target_link_libraries(
                      profiler_cpu
                      ${GTEST_LINK_LIBRARIES})
