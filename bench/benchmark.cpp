#include <benchmark/benchmark.h>
#include <chrono>

namespace {
    
void BM_Bench(benchmark::State &state) {
  for ([[maybe_unused]] auto _ : state) {
  }
}

BENCHMARK(BM_Bench);

} // namespace
