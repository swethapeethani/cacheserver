cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# add the executable
add_executable(client client.c)

add_executable(server server.c)

TARGET_LINK_LIBRARIES(client pthread)
TARGET_LINK_LIBRARIES(server pthread)