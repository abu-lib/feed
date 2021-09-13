add_executable(abu_feed_tests
    test_stream.cpp
    test_adapted_range.cpp
)
target_link_libraries(abu_feed_tests PRIVATE abu::checked::feed)
abu_configure_test_target(abu_feed_tests)
add_test(abu_feed_tests abu_feed_tests)