##Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.##

# Comments:

find_program(LCOV NAMES lcov HINTS "/usr" PATH_SUFFIXES "bin" DOC "lcov - a graphical GCOV front-end" REQUIRED)
find_program(GCOV NAMES $ENV{GCOV_NAME} gcov HINTS "/usr" PATH_SUFFIXES "bin" DOC "GNU gcov binary" REQUIRED)
find_program(GENHTML NAMES genhtml HINTS "/usr" PATH_SUFFIXES "bin" DOC "genhtml - Generate HTML view from LCOV coverage data files" REQUIRED)

if(NOT (LCOV AND GCOV) )
    message(FATAL_ERROR "locv or gcov not found! Aborting...")
endif()

set(LCOV_FILTERS "'/usr/*';'/*/_deps/*';'/*/boost/*'")
set(LCOV_FLAGS "--rc;lcov_branch_coverage=1")
set(GENHTML_FLAGS "--branch-coverage;--rc;genhtml_med_limit=80;--rc;genhtml_hi_limit=95;--legend")

message( STATUS "Code Coverage Module (LCOV)" )

add_custom_target( coverage-clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf coverage/
    COMMAND find . -name *.gcda -exec rm -v {} \;
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT "Cleaning coverage related files"
    VERBATIM
)

add_custom_target( coverage-run
    COMMAND ${CMAKE_MAKE_PROGRAM} coverage-clean
    DEPENDS test_libblis.x
    COMMAND test_libblis.x -g ${CMAKE_CURRENT_SOURCE_DIR}/input.general -o ${CMAKE_CURRENT_SOURCE_DIR}/input.operations > ${CMAKE_CURRENT_BINARY_DIR}/output.testsuite
    COMMENT "Code Coverage takes some time : Running test_libblis.x with output redirected to ${CMAKE_CURRENT_BINARY_DIR}/output.testsuite"
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    VERBATIM
)

add_custom_target( coverage-report
    COMMAND ${CMAKE_MAKE_PROGRAM} coverage-run
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage/
    COMMAND ${LCOV} ${LCOV_FLAGS} -d .. -c -o coverage/coverage.info --gcov-tool ${GCOV}
    COMMAND ${LCOV} ${LCOV_FLAGS} --remove   coverage/coverage.info --gcov-tool ${GCOV} -o coverage/coverage_filtered.info ${LCOV_FILTERS}
    COMMAND ${GENHTML} ${GENHTML_FLAGS} coverage/coverage_filtered.info --output coverage/html --title "AOCL-BLAS Code Coverage Report"
    COMMENT "Building Code Coverage Report (LCOV)"
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    VERBATIM
)

# Alias (only Makefile/Linux)
add_custom_target( coverage
    DEPENDS coverage-report
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    VERBATIM
)