cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(NelderMead_DNN VERSION 0.1)

set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The type of build" FORCE)
endif()

option(DNN_TEST "Enable building unit tests" ON)
option(DNN_BENCH "Enable building benchmarks" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

include(cmake/modules/CPM.cmake)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BLA_SIZEOF_INTEGER 4)
find_package(BLAS REQUIRED)

if(BLAS_FOUND)
    # Intel MKL -> DNN_BLAS_IS_MKL
    if((BLA_VENDOR STREQUAL "All" OR BLA_VENDOR MATCHES "^Intel*") AND
        (BLAS_mkl_LIBRARY
        OR BLAS_mkl_em64t_LIBRARY
        OR BLAS_mkl_ia32_LIBRARY
        OR BLAS_mkl_intel_LIBRARY
        OR BLAS_mkl_intel_lp64_LIBRARY
        OR BLAS_mkl_rt_LIBRARY))
        set(DNN_BLAS_IS_MKL TRUE CACHE BOOL INTERNAL "Selected BLAS for DNN is Intel MKL")
    endif()

    if(NOT DEFINED BLAS_INCLUDE_DIRS)
        set(cblas_header "cblas.h")

        if(DNN_BLAS_IS_MKL)
            set(cblas_header "mkl_cblas.h")
        endif()

        # Note: the builtin BLAS finder doesn't look for headers,
        # thus *no* BLAS_INCLUDE_DIRS is set alongside BLAS_LIBRARIES.
        # Let's try to find the CBLAS header on our own hoping for
        # the best...
        list(LENGTH BLAS_LIBRARIES blas_num_lib)

        if(blas_num_lib GREATER 1)
            # if we have a list of objects, take the first one
            list(GET BLAS_LIBRARIES 0 blas_lib)
        else()
            set(blas_lib ${BLAS_LIBRARIES})
        endif()

        get_filename_component(blas_libdir ${blas_lib} DIRECTORY)
        cmake_path(GET blas_libdir PARENT_PATH blas_home)
        find_path(BLAS_INCLUDE_DIRS
            NAMES ${cblas_header}
            HINTS ${BLAS_ROOT}
            ${blas_libdir}
            ${blas_home}
            PATH_SUFFIXES include
            REQUIRED
            DOC "BLAS include directories containing the CBLAS header")
        message(STATUS "Found CBLAS include dir: ${BLAS_INCLUDE_DIRS}")
    endif()

    # Patch BLAS::BLAS target:
    # 1. Include dirs
    target_include_directories(BLAS::BLAS INTERFACE ${BLAS_INCLUDE_DIRS})
else()
    set(OPENBLAS_VERSION "0.3.26" CACHE STRING "OpenBLAS version")
    set(OPENBLAS_URL "https://github.com/OpenMathLib/OpenBLAS" CACHE STRING "OpenBLAS repository")
    message(STATUS "No BLAS library found: downloading OpenBLAS v${OPENBLAS_VERSION} from ${OPENBLAS_URL}")
    CPMAddPackage(
        NAME openblas
        GITHUB_REPOSITORY OpenMathLib/OpenBLAS
        VERSION ${OPENBLAS_VERSION}
        OPTIONS "BUILD_WITHOUT_LAPACK ON" "BUILD_TESTING OFF" "C_LAPACK ON"
    )
endif()

# Default C++ flags
if(NOT CMAKE_CXX_FLAGS)
    # set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wshadow -Wunknown-pragmas -Wsign-conversion -Wfloat-conversion")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wunknown-pragmas -Wsign-conversion -Wfloat-conversion")
endif()

# Default Release flags
if(NOT CMAKE_CXX_FLAGS_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

# Default ReleaseWithDebugInfo flags
if(NOT CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer -march=native -mtune=native")
endif()

# Default Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer") # -fsanitize=\"address,undefined,float-divide-by-zero\"")
endif()

set(DNN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(DNN_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(DNN_EXEC_DIR "${CMAKE_SOURCE_DIR}/exec")
set(DNN_TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(DNN_BENCH_DIR "${CMAKE_SOURCE_DIR}/bench")

add_library(dnn STATIC
    "${DNN_SOURCE_DIR}/model.cpp"
    "${DNN_SOURCE_DIR}/train.cpp"
    "${DNN_SOURCE_DIR}/utils.cpp")
target_include_directories(dnn PUBLIC "${DNN_INCLUDE_DIR}")

add_executable(main "${DNN_EXEC_DIR}/main.cpp")
target_link_libraries(main PUBLIC dnn BLAS::BLAS)
target_include_directories(main PUBLIC ${DNN_INCLUDE_DIR})

if(DNN_TEST)
    set(GTEST_VERSION "1.14.0" CACHE STRING "Google Test version")
    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        VERSION ${GTEST_VERSION}
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
    )
    enable_testing()

    add_executable(dnn-test
        "${DNN_TEST_DIR}/test.cpp")
    target_include_directories(dnn-test PUBLIC ${DNN_INCLUDE_DIR})
    target_link_libraries(dnn-test PUBLIC dnn PRIVATE GTest::gtest_main BLAS::BLAS)

    include(GoogleTest)
    gtest_discover_tests(dnn-test)
endif()

if(DNN_BENCH)
    set(GBENCHMARK_VERSION "1.8.3" CACHE STRING "Google Benchmark version")
    CPMAddPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        VERSION ${GBENCHMARK_VERSION}
        OPTIONS "BENCHMARK_ENABLE_TESTING OFF" "BENCHMARK_ENABLE_WERROR OFF"
    )

    add_executable(dnn-bench "${DNN_BENCH_DIR}/benchmark.cpp")
    target_include_directories(dnn-bench PUBLIC ${DNN_INCLUDE_DIR})
    target_link_libraries(dnn-bench PUBLIC dnn PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        BLAS::BLAS)
endif()

find_package(ClangFormat QUIET)

if(CLANG_FORMAT_FOUND)
    message(STATUS "clang-format: found ${CLANG_FORMAT_EXECUTABLE} v${CLANG_FORMAT_VERSION}")
    message(STATUS "clang-format: adding custom target 'format'")
    message(STATUS "clang-format: adding custom target 'format-fix'")
    add_custom_target(format
        COMMAND
        ${CLANG_FORMAT_EXECUTABLE} -style=file --dry-run --Werror
        "${DNN_INCLUDE_DIR}/*.hpp"
        "${DNN_SOURCE_DIR}/*.cpp"
        "${DNN_EXEC_DIR}/*.cpp"
        "${DNN_TEST_DIR}/*.cpp"
        "${DNN_BENCH_DIR}/*.cpp"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_target(format-fix
        COMMAND
        ${CLANG_FORMAT_EXECUTABLE} -style=file -i
        "${DNN_INCLUDE_DIR}/*.hpp"
        "${DNN_SOURCE_DIR}/*.cpp"
        "${DNN_EXEC_DIR}/*.cpp"
        "${DNN_TEST_DIR}/*.cpp"
        "${DNN_BENCH_DIR}/*.cpp"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(CLANG_FORMAT_FOUND)

find_package(ClangTidy QUIET)

if(CLANG_TIDY_FOUND)
    message(STATUS "clang-tidy: found ${CLANG_TIDY_EXECUTABLE} v${CLANG_TIDY_VERSION}")
    message(STATUS "clang-tidy: adding custom target 'tidy'")
    message(STATUS "clang-tidy: adding custom target 'tidy-fix'")
    set(tidy_cmd
        ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_CURRENT_BINARY_DIR} -warnings-as-errors=*)
    add_custom_target(tidy
        COMMAND
        ${tidy_cmd}
        "${DNN_INCLUDE_DIR}/*.hpp"
        "${DNN_SOURCE_DIR}/*.cpp"
        "${DNN_EXEC_DIR}/*.cpp"
        "${DNN_TEST_DIR}/*.cpp"
        "${DNN_BENCH_DIR}/*.cpp"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_target(tidy-fix
        COMMAND
        ${tidy_cmd} --fix
        "${DNN_INCLUDE_DIR}/*.hpp"
        "${DNN_SOURCE_DIR}/*.cpp"
        "${DNN_EXEC_DIR}/*.cpp"
        "${DNN_TEST_DIR}/*.cpp"
        "${DNN_BENCH_DIR}/*.cpp"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(CLANG_TIDY_FOUND)

message(STATUS "")
message(STATUS "Successfully configured ${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}")
message(STATUS "COMPILER         : ${CMAKE_CXX_COMPILER} v${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "DNN_TEST         : ${DNN_TEST}")
message(STATUS "DNN_BENCH        : ${DNN_BENCH}")
message(STATUS "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message(STATUS "")
