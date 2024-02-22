#
# .rst: FindClangTidy
# ---------------
# cmake-format: off
#
# The module defines the following variables
#
# ``CLANG_TIDY_EXECUTABLE``
# Path to clang-tidy executable
# ``CLANG_TIDY_FOUND``
# True if the clang-tidy executable was found.
# ``CLANG_TIDY_VERSION``
# The version of clang-tidy found
# ``CLANG_TIDY_VERSION_MAJOR``
# The clang-tidy major version if specified, 0 otherwise
# ``CLANG_TIDY_VERSION_MINOR``
# The clang-tidy minor version if specified, 0 otherwise
# ``CLANG_TIDY_VERSION_PATCH``
# The clang-tidy patch version if specified, 0 otherwise
# ``CLANG_TIDY_VERSION_COUNT``
# Number of version components reported by clang-tidy
#
# Example usage:
#
# .. code-block:: cmake
#
# find_package(ClangTidy)
# if(CLANG_TIDY_FOUND)
# message("clang-tidy executable found: ${CLANG_TIDY_EXECUTABLE}\n"
# "version: ${CLANG_TIDY_VERSION}")
# endif()
#
# This file is part of:
# https://github.com/nazavode/cmake-depot.git
# cmake-format: on

find_program(CLANG_TIDY_EXECUTABLE
    NAMES clang-tidy
    clang-tidy-17
    clang-tidy-16
    clang-tidy-15
    clang-tidy-14
    clang-tidy-13
    clang-tidy-12
    clang-tidy-11
    clang-tidy-10
    clang-tidy-9
    clang-tidy-8
    clang-tidy-7
    clang-tidy-6
    clang-tidy-5
    clang-tidy-4
    clang-tidy-3
    DOC "clang-tidy executable")
mark_as_advanced(CLANG_TIDY_EXECUTABLE)

# Extract version from command "clang-tidy -version"
if(CLANG_TIDY_EXECUTABLE)
    execute_process(COMMAND ${CLANG_TIDY_EXECUTABLE} -version
        OUTPUT_VARIABLE clang_tidy_version
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(clang_tidy_version MATCHES "^.* LLVM version .*")
        # clang_tidy_version sample:
        # Ubuntu LLVM version 14.0.0
        #
        # Optimized build.
        # Default target: x86_64-pc-linux-gnu
        # Host CPU: skylake
        #
        string(REGEX
            REPLACE ".* LLVM version ([.0-9]+).*"
            "\\1"
            CLANG_TIDY_VERSION
            "${clang_tidy_version}")

        # CLANG_TIDY_VERSION sample: "14.0.0"

        # Extract version components
        string(REPLACE "."
            ";"
            clang_tidy_version
            "${CLANG_TIDY_VERSION}")
        list(LENGTH clang_tidy_version CLANG_TIDY_VERSION_COUNT)

        if(CLANG_TIDY_VERSION_COUNT GREATER 0)
            list(GET clang_tidy_version 0 CLANG_TIDY_VERSION_MAJOR)
        else()
            set(CLANG_TIDY_VERSION_MAJOR 0)
        endif()

        if(CLANG_TIDY_VERSION_COUNT GREATER 1)
            list(GET clang_tidy_version 1 CLANG_TIDY_VERSION_MINOR)
        else()
            set(CLANG_TIDY_VERSION_MINOR 0)
        endif()

        if(CLANG_TIDY_VERSION_COUNT GREATER 2)
            list(GET clang_tidy_version 2 CLANG_TIDY_VERSION_PATCH)
        else()
            set(CLANG_TIDY_VERSION_PATCH 0)
        endif()
    endif()

    unset(clang_tidy_version)
endif()

if(CLANG_TIDY_EXECUTABLE)
    set(CLANG_TIDY_FOUND TRUE)
else()
    set(CLANG_TIDY_FOUND FALSE)
endif()
