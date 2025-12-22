#include "counters/event_counter.h"
#include <utility>

/// Parameters to control benchmarking behavior.
///
/// The benchmark performs two levels of repetition:
/// - Outer iterations (``min_repeat`` / ``max_repeat``): the main warm-up and
///   measurement loop. We run the outer loop `N` times to collect multiple
///   samples and compute averages / best / worst results.
/// - Inner iterations (abstracted by ``inner_max_repeat`` /
///   ``min_time_per_inner_ns``): when a single call to the tested function is
///   too fast to measure reliably, the implementation runs the callable `M`
///   times in a tight inner loop and measures that block. All recorded counters
///   are then divided by `M` to produce per-call results. This keeps the API
///   semantics identical: users receive metrics "as if" the callable ran once,
///   while benefiting from more stable timing for very short functions.
///
/// All fields have sensible defaults; override only those you need.
struct bench_parameter {
  /// Minimum number of outer iterations (warmup + measurement).
  ///
  /// The outer loop count `N` controls how many independent samples are
  /// collected. If the warm-up period (after `min_repeat` iterations) is too
  /// short the outer loop may be increased (up to `max_repeat`).
  size_t min_repeat = 10;

  /// Minimum total warm-up time in nanoseconds.
  /// If the total warm-up time (after `min_repeat` iterations) is less than
  /// this value the number of iterations is increased (up to `max_repeat`).
  size_t min_time_ns = 400000000; // 0.4 s

  /// Maximum number of outer iterations.
  /// Guards against runaway increases when trying to reach `min_time_ns`.
  size_t max_repeat = 1000000;

  /// Maximum multiplier for inner repetition when measuring very fast callables.
  ///
  /// The inner repetition factor `M` is an implementation detail: the bench
  /// will increase `M` (by powers of ten) until the inner block takes at least
  /// `min_time_per_inner_ns` nanoseconds, or until `inner_max_repeat` is
  /// reached. All counters are divided by `M` before returning, so the caller
  /// observes per-call metrics.
  size_t inner_max_repeat = 1000;

  /// Minimum time in nanoseconds for the inner repeated block.
  /// The benchmark increases the inner repetition factor until the elapsed
  /// time for the inner loop is at least this many nanoseconds (or until
  /// `inner_max_repeat` is reached).
  size_t min_time_per_inner_ns = 1000;
};

template <class Function>
event_aggregate bench(Function &&function, const bench_parameter &params) {
  return bench(std::forward<Function>(function), params.min_repeat,
               params.min_time_ns, params.max_repeat,
               params.inner_max_repeat, params.min_time_per_inner_ns);
}

template <class Function>
event_aggregate bench(Function &&function, size_t min_repeat = 10,
                      size_t min_time_ns = 400'000'000,
                      size_t max_repeat = 1000000,
                      size_t inner_max_repeat = 1000,
                      size_t min_time_per_inner_ns = 1000) {
  static thread_local event_collector collector;
  auto fn = std::forward<Function>(function);
  size_t N = min_repeat;
  // if function() is too fast, repeat it M times to get a measurable time.
  size_t M = 1;
  while (M < inner_max_repeat) {
    collector.start();
    for (size_t i = 0; i < M; i++) {
      fn();
    }
    event_count allocate_count = collector.end();
    if (allocate_count.elapsed_ns() >= min_time_per_inner_ns) {
      break;
    }
    M *= 10;
    if (M >= inner_max_repeat) {
      M = inner_max_repeat;
      break;
    }
  }
  if (N == 0) {
    N = 1;
  }
  // We warm up first. We warmup for at least 0.4s (by default). This makes
  // sure that the processor is in a consistent state.
  event_aggregate warm_aggregate{};
  for (size_t i = 0; i < N; i++) {
    collector.start();
    for (size_t j = 0; j < M; j++) {
      fn();
    }
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
    for (size_t j = 0; j < M; j++) {
      fn();
    }
    event_count allocate_count = collector.end();
    aggregate << allocate_count;
  }
  aggregate /= M; // average per single function() call
  return aggregate;
}