# Counters

This project provides a C++ library to access hardware performance counters (CPU cycles, instructions, etc.) on different platforms (Linux, macOS/Apple Silicon).

## Main Features
- Access to hardware counters via native interfaces (perf events on Linux, kpc on macOS/Apple Silicon)
- Measurement of elapsed time and hardware events for code sections
- Simple and portable interface

## Usage

The library is low-level and does not actually provide the benchmarking code itself.
I encourage you to build up your own benchmarking code.

As an example, the repository provides a small helper `bench()` to measure a
callable. The helper accepts callables as forwarding references and provides a
second form that accepts a `bench_parameter` struct to tune behaviour.

```cpp
#include "counters/bench.h"

// simple usage: forwards the callable
auto agg = counters::bench([] {
  // code to benchmark
});

// or use parameters to tune the measurement
bench_parameter params;
params.min_repeat = 20;
params.min_time_ns = 200'000'000; // 0.2 s
auto agg2 = counters::bench([] {
  // code to benchmark
}, params);
```

- **Tailoring `bench`**

You can tune the measurement behaviour via the `bench_parameter` struct.

- `min_repeat`: minimum number of outer iterations (warm-up + measurement).
  Increase when you need more samples or when per-iteration variance is high.
- `min_time_ns`: target minimum warm-up time (nanoseconds). If the warm-up
  time after `min_repeat` is shorter, the outer loop will grow up to
  `max_repeat`. Increase for longer stabilization on complex workloads.
- `max_repeat`: safety cap on the outer loop. Raised if you expect long runs
  or want more samples; keep reasonable to avoid runaway loops.

Notes:
- `bench` accepts the callable as a forwarding reference and uses
  `std::forward` internally.
- For very short functions `bench` runs an inner loop that repeats the
  callable `M` times (up to `inner_max_repeat`) so the measured block is
  stable; all returned counters are divided by `M` to produce per-call
  metrics (the caller observes results "as if" the callable ran once). Timings are then divided by the number of repetitions. This might be problematic in some cases, so use some care in interpreting the results from short functions.


*WARNINGS*:
- It might matter a great deal whether function is inlineable. Inlining can drastically change the working being benchmarked. 
- Care should be taken that the call to `function()` is not optimized away. You can avoid such problems by saving results to a volatile variable (although not too often in a tight loop). You may also want to add synchronization and other features.

The `event_aggregate` struct provides aggregate statistics over multiple `event_count` measurements. Its main methods are


- `double elapsed_sec() const`: mean elapsed time in seconds
- `double elapsed_ns() const`: mean elapsed time in nanoseconds
- `double total_elapsed_ns() const`: total elapsed time in nanoseconds
- `double cycles() const`: mean CPU cycles
- `double instructions() const`: mean instructions
- `double branch_misses() const`: mean branch misses
- `double branches() const`: mean branches
- `double fastest_elapsed_ns() const`: best (minimum) elapsed time in nanoseconds
- `double fastest_cycles() const`: best (minimum) cycles
- `double fastest_instructions() const`: best (minimum) instructions
- `int iteration_count() const`: the number of iterations

You can use these methods to analyze the performance of your function, for example:

```cpp
printf("Fastest time (ns): %f\n", agg.fastest_elapsed_ns());
printf("Iterations: %d\n", agg.iterations);
if(counters::has_performance_counters()) {
  printf("Mean cycles: %f\n", agg.cycles());
  printf("Mean instructions: %f\n", agg.instructions());
  printf(" %f GHz\n", agg.cycles() /agg.elapsed_ns())
  printf(" %f instructions/cycle\n", agg.instructions()/agg.cycles())
}
```

The performance counters are only available when `counters::has_performance_counters()` returns true.
You may need to run your software with privileged access (sudo) to get the performance
counters.


## CMake


You can add the library as a dependency as follows. Replace `x.y.z` by 
the version you want to use.

```cmake
FetchContent_Declare(
  counters
  GIT_REPOSITORY https://github.com/lemire/counters.git
  GIT_TAG vx.y.z
)

FetchContent_MakeAvailable(counters)

target_link_libraries(yourtarget PRIVATE counters::counters)
```

If you use CPM, it is somewhat simplier:

```CMake
include(cmake/CPM.cmake)

CPMAddPackage("gh:lemire/counters#vx.y.z")
target_link_libraries(yourtarget PRIVATE counters::counters)
```


## Citing This Work

If you use this project in a publication or report, please consider citing it. Replace fields (year, author, url, commit) as appropriate.

```bibtex
@misc{counters2025,
  author = {Daniel Lemire},
  title = {{The counters library: Lightweight performance counters for Linux and macOS (Apple Silicon)}},
  year = {2025},
  howpublished = {GitHub repository},
  note = {https://github.com/lemire/counters}
}
```


## Project Structure
## Project Structure
- `include/counters/event_counter.h`: Main interface for event measurement
- `include/counters/linux-perf-events.h`: Linux implementation (perf events)
- `include/counters/apple_arm_events.h`: Apple Silicon/macOS implementation
- `include/counters/bench.h`: `bench()` helper and `bench_parameter` tuning API
- `include/counters/*`: public headers used by consumers
- `CMakeLists.txt`: CMake configuration file
- `README.md`: this documentation and usage examples

Feel free to open an issue or pull request for any improvement or correction.

