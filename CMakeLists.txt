cmake_minimum_required(VERSION 3.8)
project(archivec_core)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

set(SOURCE_FILES src/jnimain.cpp src/jnimain.hpp main.cpp src/filemanager.hpp src/util/native.hpp src/util/message.hpp src/fstest.hpp)
add_library(archivec_core SHARED ${SOURCE_FILES})

add_executable(archivec main.cpp)
target_link_libraries(archivec archivec_core)