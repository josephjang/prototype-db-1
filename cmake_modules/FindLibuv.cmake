# libuv

set(LIBUV_ROOT CACHE PATH "Root directory of libuv installation")
set(LIBUV_EXTRA_PREFIXES /usr/local ${LIBUV_ROOT})

foreach(prefix ${LIBUV_EXTRA_PREFIXES})
  list(APPEND LIBUV_INCLUDE_PATHS "${prefix}/include")
  list(APPEND LIBUV_LIBRARIES_PATHS "${prefix}/lib")
endforeach()

message(STATUS "Paths: ${LIBUV_INCLUDE_PATHS}")
find_path(LIBUV_INCLUDE_DIRS uv.h PATHS ${LIBUV_INCLUDE_PATHS})
message(STATUS "Found include: ${LIBUV_INCLUDE_DIRS}")
find_library(
  LIBUV_LIBRARIES
  NAMES uv
  PATHS ${LIBUV_LIBRARIES_PATHS})
message(STATUS "Found lib: ${LIBUV_LIBRARIES}")

if(LIBUV_LIBRARIES AND LIBUV_INCLUDE_DIRS)
  set(LIBUV_FOUND 1)
  set(LIBUV_LIBRARIES ${LIBUV_LIBRARIES})
  message(
    STATUS
      "Found libuv (include: ${LIBUV_INCLUDE_DIRS}, library: ${LIBUV_LIBRARIES})"
  )
else()
  set(LIBUV_FOUND 0)
  message(FATAL_ERROR "Could not find libuv.")
endif()
