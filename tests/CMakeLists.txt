# Helper so we don't have to repeat ourselves so much
# Usage: add_cpp_test(testname [COMPILE_ONLY] [SOURCES a.cpp b.cpp ...] [LABELS acceptance per_implementation ...])
# SOURCES defaults to testname.cpp if not specified.
function(add_cpp_test TEST_NAME)
  # Parse arguments
  cmake_parse_arguments(PARSE_ARGV 1 ARGS "COMPILE_ONLY;WILL_FAIL" "" "SOURCES;LABELS")
  if (NOT ARGS_SOURCES)
    list(APPEND ARGS_SOURCES ${TEST_NAME}.cpp)
  endif()
  if (COMPILE_ONLY)
    list(APPEND ${ARGS_LABELS} compile)
  endif()

  # Add executable
  add_executable(${TEST_NAME} ${ARGS_SOURCES})

  # Add test
  if (ARGS_COMPILE_ONLY)
    add_test(
      NAME ${TEST_NAME}
      COMMAND ${CMAKE_COMMAND} --build . --target ${TEST_NAME} --config $<CONFIGURATION>
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
    set_target_properties(${TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  else()
    add_test(${TEST_NAME} ${TEST_NAME})
  endif()

  if (ARGS_LABELS)
    set_property(TEST ${TEST_NAME} APPEND PROPERTY LABELS ${ARGS_LABELS})
  endif()

  if (ARGS_WILL_FAIL)
    set_property(TEST ${TEST_NAME} PROPERTY WILL_FAIL TRUE)
  endif()
endfunction()
# Most tests need test data, and many need windows headers.
link_libraries(simdjson-flags test-data simdjson-windows-headers)

include(${PROJECT_SOURCE_DIR}/tests/add_cpp_test.cmake)

#
# These tests explicitly do #include "simdjson.cpp" so they can override stuff
#
if (NOT MSVC) # Can't get simdjson-source to compile on Windows for some reason.
  add_cpp_test(numberparsingcheck LABELS acceptance per_implementation)
  target_link_libraries(numberparsingcheck simdjson-include-source)
  add_cpp_test(stringparsingcheck LABELS acceptance per_implementation)
  target_link_libraries(stringparsingcheck simdjson-include-source)
endif()

# All remaining tests link with simdjson proper
link_libraries(simdjson)
add_cpp_test(basictests LABELS acceptance per_implementation)
add_cpp_test(errortests LABELS acceptance per_implementation)
add_cpp_test(integer_tests LABELS acceptance per_implementation)
add_cpp_test(jsoncheck LABELS acceptance per_implementation)
add_cpp_test(parse_many_test LABELS acceptance per_implementation)
add_cpp_test(pointercheck LABELS acceptance per_implementation)
add_cpp_test(extracting_values_example LABELS acceptance per_implementation)

# Script tests
if (NOT MSVC) # Can't run .sh on windows
  #
  # json2json test
  #
  add_test(
    NAME testjson2json
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/testjson2json.sh
    WORKING_DIRECTORY $<TARGET_FILE_DIR:minify>
  )
  set_property(TEST testjson2json APPEND PROPERTY DEPENDS minify json2json)
  set_property(TEST testjson2json APPEND PROPERTY LABELS per_implementation)

  #
  # Competition parse test
  #
  if (SIMDJSON_COMPETITION)
    add_executable(allparserscheckfile allparserscheckfile.cpp)
    target_link_libraries(allparserscheckfile competition-all)

    add_test(issue150 bash ${CMAKE_CURRENT_SOURCE_DIR}/issue150.sh)
    set_property(TEST issue150 APPEND PROPERTY DEPENDS allparserscheckfile)
    set_property(TEST issue150 APPEND PROPERTY LABELS per_implementation)
  endif()
endif()

# This should really test whether bash is available.
if (NOT MSVC)
  #
  # json2json tool test: check that json2json can parse twitter.json
  #

  # This tests validates that the implementation is what we think it is if we get passed
  # SIMDJSON_FORCE_IMPLEMENTATION, so we know we're testing what we think we're testing
  add_cpp_test(checkimplementation LABELS per_implementation)

  add_test(NAME json2json COMMAND $<TARGET_FILE:json2json> ${EXAMPLE_JSON})
  set_property(TEST json2json APPEND PROPERTY LABELS acceptance per_implementation)

  #
  # SIMDJSON_FORCE_IMPLEMENTATION tests: run json2json with SIMDJSON
  #
  if (SIMDJSON_IMPLEMENTATION_FALLBACK)
    add_test(
      NAME simdjson_force_implementation
      COMMAND
        ${CMAKE_COMMAND} -E env
        SIMDJSON_FORCE_IMPLEMENTATION=fallback
        $<TARGET_FILE:checkimplementation>
    )
  endif()
  add_test(
    NAME simdjson_force_implementation_error
    COMMAND
      ${CMAKE_COMMAND} -E env
      SIMDJSON_FORCE_IMPLEMENTATION=doesnotexist
      $<TARGET_FILE:json2json> ${EXAMPLE_JSON}
  )
  set_tests_properties(simdjson_force_implementation_error PROPERTIES WILL_FAIL TRUE)
endif()

#
# Compile-only tests with simdjson flags on
#

# Don't add the tests if we're on VS2017 or older; they don't succeed.
if(NOT (MSVC AND MSVC_VERSION LESS 1920))
  if(SIMDJSON_EXCEPTIONS)
    add_cpp_test(readme_examples COMPILE_ONLY LABELS acceptance)
    add_cpp_test(readme_examples11 COMPILE_ONLY LABELS acceptance SOURCES readme_examples.cpp)
    set_target_properties(readme_examples11 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
  endif()

  add_cpp_test(readme_examples_noexceptions COMPILE_ONLY LABELS acceptance)

  add_cpp_test(readme_examples_noexceptions11 COMPILE_ONLY LABELS acceptance SOURCES readme_examples_noexceptions.cpp)
  set_target_properties(readme_examples_noexceptions11 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

  # Compile tests that *should fail*
  add_cpp_test(readme_examples_will_fail_with_exceptions_off WILL_FAIL COMPILE_ONLY LABELS acceptance SOURCES readme_examples.cpp)
  target_compile_definitions(readme_examples_will_fail_with_exceptions_off PRIVATE SIMDJSON_EXCEPTIONS=0)
endif()


if(MSVC)
  add_custom_command(TARGET basictests POST_BUILD        # Adds a post-build event
    COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:simdjson>"
    COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE_DIR:basictests>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake -E copy_if_different..."
        "$<TARGET_FILE:simdjson>"      # <--this is in-file
        "$<TARGET_FILE_DIR:basictests>")                 # <--this is out-file path
endif()

# Copy the simdjson dll into the tests directory
if(MSVC)
  add_custom_command(TARGET basictests POST_BUILD        # Adds a post-build event
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake -E copy_if_different..."
        "$<TARGET_FILE:simdjson>"      # <--this is in-file
        "$<TARGET_FILE_DIR:basictests>")                 # <--this is out-file path
endif()

## Next bit should not be needed!
#if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
# next line is a workaround for an odr-violation in basictests regarding the globals 0x432a40 and 0x52045c under clang
#set_tests_properties(basictests PROPERTIES
#    ENVIRONMENT ASAN_OPTIONS="detect_odr_violation=0")
#endif()

## This causes problems
# add_executable(singleheader ./singleheadertest.cpp ${PROJECT_SOURCE_DIR}/singleheader/simdjson.cpp)
# target_link_libraries(singleheader simdjson simdjson-flags)
# add_test(singleheader singleheader)

add_subdirectory(compilation_failure_tests)
