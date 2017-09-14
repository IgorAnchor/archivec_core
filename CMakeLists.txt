cmake_minimum_required(VERSION 3.8)
project(archivec_core)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS}  ${Boost_INCLUDE_DIR})

set(SOURCE_FILES src/jnimain.cpp src/jnimain.hpp main.cpp src/filemanager.hpp src/util/native.hpp src/util/message.hpp src/fstest.hpp)
add_library(archivec_core SHARED ${SOURCE_FILES})
target_link_libraries(archivec_core ${Boost_LIBRARIES})

add_executable(archivec main.cpp)
target_link_libraries(archivec archivec_core )