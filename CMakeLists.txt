# libcur — the Code of Universe Regulations as a C++17 state machine.
#
# Standalone build, for CI and for consumers that are not SCons-based:
#
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build --output-on-failure
#
# SCons consumers (the Aevoria Simulator) use SConscript instead; both compile
# the same source list, so neither can drift without the other noticing.

cmake_minimum_required(VERSION 3.16)

project(cur
    VERSION 0.1.0
    DESCRIPTION "Code of Universe Regulations — deterministic governance FSM"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CUR_BUILD_TESTS "Build the libcur test suite" ON)

add_library(cur STATIC
    src/cur_state.cpp
    src/cur_event.cpp
    src/cur_entity.cpp
    src/cur_regulation.cpp
    src/cur_violation.cpp
    src/cur_event_log.cpp
    src/cur_capture_index.cpp
    src/cur_advocate.cpp
    src/cur_state_machine.cpp
)

add_library(cur::cur ALIAS cur)

target_include_directories(cur PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    # /utf-8 is required: the transition tables carry section signs in their
    # citations, and MSVC otherwise reads the source as the local codepage.
    target_compile_options(cur PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(cur PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CUR_BUILD_TESTS)
    enable_testing()

    # No external test framework — the assertions are plain and the suite has
    # to run anywhere the library does.
    add_executable(cur_tests tests/cur_tests.cpp)
    target_link_libraries(cur_tests PRIVATE cur)
    if(MSVC)
        target_compile_options(cur_tests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(cur_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME cur_tests COMMAND cur_tests)
endif()

install(TARGETS cur EXPORT curTargets ARCHIVE DESTINATION lib)
install(DIRECTORY include/cur DESTINATION include)
