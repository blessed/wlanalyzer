project(wldump)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(src SRC_LIST)
file(GLOB HEADERS src/*.h src/*.h)
file(GLOB SOURCES src/*.cpp src/*.c)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

add_executable(client src/client.cpp src/server_socket.cpp src/socket.cpp src/common.cpp)
