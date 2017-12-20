cmake_minimum_required(VERSION 3.8)
project(archivec_core)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

set(SOURCE_FILES
        src/util/util.hpp
        src/util/native.hpp
        src/util/messagebox.hpp
        src/archiver.cpp
        src/archiver.hpp
        src/util/archiveccore.hpp
        src/util/archiveccore.cpp
        src/util/compressors/lzw/lzw.hpp
        src/util/compressors/lzw/bitstreamwriter.cpp
        src/util/compressors/lzw/bitstreamwriter.hpp
        src/util/compressors/lzw/bitstreamreader.cpp
        src/util/compressors/lzw/bitstreamreader.hpp
        src/util/compressors/lzw/dictionary.cpp
        src/util/compressors/lzw/dictionary.hpp)

add_library(archivec_core SHARED ${SOURCE_FILES})
target_link_libraries(archivec_core ${Boost_LIBRARIES})

add_executable(archivec main.cpp ${SOURCE_FILES})
target_link_libraries(archivec archivec_core)