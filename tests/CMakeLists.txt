add_executable(abu_feed_tests
    test_stream.cpp
    test_adapted_range.cpp
)

target_link_libraries(abu_feed_tests PRIVATE GTest::gtest_main abu::feed)
add_test(abu_feed_tests abu_feed_tests)