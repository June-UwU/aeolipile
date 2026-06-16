cmake_minimum_required(VERSION 3.12)
project(aeolipile VERSION 1.0)

find_package(Vulkan REQUIRED)

set(SESSION_TYPE $ENV{XDG_SESSION_TYPE})

if(SESSION_TYPE STREQUAL "wayland")
    message(STATUS "Display server detected: Wayland")
    add_compile_definitions(AEO_WAYLAND)
elseif(SESSION_TYPE STREQUAL "x11")
    message(STATUS "Display server detected: X11")
    add_compile_definitions(AEO_X11)
else()
    if(DEFINED ENV{WAYLAND_DISPLAY})
        message(STATUS "Display server detected via fallback: Wayland")
        add_compile_definitions(AEO_WAYLAND)
    elseif(DEFINED ENV{DISPLAY})
        message(STATUS "Display server detected via fallback: X11")
        add_compile_definitions(AEO_X11)
    else()
        message(WARNING "Could not determine display server.")
    endif()
endif()

file(GLOB_RECURSE UTILS_FILES "utils/*.cxx")
file(GLOB_RECURSE CORE_FILES "core/*.cxx")
file(GLOB_RECURSE VULKAN_FILES "vulkan/*.cxx")

add_library(aeolipile ${UTILS_FILES} ${CORE_FILES} ${VULKAN_FILES})

target_include_directories(aeolipile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/volk>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    ${Vulkan_INCLUDE_DIRS}
    $<INSTALL_INTERFACE:include>
)


add_subdirectory(libs)
add_subdirectory(example)
target_compile_features(aeolipile PUBLIC cxx_std_20)
target_link_libraries(aeolipile PRIVATE medusa glfw Vulkan::Vulkan volk::volk)
