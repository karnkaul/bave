project(bave-ext)

add_library(${PROJECT_NAME} INTERFACE)
add_library(bave::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(src/fmt)
add_subdirectory(src/glm)

if(NOT ANDROID)
  set(GLFW_VULKAN_STATIC OFF)
  set(GLFW_INSTALL OFF)
  set(GLFW_BUILD_DOCS OFF)
  set(GLFW_BUILD_EXAMPLES OFF)
  set(GLFW_BUILD_TESTS OFF)
  add_subdirectory(src/glfw)
  add_library(glfw::glfw ALIAS glfw)

  add_subdirectory(src/dear_imgui)
  add_subdirectory(src/clap)
endif()

add_subdirectory(src/dyvk)
add_subdirectory(src/vma)
add_subdirectory(src/stb)

if(BAVE_USE_FREETYPE)
  add_subdirectory(src/freetype)
endif()

set(CAPO_BUILD_EXAMPLES OFF)

# TODO set ON
set(CAPO_USE_OPENAL OFF)
add_subdirectory(src/capo-lite)

add_subdirectory(src/djson)

target_compile_definitions(glm INTERFACE
  GLM_FORCE_XYZW_ONLY
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_SILENT_WARNINGS
  GLM_ENABLE_EXPERIMENTAL
)

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(glm INTERFACE -Wno-volatile)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  target_compile_options(glm INTERFACE -Wno-deprecated-volatile)
endif()

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
  glm::glm
  dyvk::dyvk
  vma::vma
  stb-image::stb-image
  djson::djson
  capo::lite

  $<$<BOOL:${BAVE_USE_FREETYPE}>:freetype::freetype>

  Threads::Threads
)

target_compile_definitions(${PROJECT_NAME} INTERFACE
  $<$<BOOL:${BAVE_USE_FREETYPE}>:BAVE_USE_FREETYPE>
)

if(NOT ANDROID)
  target_compile_definitions(${PROJECT_NAME} INTERFACE
    BAVE_IMGUI
  )

  target_link_libraries(${PROJECT_NAME} INTERFACE
    dear_imgui::dear_imgui
    glfw::glfw
    clap::clap
  )
endif()
