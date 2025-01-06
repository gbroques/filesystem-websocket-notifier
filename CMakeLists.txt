cmake_minimum_required(VERSION 3.15)
project(filesystem-websocket-notifier CXX)

find_package(fswatch REQUIRED)
find_package(simple-websocket-server REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} fswatch::fswatch simple-websocket-server::simple-websocket-server)
