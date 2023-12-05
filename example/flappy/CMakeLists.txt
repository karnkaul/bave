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
