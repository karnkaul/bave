add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
  include
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_include_directories(${PROJECT_NAME} PRIVATE
  .
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  bave::bave-ext
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::bave-compile-options
)

set(generated_version_header "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/build_version.hpp")
configure_file(src/build_version.hpp.in "${generated_version_header}" @ONLY)

set(glob_expr "include/*.?pp" "src/*.cpp")

if(ANDROID)
  set(glob_expr ${glob_expr} "platform/android/*.cpp")
else()
  set(glob_expr ${glob_expr} "platform/desktop/*.cpp")
endif()

file(GLOB_RECURSE sources CONFIGURE_DEPENDS LIST_DIRECTORIES false ${glob_expr})
set(sources "${sources};${generated_version_header}")

target_sources(${PROJECT_NAME} PRIVATE ${sources})

if(BAVE_USE_PCH)
  target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vulkan/vulkan.hpp>
    <fmt/format.h>
    <algorithm>
    <atomic>
    <chrono>
    <cstdint>
    <deque>
    <filesystem>
    <format>
    <functional>
    <future>
    <memory>
    <mutex>
    <optional>
    <span>
    <string>
    <string_view>
    <unordered_map>
    <unordered_set>
    <variant>
    <vector>
  )
endif()
