cmake_minimum_required(VERSION 3.0.0)

project(prototype-db-1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

set(BUILD_SUPPORT_DIR ${CMAKE_SOURCE_DIR}/build-support)

#
# Target: protodb1
#

add_executable(protodb1 src/main.cc)
target_include_directories(protodb1 PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_sources(protodb1 PRIVATE src/echo_server.cc src/redis_server.cc src/storage_engine.cc)

#
# Libraries: Libuv
#

find_package(Libuv REQUIRED)
target_include_directories(protodb1 PRIVATE "${LIBUV_INCLUDE_DIRS}")
target_link_libraries(protodb1 PRIVATE "${LIBUV_LIBRARIES}")

#
# Libraries: jemalloc
#
find_package(Jemalloc REQUIRED)
target_link_libraries(protodb1 PRIVATE "${JEMALLOC_LIBRARIES}")

#
# Compiler Tools
#

set(CLANG_FORMAT_VERSION_PATTERN "9")
find_package(ClangTools)

#
# make format
#

if(${CLANG_FORMAT_FOUND})
  # runs clang format and updates files in place.
  add_custom_target(
    format
    ${BUILD_SUPPORT_DIR}/run_clang_format.py
    ${CLANG_FORMAT_BIN}
    ${BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
    --source_dir
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    --fix)

  # runs clang format and exits with a non-zero exit code if any files need to
  # be reformatted
  add_custom_target(
    check-format ${BUILD_SUPPORT_DIR}/run_clang_format.py ${CLANG_FORMAT_BIN}
    ${BUILD_SUPPORT_DIR}/clang_format_exclusions.txt --source_dir
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
