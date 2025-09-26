cmake_minimum_required(VERSION 3.16)
project(module)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

if (MSVC)
    add_compile_options(/utf-8)
endif()
find_library(VORTEX_SHARED_LIBRARY vortex_shared HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex/build/prod/)
find_library(CHERRY_SHARED_LIBRARY cherry imgui HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex/build/prod/)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex/lib/spdlog/include")

if(NOT VORTEX_SHARED_LIBRARY)
    message(FATAL_ERROR "Please build/install the vortex core before compiling modules!")
endif()
if(NOT CHERRY_SHARED_LIBRARY)
    message(FATAL_ERROR "Please build/install the vortex core before compiling modules!")
endif()

file(GLOB_RECURSE SOURCES main/*.cpp)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_library(module SHARED ${SOURCES})
if(UNIX AND NOT APPLE)
target_compile_options(module PRIVATE -Wall -Wextra)
endif()
target_link_libraries(module PRIVATE ${VORTEX_SHARED_LIBRARY} ${CHERRY_SHARED_LIBRARY})
target_include_directories(module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex/main/include/)
target_include_directories(module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex/)

file(GLOB_RECURSE ASSET_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
)

# Configure per-config output directories
foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
endforeach()

# Script to remove existing resources
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/remove_resources_if_exists.cmake
"if(EXISTS \"${CMAKE_BINARY_DIR}/bin/$<CONFIG>/resources\")\n"
"    file(REMOVE_RECURSE \"${CMAKE_BINARY_DIR}/bin/$<CONFIG>/resources\")\n"
"endif()\n"
)

# Custom target to sync assets
add_custom_target(sync_assets_editor ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Synchronizing assets..."
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/remove_resources_if_exists.cmake
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:module>/resources
    COMMAND ${CMAKE_COMMAND} -E echo "Assets synchronized."
    DEPENDS ${ASSET_FILES} module
)
