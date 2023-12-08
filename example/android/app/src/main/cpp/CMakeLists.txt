cmake_minimum_required(VERSION 3.22)

project(bave-example)

if(NOT TARGET bave)
  set(bave_path ../../../../../../../bave)
  add_subdirectory(${bave_path} "${CMAKE_CURRENT_BINARY_DIR}/bave")

  set(flappy_path ${bave_path}/example/flappy)
  add_subdirectory(${flappy_path} "${CMAKE_CURRENT_BINARY_DIR}/flappy")
endif()

set(validation_layers_zip "${CMAKE_CURRENT_SOURCE_DIR}/validation_layers.zip")

if(NOT EXISTS "${validation_layers_zip}")
  set(validation_layers_url "https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases/download/sdk-1.3.250.1/android-binaries-1.3.250.1.zip")
  file(DOWNLOAD ${validation_layers_url} "${validation_layers_zip}" SHOW_PROGRESS)
endif()

if(EXISTS "${validation_layers_zip}")
  file(ARCHIVE_EXTRACT INPUT "${validation_layers_zip}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs")
endif()

add_library(${PROJECT_NAME} SHARED)

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::flappy
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)
