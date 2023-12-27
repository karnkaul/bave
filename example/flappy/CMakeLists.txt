project(bave-flappy)

option(BAVE_BUILD_SHADERS "Build shaders using glslc (if available)" ON)

add_library(${PROJECT_NAME})
add_library(bave::flappy ALIAS bave-flappy)

target_link_libraries(${PROJECT_NAME} PUBLIC
  bave::bave
  bave::bave-compile-options
)

target_include_directories(${PROJECT_NAME} PUBLIC
  .
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "src/*.[hc]pp")

target_sources(${PROJECT_NAME} PRIVATE ${sources})

file(MAKE_DIRECTORY ../assets/shaders)

if(BAVE_BUILD_SHADERS)
  find_program(glslc glslc)

  if("${glslc}" STREQUAL "glslc-NOTFOUND")
    message(WARNING "Cannot build shaders: glslc not found")
  else()
    add_custom_target(glsl2spirv ALL
      COMMENT "Building shaders"
      WORKING_DIRECTORY .

      COMMAND ${glslc} glsl/default.vert -o ../assets/shaders/default.vert.spv
      COMMAND ${glslc} glsl/default.frag -o ../assets/shaders/default.frag.spv

      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"

      VERBATIM USES_TERMINAL
    )

    add_dependencies(${PROJECT_NAME} glsl2spirv)
  endif()
endif()
