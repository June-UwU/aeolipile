cmake_minimum_required(VERSION 3.12)
project(app_example VERSION 1.0)

file(GLOB_RECURSE SRC_FILES "*.cxx")

add_executable(app_example ${SRC_FILES})

target_link_libraries(app_example PRIVATE aeolipile)
