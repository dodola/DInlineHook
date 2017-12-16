cmake_minimum_required(VERSION 3.6)

set(UTILS_SOURCE
     activity_manager.cc
     background_queue.cc
     bash_command.cc
     current_process.cc
     device_info.cc
     file_cache.cc
     file_reader.cc
     fs/dir.cc
     fs/disk_file_system.cc
     fs/file.cc
     fs/file_system.cc
     fs/memory_file_system.cc
     fs/path.cc
     installer.cc
     package_manager.cc
     process_manager.cc
     socket_utils.cc
     stopwatch.cc
     tokenizer.cc
     trace.cc
     clock.cc
     current_process.linux.cc
     filesystem_notifier.cc
     thread_name.cc
     log.android.cc
)
add_library(utils
           STATIC
            ${UTILS_SOURCE})

target_link_libraries(utils )