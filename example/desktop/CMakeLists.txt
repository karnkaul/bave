project(bave-example)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::bave
  bave::bave-compile-options
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)
