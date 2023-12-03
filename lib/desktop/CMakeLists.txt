target_include_directories(${PROJECT_NAME} PUBLIC
  include
)

target_include_directories(${PROJECT_NAME} PRIVATE
  .
)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${sources})
