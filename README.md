# Counters

This project provides a C++ library to access hardware performance counters (CPU cycles, instructions, etc.) on different platforms (Linux, macOS/Apple Silicon).

## Main Features
- Access to hardware counters via native interfaces (perf events on Linux, kpc on macOS/Apple Silicon)
- Measurement of elapsed time and hardware events for code sections
- Simple and portable interface

## Usage

Include the `include/` folder in your project and link the generated library if needed.

Example usage:

```cpp
#include "counters/event_counter.h"

// ...
event_count c = measure_events([](){
    // code to measure
});
printf("Cycles: %llu\n", c.event_counts[event_count::CPU_CYCLES]);
```

## Project Structure
- `include/counters/event_counter.h`: Main interface for event measurement
- `include/counters/linux-perf-events.h`: Linux implementation (perf events)
- `include/counters/apple_arm_events.h`: Apple Silicon/macOS implementation
- `CMakeLists.txt`: CMake configuration file

Feel free to open an issue or pull request for any improvement or correction.
