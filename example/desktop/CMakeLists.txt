project(bave-example)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  bave::flappy
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)

if(MSVC)
  target_link_options(${PROJECT_NAME} PRIVATE
    /ENTRY:mainCRTStartup
    $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
  )
endif()
