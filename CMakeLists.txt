cmake_minimum_required(VERSION 3.1)
include(GNUInstallDirs)

project(nosync)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Werror")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("src")
file(GLOB_RECURSE nosync_SRC FOLLOW_SYMLINKS "src/*.cc")

add_library(nosync-static STATIC ${nosync_SRC})
target_compile_options(nosync-static PRIVATE -O2 -flto -ffat-lto-objects)

add_library(nosync SHARED ${nosync_SRC})
target_compile_options(nosync PRIVATE -O2 -flto)
set_target_properties(nosync PROPERTIES LINK_FLAGS "-flto")
set_target_properties(nosync PROPERTIES VERSION 0.0.1 SOVERSION 0)

install(TARGETS nosync-static ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS nosync DESTINATION ${CMAKE_INSTALL_LIBDIR}/)
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")

if(ENABLE_TESTS)
    add_library(nosync-static-for-test STATIC ${nosync_SRC})
    target_compile_options(nosync-static-for-test PRIVATE --coverage -O0)

    configure_file(gtest-CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
    execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

    file(GLOB_RECURSE nosync-test_SRC FOLLOW_SYMLINKS "test/*.cc")
    add_executable(nosync-test ${nosync-test_SRC})
    target_compile_options(nosync-test PRIVATE --coverage -O0)
    set_target_properties(nosync-test PROPERTIES LINK_FLAGS "--coverage")
    target_include_directories(nosync-test PUBLIC mock test)
    target_link_libraries(nosync-test nosync-static-for-test gtest gmock gtest_main)
endif(ENABLE_TESTS)
