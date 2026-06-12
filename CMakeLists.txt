cmake_minimum_required(VERSION 3.12)
project(aeolipile VERSION 1.0)

file(GLOB_RECURSE UTILS_FILES "utils/*.cxx")
file(GLOB_RECURSE CORE_FILES "core/*.cxx")

add_library(aeolipile ${UTILS_FILES} ${CORE_FILES})

target_include_directories(aeolipile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<INSTALL_INTERFACE:include>
)


add_subdirectory(libs)
add_subdirectory(example)
target_compile_features(aeolipile PUBLIC cxx_std_20)
target_link_libraries(aeolipile PRIVATE medusa glfw)
