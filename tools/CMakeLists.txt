project(bave-tools)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::bave
  bave::bave-compile-options
  portable-file-dialogs::portable-file-dialogs
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})
