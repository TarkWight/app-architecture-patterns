cmake_minimum_required(VERSION 4.2.1)
project("MVP - Counter" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format check" ON)

# in presets:
set(CLANG_TIDY_PATH "" CACHE FILEPATH "Path to clang-tidy executable")
set(CLANG_FORMAT_PATH "" CACHE FILEPATH "Path to clang-format executable")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(develop
  src/main.cpp
  src/Domain/CounterId.hpp
  src/Domain/Command.hpp
)
target_link_libraries(develop PRIVATE Qt::Core Qt::Widgets)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CodeQuality)
enable_code_quality(develop)

if (APPLE)
  set_target_properties(develop PROPERTIES MACOSX_BUNDLE TRUE)
endif()
