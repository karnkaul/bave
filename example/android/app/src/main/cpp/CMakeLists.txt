cmake_minimum_required(VERSION 3.22)

project(bave-example)

if(NOT TARGET bave)
  set(bave_path ../../../../../../../bave)
  add_subdirectory(${bave_path} "${CMAKE_CURRENT_BINARY_DIR}/bave")

  set(flappy_path ${bave_path}/example/flappy)
  add_subdirectory(${flappy_path} "${CMAKE_CURRENT_BINARY_DIR}/flappy")
endif()

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::flappy
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)
