project(bave-ext)

add_library(${PROJECT_NAME} INTERFACE)
add_library(bave::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# TODO: unzip
#
add_subdirectory(fmt)

target_link_libraries(${PROJECT_NAME} INTERFACE
  fmt::fmt
)
