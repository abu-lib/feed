add_executable(abu_feed_benchmarks
    benchmark_range_adaptor.cpp
)

target_link_libraries(abu_feed_benchmarks PRIVATE abu::feed)
abu_configure_benchmark_target(abu_feed_benchmarks)
