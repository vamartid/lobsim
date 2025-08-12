cmake_minimum_required(VERSION 3.20)
project(LOBSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Optional: disable GNU extensions if you want strict standard

# ---- Fetch GTest ----
include(FetchContent)

# Allow offline build by not trying to update if already present
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

# Prevent re-downloading by checking if it was already populated
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_MakeAvailable(googletest)
endif()

# ---- Sources ----
# Collect all .cpp files recursively under src/
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(lobsim STATIC ${SOURCES})
target_include_directories(lobsim PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

# ---- Tests ----
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "test/*_test.cpp")
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} lobsim gtest_main)
    target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/test)
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endforeach()
# ---- Examples ----

file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
foreach(EXAMPLE_SRC ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
    target_link_libraries(${EXAMPLE_NAME} lobsim)
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
endforeach()
