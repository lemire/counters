# Counters

This project provides a C++ library to access hardware performance counters (CPU cycles, instructions, etc.) on different platforms (Linux, macOS/Apple Silicon).

## Main Features
- Access to hardware counters via native interfaces (perf events on Linux, kpc on macOS/Apple Silicon)
- Measurement of elapsed time and hardware events for code sections
- Simple and portable interface

## Usage

The library is low-level and does not actually provide the benchmarking code itself.
I encourage you to build up your own benchmarking code.

As an example, the following code example will benchmark a function. 
The C++ templated function bench measures the performance of a provided callable function by executing it multiple times, using a collector to track metrics like execution time, and returns an event_aggregate object with the results; it accepts parameters for minimum repetitions (min_repeat, default 10), minimum warm-up time in nanoseconds (min_time_ns, default 400ms), and maximum repetitions (max_repeat, default 1,000,000), ensuring at least one iteration; the function first runs a warm-up phase, executing function at least min_repeat times and increasing iterations (up to max_repeat) by a factor of 10 if the total warm-up time is less than min_time_ns, to stabilize the processor state, then performs the actual measurement phase with the same number of iterations, collecting metrics into a new event_aggregate for consistent and reliable benchmarking.


```cpp
 #include "counters/event_counter.h"

 event_collector collector;

 template <class function_type>
 event_aggregate bench(const function_type &&function, size_t min_repeat = 10,
                       size_t min_time_ns = 400'000'000,
                       size_t max_repeat = 1000000) {
   size_t N = min_repeat;
   if (N == 0) { N = 1; }
   // We warm up first. We warmup for at least 0.4s (by default). This makes
   // sure that the processor is in a consistent state.
   event_aggregate warm_aggregate{};
   for (size_t i = 0; i < N; i++) {
     collector.start();
     function();
     event_count allocate_count = collector.end();
     warm_aggregate << allocate_count;
     if ((i + 1 == N) && (warm_aggregate.total_elapsed_ns() < min_time_ns) &&
         (N < max_repeat)) {
       N *= 10;
     }
   }
   // Actual measure, another 0.4s (by default), this time with a processor
   // warmed up.
   event_aggregate aggregate{};
   for (size_t i = 0; i < N; i++) {
     collector.start();
     function();
     event_count allocate_count = collector.end();
     aggregate << allocate_count;
   }
   return aggregate;
 }
```

Care should be taken that the call to `function()` is not optimized away. You can avoid
such problems by saving results to a volatile variable. You may also want to add synchronization and other features. For very inexpensive functions, this routine will fail: so you should benchmark sensible functions (at least tens of nanoseconds). 

You call it as...

```cpp
auto agg = bench(myfunction);
```

where `myfunction` could be a lambda.


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

You can use these methods to analyze the performance of your function, for example:

```cpp
printf("Mean cycles: %f\n", agg.cycles());
printf("Mean instructions: %f\n", agg.instructions());
printf("Fastest time (ns): %f\n", agg.fastest_elapsed_ns());
printf("Iterations: %d\n", agg.iterations);
```

The performance counters are only available when `collector.has_events()` returns true.
You may need to run your software with privileged access (sudo) to get the performance
counters.


## CMake


You can add the library as a dependency as follows.

```cmake
FetchContent_Declare(
  counters
  GIT_REPOSITORY https://github.com/lemire/counters.git
  GIT_TAG v1.0.4
)

FetchContent_MakeAvailable(counters)

target_link_libraries(yourtarget PRIVATE counters::counters)
```

If you use CPM, it is somewhat simplier:

```CMake
include(cmake/CPM.cmake)

CPMAddPackage("gh:lemire/counters#v1.0.4")
target_link_libraries(yourtarget PRIVATE counters::counters)
```


## Citing This Work

If you use this project in a publication or report, please consider citing it. Replace fields (year, author, url, commit) as appropriate.

```bibtex
@misc{counters2025,
  author = {Daniel Lemire},
  title = {counters: Lightweight performance counters for Linux and macOS (Apple Silicon)},
  year = {2025},
  howpublished = {GitHub repository},
  note = {https://github.com/lemire/counters}
}
```


## Project Structure
- `include/counters/event_counter.h`: Main interface for event measurement
- `include/counters/linux-perf-events.h`: Linux implementation (perf events)
- `include/counters/apple_arm_events.h`: Apple Silicon/macOS implementation
- `CMakeLists.txt`: CMake configuration file

Feel free to open an issue or pull request for any improvement or correction.

