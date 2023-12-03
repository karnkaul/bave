if("${project_prefix}" STREQUAL "")
  message(FATAL_ERROR "project_prefix not set")
endif()

project(${project_prefix}-test)

add_library(${PROJECT_NAME})
add_library(${project_prefix}::${project_prefix}-test ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
  test/test.cpp
  test/test.hpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
)

target_include_directories(${PROJECT_NAME} PUBLIC
  .
)

add_subdirectory(tests)
