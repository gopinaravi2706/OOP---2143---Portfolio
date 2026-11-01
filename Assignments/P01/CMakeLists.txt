cmake_minimum_required(VERSION 3.20)
project(CardGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(ftxui
  URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v6.1.9.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ftxui)

set(FTXUI_LIBS ftxui::component)

# Milestone 0: toolchain check.
add_executable(check check/check.cpp)
target_link_libraries(check PRIVATE ${FTXUI_LIBS})

# The game. Every .cpp in src/ is compiled automatically.
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(game ${GAME_SOURCES})
target_include_directories(game PRIVATE include)
target_link_libraries(game PRIVATE ${FTXUI_LIBS})

if(MINGW)
  target_link_options(check PRIVATE -static)
  target_link_options(game  PRIVATE -static)
endif()