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

target_include_directories(${PROJECT_NAME} PUBLIC
  include
)

target_include_directories(${PROJECT_NAME} PRIVATE
  .
)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${sources})

target_link_libraries(${PROJECT_NAME} PUBLIC
  android-app-glue
  log
)
