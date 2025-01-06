cmake_minimum_required(VERSION 3.15)
project(filesystem-websocket-notifier CXX)

find_package(efsw)
find_package(simple-websocket-server)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} efsw::efsw simple-websocket-server::simple-websocket-server)
