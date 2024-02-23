cmake_minimum_required(VERSION 3.5)

file(GLOB SOURCES
  "src/*.hpp"
  "src/*.cpp"
)
set(TARGET nopf-core)

project(${TARGET} LANGUAGES CXX)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_executable(${TARGET} ${SOURCES})
endif()
    
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_executable(${TARGET} WIN32 ${SOURCES})
endif()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-W -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-W -Wall -O2")
target_link_libraries(${TARGET} WinDivert ws2_32 pthread)