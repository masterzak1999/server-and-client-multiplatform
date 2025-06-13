# CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(remote_cmd C)

add_executable(server server.c common.h)
add_executable(client client.c common.h)

if(WIN32)
  target_link_libraries(server PRIVATE ws2_32)
  target_link_libraries(client PRIVATE ws2_32)
endif()
