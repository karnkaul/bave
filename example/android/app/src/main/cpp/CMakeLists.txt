cmake_minimum_required(VERSION 3.22)

project(bave-example)

if(NOT TARGET bave)
  set(bave_path ../../../../../../../bave)
  add_subdirectory(${bave_path} "${CMAKE_CURRENT_BINARY_DIR}/bave")
endif()

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::bave
  bave::bave-compile-options
  android-app-glue
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)
