#include <benchmark/benchmark.h>

#include <chrono>
#include <span>

#include "abu/feed.h"

std::vector<int> get_int_data(std::size_t n) {
  return std::vector<int>(n, 12);
}

static void BM_accum_reference(benchmark::State& state) {
  auto data = get_int_data(state.range(0));

  for (auto _ : state) {
    int accum = 0;
    for (const auto& v : data) {
      accum += v;
    }
    benchmark::DoNotOptimize(accum);
  }
}
BENCHMARK(BM_accum_reference)->Range(1024, 10000000);

static void BM_accum_adapted(benchmark::State& state) {
  auto data = get_int_data(state.range(0));

  for (auto _ : state) {
    auto adapted = abu::feed::adapt_range(data);
    int accum = 0;
    while (adapted != abu::feed::end_of_feed) {
      accum += *adapted;
      ++adapted;
    }

    benchmark::DoNotOptimize(accum);
  }
}
BENCHMARK(BM_accum_adapted)->Range(1024, 10000000);

static void BM_stream_single_chunk_shared_ptr(benchmark::State& state) {
  auto data = get_int_data(state.range(0));

  for (auto _ : state) {
    abu::feed::stream<std::span<int>> adapted;
    adapted.append(data);
    adapted.finish();

    int accum = 0;
    while (adapted != abu::feed::end_of_feed) {
      accum += *adapted;
      ++adapted;
    }

    benchmark::DoNotOptimize(accum);
  }
}
BENCHMARK(BM_stream_single_chunk_shared_ptr)->Range(1024, 10000000);

static void BM_stream_multi_chunks_shared_ptr(benchmark::State& state) {
  auto data = get_int_data(state.range(0));

  abu::feed::stream<std::span<int>> adapted;
  auto from = data.begin();
  auto segments = state.range(1);
  auto seg_len = data.size() / segments;
  for (int i = 0; i < segments; ++i) {
    auto next = std::next(from, seg_len);
    adapted.append(std::span<int>{from, next});
    from = next;
  }
  adapted.append(std::span<int>{from, data.end()});
  adapted.finish();

  auto cp = adapted.checkpoint();
  for (auto _ : state) {
    adapted.rollback(cp);

    int accum = 0;
    while (adapted != abu::feed::end_of_feed) {
      accum += *adapted;
      ++adapted;
    }

    benchmark::DoNotOptimize(accum);
  }
}

BENCHMARK(BM_stream_multi_chunks_shared_ptr)
    ->ArgsProduct({benchmark::CreateRange(1024, 10000000, 4),
                   benchmark::CreateDenseRange(2, 10, 1)});
BENCHMARK_MAIN();