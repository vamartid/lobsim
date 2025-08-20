cmake_minimum_required(VERSION 3.20)
project(LOBSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -------------------------------
# 1️⃣ Detect all modules (directories in src/)
# -------------------------------
file(GLOB MODULE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

set(MODULES "")
foreach(dir ${MODULE_DIRS})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/${dir}")
        list(APPEND MODULES ${dir})
    endif()
endforeach()

message(STATUS "Detected modules: ${MODULES}")

# -------------------------------
# 2️⃣ Create static libraries for each module
# -------------------------------
set(MODULE_LIBS "")
foreach(mod ${MODULES})
    file(GLOB_RECURSE ${mod}_SRCS CONFIGURE_DEPENDS "src/${mod}/*.cpp" "src/${mod}/*.h")
    add_library(${mod} STATIC ${${mod}_SRCS})
    target_include_directories(${mod} PUBLIC include)
    target_include_directories(${mod} PRIVATE src/${mod}) # Add private include for internal headers
    list(APPEND MODULE_LIBS ${mod})
endforeach()

# -------------------------------
# 3️⃣ Detect and link module dependencies recursively
# -------------------------------
# -------------------------------
# 3️⃣ Detect and link module dependencies recursively
# -------------------------------
foreach(mod ${MODULES})
    set(PRIVATE_DEPENDENCIES "")
    set(PUBLIC_DEPENDENCIES "")
    
    # We will check both source files and public headers for dependencies
    file(GLOB_RECURSE MOD_SRCS "src/${mod}/*.cpp")
    file(GLOB_RECURSE MOD_HEADERS "include/${mod}/*.h")

    # Check for private dependencies (dependencies only in the module's source files)
    foreach(src_file ${MOD_SRCS})
        file(READ ${src_file} FILE_CONTENT)
        foreach(dep_mod ${MODULES})
            if(NOT ${mod} STREQUAL ${dep_mod})
                string(FIND "${FILE_CONTENT}" "#include \"${dep_mod}/" FOUND_DEP)
                if(FOUND_DEP GREATER -1)
                    # Found a dependency in a source file, but we don't know if it's private or public yet
                    list(APPEND PRIVATE_DEPENDENCIES ${dep_mod})
                endif()
            endif()
        endforeach()
    endforeach()

    # Now, check for public dependencies (dependencies in the module's public headers)
    foreach(header_file ${MOD_HEADERS})
        file(READ ${header_file} FILE_CONTENT)
        foreach(dep_mod ${MODULES})
            if(NOT ${mod} STREQUAL ${dep_mod})
                string(FIND "${FILE_CONTENT}" "#include \"${dep_mod}/" FOUND_DEP)
                if(FOUND_DEP GREATER -1)
                    list(APPEND PUBLIC_DEPENDENCIES ${dep_mod})
                    # A dependency found in a public header is a public dependency
                endif()
            endif()
        endforeach()
    endforeach()
    
    # Remove duplicates
    list(REMOVE_DUPLICATES PRIVATE_DEPENDENCIES)
    list(REMOVE_DUPLICATES PUBLIC_DEPENDENCIES)
    
    # A public dependency is also a private one, so remove it from the private list
    foreach(pub_dep ${PUBLIC_DEPENDENCIES})
        list(REMOVE_ITEM PRIVATE_DEPENDENCIES ${pub_dep})
    endforeach()
    
    # Link the dependencies
    if(PUBLIC_DEPENDENCIES)
        message(STATUS "Module ${mod} has public dependencies: ${PUBLIC_DEPENDENCIES}")
        target_link_libraries(${mod} PUBLIC ${PUBLIC_DEPENDENCIES})
    endif()

    if(PRIVATE_DEPENDENCIES)
        message(STATUS "Module ${mod} has private dependencies: ${PRIVATE_DEPENDENCIES}")
        target_link_libraries(${mod} PRIVATE ${PRIVATE_DEPENDENCIES})
    endif()
    
    if(NOT PRIVATE_DEPENDENCIES AND NOT PUBLIC_DEPENDENCIES)
        message(STATUS "Module ${mod} has no dependencies")
    endif()
endforeach()

# Section 5 and 6 remain the same, as they now automatically inherit the public dependencies
# when linking to a module.

# -------------------------------
# 4️⃣ Setup GoogleTest
# -------------------------------
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED_GOOGLETEST ON)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
set(BUILD_SHARED_LIBS OFF)
set(gtest_force_shared_crt OFF)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.x
)
FetchContent_MakeAvailable(googletest)
enable_testing()

# -------------------------------
# 5️⃣ Add tests dynamically and detect module dependencies
# -------------------------------
# Assume MODULES is a list of your static libraries, including "PriceTimePriorityStrategy"

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "test/*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${test_src} NAME_WE)
    add_executable(${TEST_NAME} ${test_src})
    target_include_directories(${TEST_NAME} PRIVATE include test)
    
    # Detect dependencies for the test executable
    set(TEST_DEPS "")
    foreach(mod ${MODULES})
        file(READ ${test_src} FILE_CONTENT)
        string(FIND "${FILE_CONTENT}" "#include \"${mod}/" FOUND_DEP)
        if(FOUND_DEP GREATER -1)
            list(APPEND TEST_DEPS ${mod})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES TEST_DEPS)

    message(STATUS "Test ${TEST_NAME} depends on modules: ${TEST_DEPS}")
    
    # Special handling for the PriceTimePriorityStrategy test
    if(TEST_NAME STREQUAL "PriceTimePriorityStrategy_test")
        # Define the macro for the static library target *before* it's linked
        # This ensures the library is recompiled with the macro
        target_compile_definitions(engine PRIVATE PRICE_TIME_PRIORITY_DEBUG_REDUCE_TAKER)
    endif()

    target_link_libraries(${TEST_NAME} PRIVATE ${TEST_DEPS} gtest gtest_main)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

endforeach()

# -------------------------------
# 6️⃣ Build example files
# -------------------------------
file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS "examples/*.cpp")
foreach(example_src ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${example_src} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${example_src})
    target_include_directories(${EXAMPLE_NAME} PRIVATE include example)

    # Detect dependencies for the example executable
    set(EXAMPLE_DEPS "")
    foreach(mod ${MODULES})
        file(READ ${example_src} FILE_CONTENT)
        string(FIND "${FILE_CONTENT}" "#include \"${mod}/" FOUND_DEP)
        if(FOUND_DEP GREATER -1)
            list(APPEND EXAMPLE_DEPS ${mod})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES EXAMPLE_DEPS)

    message(STATUS "Example ${EXAMPLE_NAME} depends on modules: ${EXAMPLE_DEPS}")

    # Link the example to its detected module dependencies
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${EXAMPLE_DEPS})
endforeach()