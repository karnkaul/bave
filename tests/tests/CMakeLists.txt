project(${project_prefix}-tests)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS LIST_DIRECTORIES false "*.cpp")

if(NOT "${sources}" STREQUAL "")
  add_executable(${PROJECT_NAME})
  target_sources(${PROJECT_NAME} PRIVATE ${sources})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${project_prefix}::${project_prefix}-test)

  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
endif()
