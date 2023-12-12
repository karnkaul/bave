project(bave-flappy)

add_library(${PROJECT_NAME})
add_library(bave::flappy ALIAS bave-flappy)

target_link_libraries(${PROJECT_NAME} PUBLIC
  bave::bave
  bave::bave-compile-options
)

target_include_directories(${PROJECT_NAME} PUBLIC
  .
)

target_sources(${PROJECT_NAME} PRIVATE
  flappy.hpp
  flappy.cpp
)

file(MAKE_DIRECTORY ../assets/shaders)

find_program(glslc glslc)

add_custom_target(glsl2spirv ALL
  COMMENT "Building shaders"
  WORKING_DIRECTORY .

  COMMAND ${glslc} glsl/default.vert -o ../assets/shaders/default.vert.spv
  COMMAND ${glslc} glsl/default.frag -o ../assets/shaders/default.frag.spv

  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"

  VERBATIM USES_TERMINAL
)

add_dependencies(${PROJECT_NAME} glsl2spirv)
