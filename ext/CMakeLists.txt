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
endif()

add_subdirectory(src/dyvk)
add_subdirectory(src/vma)
add_subdirectory(src/stb)
add_subdirectory(src/dear_imgui)

if(BAVE_USE_FREETYPE)
  add_subdirectory(src/freetype)
endif()

set(CAPO_BUILD_EXAMPLES OFF)
add_subdirectory(src/capo-lite)

add_subdirectory(src/djson)

add_subdirectory(src/portable-file-dialogs)

target_compile_definitions(glm INTERFACE
  GLM_FORCE_XYZW_ONLY
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_SILENT_WARNINGS
  GLM_ENABLE_EXPERIMENTAL
)

string(FIND "${CMAKE_CXX_COMPILER_ID}" "Clang" is_clang)

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(glm INTERFACE -Wno-volatile)
elseif(NOT is_clang EQUAL -1)
  target_compile_options(glm INTERFACE -Wno-deprecated-volatile)
endif()

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
  glm::glm
  dyvk::dyvk
  vma::vma
  stb-image::stb-image
  dear_imgui::dear_imgui
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
    glfw::glfw
  )
endif()

if(ANDROID)
  set(app_glue_src "${ANDROID_NDK}/sources/android/native_app_glue")

  add_library(android-app-glue STATIC
    ${app_glue_src}/android_native_app_glue.c
  )

  target_include_directories(android-app-glue PUBLIC
    ${app_glue_src}
  )

  target_link_libraries(android-app-glue PUBLIC
    android
  )

  target_link_libraries(${PROJECT_NAME} INTERFACE
    android-app-glue
    log
  )

  target_compile_definitions(dyvk PUBLIC -DVK_USE_PLATFORM_ANDROID_KHR=1)
endif()
