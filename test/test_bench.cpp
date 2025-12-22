#include "counters/bench.h"
#include <cstdio>

volatile int sink = 0;

int fib(int n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

int main() {
  if (!counters::event_collector().has_events()) {
    printf("Warning: Performance events are not available on this platform. Maybe use sudo?\n");
  }
  // Default measurement for a very simple function
  auto agg_simple = counters::bench([] { sink++; });
  printf("simple: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f\n",
         agg_simple.elapsed_ns(), agg_simple.total_elapsed_ns(), agg_simple.iteration_count(), agg_simple.instructions());

  // A slightly heavier micro-workload tuned with parameters
  counters::bench_parameter p;

  auto agg_fancy = counters::bench([] {
    volatile int s = 0;
    for (int i = 0; i < 100; ++i) s += i;
    sink += s;
  }, p);
  printf("fancy: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f\n",
         agg_fancy.elapsed_ns(), agg_fancy.total_elapsed_ns(), agg_fancy.iteration_count(), agg_fancy.instructions());

  // A more expensive (CPU-bound) function
  auto agg_fib = bench([] { volatile int x = fib(20); (void)x; }, p);
  printf("fib20: elapsed_ns=%f total_ns=%f iterations=%d instructions=%f\n",
         agg_fib.elapsed_ns(), agg_fib.total_elapsed_ns(), agg_fib.iteration_count(), agg_fib.instructions());

  return 0;
}
