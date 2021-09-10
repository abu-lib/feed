cmake_minimum_required(VERSION 3.16)
project(abu_feed VERSION 0.1.0)

include(.abu/build-utils/abu_lib.cmake)

add_library(abu_feed INTERFACE)
add_library(abu::feed ALIAS abu_feed)

target_include_directories(abu_feed INTERFACE include)
target_compile_features(abu_feed INTERFACE cxx_std_20)

abu_announce(feed)
abu_add_dependency(feed debug 0.2.0)

if(abu_feed_TEST)
  enable_testing()
  add_subdirectory(tests)
endif()