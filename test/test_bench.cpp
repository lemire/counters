#include "counters/bench.h"
#include <cstdio>

volatile int sink = 0;

int fib(int n) {
  if (n < 2) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

int main() {
  if (!counters::has_performance_counters()) {
    printf("Warning: Performance events are not available on this platform. Maybe use sudo?\n");
  }
  // Empty function benchmark
  auto trivial_simple = counters::bench([] {  });
  printf("trivial: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f branches=%f branch_misses=%f cache_misses=%f\n",
         trivial_simple.elapsed_ns(), trivial_simple.total_elapsed_ns(), trivial_simple.iteration_count(), trivial_simple.instructions(), trivial_simple.branches(), trivial_simple.branch_misses(), trivial_simple.cache_misses());

  // Default measurement for a very simple function
  auto agg_simple = counters::bench([] { sink++; });
  printf("simple: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f branches=%f branch_misses=%f cache_misses=%f\n",
         agg_simple.elapsed_ns(), agg_simple.total_elapsed_ns(), agg_simple.iteration_count(), agg_simple.instructions(), agg_simple.branches(), agg_simple.branch_misses(), agg_simple.cache_misses());

  // A slightly heavier micro-workload tuned with parameters
  counters::bench_parameter p;

  auto agg_fancy = counters::bench([] {
    volatile int s = 0;
    for (int i = 0; i < 100; ++i) s += i;
    sink += s;
  }, p);
  printf("fancy: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f branches=%f branch_misses=%f cache_misses=%f\n",
         agg_fancy.elapsed_ns(), agg_fancy.total_elapsed_ns(), agg_fancy.iteration_count(), agg_fancy.instructions(), agg_fancy.branches(), agg_fancy.branch_misses(), agg_fancy.cache_misses());

  // A more expensive (CPU-bound) function
  auto agg_fib = bench([] { volatile int x = fib(20); (void)x; }, p);
  printf("fib20: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f branches=%f branch_misses=%f cache_misses=%f\n",
         agg_fib.elapsed_ns(), agg_fib.total_elapsed_ns(), agg_fib.iteration_count(), agg_fib.instructions(), agg_fib.branches(), agg_fib.branch_misses(), agg_fib.cache_misses());
  // A memcpy benchmark
  std::vector<char> src(1024 * 1024);
  std::vector<char> dst(1024 * 1024);
  auto agg_memcpy = bench([&] { std::memcpy(dst.data(), src.data(), src.size()); }, p);
  printf("memcpy 1MB: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f branches=%f branch_misses=%f cache_misses=%f speed=%f GB/s\n",
         agg_memcpy.elapsed_ns(), agg_memcpy.total_elapsed_ns(), agg_memcpy.iteration_count(), agg_memcpy.instructions(), agg_memcpy.branches(), agg_memcpy.branch_misses(), agg_memcpy.cache_misses(), (double(src.size()) * agg_memcpy.iteration_count()) / agg_memcpy.total_elapsed_ns());
  return EXIT_SUCCESS;
}
