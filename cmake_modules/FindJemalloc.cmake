# jemalloc

find_library(
  JEMALLOC_LIBRARIES
  NAMES jemalloc
)
message(STATUS "Found lib: ${JEMALLOC_LIBRARIES}")

if(JEMALLOC_LIBRARIES)
  set(JEMALLOC_FOUND 1)
  set(JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARIES})
  message(
    STATUS
      "Found jemalloc (library: ${JEMALLOC_LIBRARIES})"
  )
else()
  set(JEMALLOC_FOUND 0)
  message(FATAL_ERROR "Could not find jemalloc.")
endif()
