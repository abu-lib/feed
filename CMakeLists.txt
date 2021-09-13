cmake_minimum_required(VERSION 3.16)
project(abu_feed VERSION 0.1.0)

include(.abu/build-utils/abu_lib.cmake)

# abu::feed
add_library(abu_feed INTERFACE)
target_include_directories(abu_feed INTERFACE include)
target_link_libraries(abu_feed INTERFACE abu::mem)
add_library(abu::feed ALIAS abu_feed)
abu_announce(feed
  DEPENDENCIES
    debug@0.2.4
    mem@0.1.1
)

# abu::checked::feed
add_library(abu_checked_feed INTERFACE)
target_compile_definitions(abu_checked_feed INTERFACE ABU_FEED_CHECK_ASSUMPTIONS)
target_include_directories(abu_checked_feed INTERFACE include)
target_link_libraries(abu_checked_feed INTERFACE abu::mem)
add_library(abu::checked::feed ALIAS abu_checked_feed)


if(ABU_FEED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(ABU_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
