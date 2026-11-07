cmake_minimum_required(VERSION 3.16)
project(console_pc CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(SDL2 REQUIRED)

add_library(engine STATIC
  engine/src/engine.cpp
  engine/src/renderer.cpp
  engine/src/mixer.cpp
  platform/pc/hal_sdl.cpp)
target_include_directories(engine PUBLIC engine/include)
if(TARGET SDL2::SDL2)
  target_link_libraries(engine PUBLIC SDL2::SDL2)
else()
  target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(engine PUBLIC ${SDL2_LIBRARIES})
endif()
target_compile_options(engine PRIVATE -Wall -Wextra)

add_executable(demo games/demo/demo.cpp platform/pc/entry.cpp)
target_link_libraries(demo PRIVATE engine)

add_executable(snake games/snake/snake.cpp platform/pc/entry.cpp)
target_link_libraries(snake PRIVATE engine)

add_executable(spaceshooter games/spaceshooter/spaceshooter.cpp platform/pc/entry.cpp)
target_link_libraries(spaceshooter PRIVATE engine)
