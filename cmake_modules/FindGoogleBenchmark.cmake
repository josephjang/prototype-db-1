# jemalloc

find_library(
  GOOGLE_BENCHMARK_LIBRARIES
  NAMES benchmark
)
message(STATUS "Found lib: ${GOOGLE_BENCHMARK_LIBRARIES}")

if(GOOGLE_BENCHMARK_LIBRARIES)
  set(GOOGLE_BENCHMARK_FOUND 1)
  set(GOOGLE_BENCHMARK_LIBRARIES ${GOOGLE_BENCHMARK_LIBRARIES})
  message(
    STATUS
      "Found Google Benchmark (library: ${GOOGLE_BENCHMARK_LIBRARIES})"
  )
else()
  set(GOOGLE_BENCHMARK_FOUND 0)
  message(FATAL_ERROR "Could not find Google Benchmark.")
endif()
