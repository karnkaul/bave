cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(project_prefix bave)
project(${project_prefix} VERSION 0.5.1)

set(is_root_project FALSE)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(is_root_project TRUE)
endif()

set(build_tests ${is_root_project})

option(BAVE_USE_PCH "Use PCH for bave builds" ON)
option(BAVE_BUILD_EXAMPLE "Build bave example" ${is_root_project})
option(BAVE_BUILD_TESTS "Build bave tests" ${build_tests})
option(BAVE_USE_FREETYPE "Use freetype for text rendering in bave" ON)
option(BAVE_BUILD_TOOLS "Build bave-tools" ${is_root_project})

add_library(${project_prefix}-compile-options INTERFACE)
add_library(${project_prefix}::${project_prefix}-compile-options ALIAS ${project_prefix}-compile-options)

target_compile_definitions(${project_prefix}-compile-options INTERFACE
  $<$<CONFIG:Debug>:BAVE_DEBUG>
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(${project_prefix}-compile-options INTERFACE
    -Wall -Wextra -Wpedantic -Wconversion -Wunused -Werror=return-type
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_options(${project_prefix}-compile-options INTERFACE
    /MP
  )
  target_compile_definitions(${project_prefix}-compile-options INTERFACE
    _AMD64_ NOMINMAX
  )
endif()

add_subdirectory(ext)
add_subdirectory(lib)

if(BAVE_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

if(BAVE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BAVE_BUILD_TOOLS)
  if(ANDROID)
    message(WARNING "bave-tools can only built for desktop")
  else()
    add_subdirectory(tools)
  endif()
endif()
