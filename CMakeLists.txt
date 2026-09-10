cmake_minimum_required(VERSION 3.16)
project(SpaceDB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SpaceDB
  src/aof.cpp
  src/main.cpp
  src/protocol.cpp
  src/server.cpp
  src/snapshot.cpp
  src/store.cpp
)
target_include_directories(SpaceDB PRIVATE include)
target_compile_options(SpaceDB PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(SpaceDB PRIVATE pthread)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME starts COMMAND SpaceDB --help)
endif()