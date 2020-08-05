project(baku)
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)

find_package(fmt)
find_package(pthread)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/3rd_party/libzippp/cmake)
find_package(ZIP REQUIRED)
set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(${CMAKE_SOURCE_DIR}/3rd_party/json)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd_party/libzippp)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd_party/selene)

add_library(lodepng ${CMAKE_SOURCE_DIR}/3rd_party/lodepng/lodepng.cpp)

set(BAKU_SRCS
  ${CMAKE_SOURCE_DIR}/src/utility.cc
  ${CMAKE_SOURCE_DIR}/src/core/feed.cc
  ${CMAKE_SOURCE_DIR}/src/core/pour.cc
  ${CMAKE_SOURCE_DIR}/src/meta.cc
  )

add_library(baku ${BAKU_SRCS})

target_link_libraries(baku
  PUBLIC fmt::fmt ${ZIP_LIBRARIES} selene::selene nlohmann_json::nlohmann_json lodepng)
target_include_directories(baku
  PRIVATE ${CMAKE_SOURCE_DIR}/3rd_party/libzippp
  ${CMAKE_SOURCE_DIR}/3rd_party/selene
  ${CMAKE_SOURCE_DIR}/3rd_party/json/include
  
  PUBLIC ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/3rd_party/digestpp
  ${CMAKE_SOURCE_DIR}/3rd_party/argparse/include
  ${CMAKE_SOURCE_DIR}/3rd_party/lodepng)

set(BAKU_EXE_SRCS ${CMAKE_SOURCE_DIR}/src/main.cc)
add_executable(baku_exe ${BAKU_EXE_SRCS})
set_target_properties(baku_exe
  PROPERTIES
  OUTPUT_NAME "baku")
target_link_libraries(baku_exe
  PUBLIC baku
  PRIVATE pthread
  )

