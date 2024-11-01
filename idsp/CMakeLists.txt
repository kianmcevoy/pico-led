cmake_minimum_required(VERSION 3.10)

# Project name
project(idsp)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make sure the compiler looks here on MacOS
if(APPLE)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
endif()

# Include the include/ directory for downstream projects
include_directories(${PROJECT_SOURCE_DIR}/include)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

### TESTING ###

if(NOT DEFINED ENV{IDSP_ENABLE_TESTING})
    set(ENV{IDSP_ENABLE_TESTING} ON)
endif()

if ($ENV{IDSP_ENABLE_TESTING})
    message("IDSP: tests enabled")

    # Enable `make test`
    enable_testing()

    # Get test source files
    file(GLOB TEST_SRCS ${PROJECT_SOURCE_DIR}/test/*.cpp)

    # Default DSP parameters
    if(NOT DEFINED ENV{SAMPLE_TYPE})
        set(ENV{SAMPLE_TYPE} float)
    endif()

    # Run through each test file
    foreach(testSrc ${TEST_SRCS})
        # Get extension-less file name
        get_filename_component(testFileName ${testSrc} NAME_WE)
        # Make test name
        set(testName ${PROJECT_NAME}_test_${testFileName})
        # Add target
        add_executable(${testName} ${testSrc})
        # Definitions
        target_compile_definitions(${testName} PUBLIC
            Sample=$ENV{SAMPLE_TYPE}
        )
        # Put test executables in their own directory
        set_target_properties(${testName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test/bin)

        # Add test to execution
        add_test(
            NAME ${testName}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test/bin
            COMMAND ${testName}
        )
    endforeach(testSrc)
else()
    message("IDSP: tests disabled")
endif()
