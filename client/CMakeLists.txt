cmake_minimum_required(VERSION 3.8)

project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")

find_package(SDL2 REQUIRED)
FIND_PACKAGE(Boost REQUIRED)

include_directories(src)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})

file(GLOB_RECURSE SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/src/*.h
  ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${Boost_LIBRARIES})
