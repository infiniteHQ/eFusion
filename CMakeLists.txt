cmake_minimum_required(VERSION 3.16)
project(module)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

if (MSVC)
    add_compile_options(/utf-8)
endif()

unset(VORTEX_SHARED_LIBRARY CACHE)
unset(CHERRY_SHARED_LIBRARY CACHE)

set(VORTEX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/vortex")
set(VXPATH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vxpath")

if(EXISTS "${VXPATH_FILE}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${VXPATH_FILE}")

    file(READ "${VXPATH_FILE}" VXPATH_CONTENT)
    string(REGEX REPLACE "[\r\n].*" "" VXPATH_CONTENT "${VXPATH_CONTENT}")
    string(STRIP "${VXPATH_CONTENT}" VXPATH_CONTENT)
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" VXPATH_CONTENT "${VXPATH_CONTENT}")

    if(NOT VXPATH_CONTENT STREQUAL "")
        if(VXPATH_CONTENT MATCHES "^~")
            if(WIN32)
                set(VXPATH_HOME "$ENV{USERPROFILE}")
            else()
                set(VXPATH_HOME "$ENV{HOME}")
            endif()
            string(REGEX REPLACE "^~" "${VXPATH_HOME}" VXPATH_CONTENT "${VXPATH_CONTENT}")
        endif()

        file(TO_CMAKE_PATH "${VXPATH_CONTENT}" VXPATH_CONTENT)
        get_filename_component(VXPATH_ROOT "${VXPATH_CONTENT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

        if(IS_DIRECTORY "${VXPATH_ROOT}/build/prod")
            find_library(VORTEX_SHARED_LIBRARY vortex_shared HINTS "${VXPATH_ROOT}/build/prod" NO_DEFAULT_PATH)
            find_library(CHERRY_SHARED_LIBRARY cherry imgui HINTS "${VXPATH_ROOT}/build/prod" NO_DEFAULT_PATH)

            if(VORTEX_SHARED_LIBRARY AND CHERRY_SHARED_LIBRARY)
                set(VORTEX_ROOT "${VXPATH_ROOT}")
                message(STATUS "Using external Vortex from vxpath: ${VORTEX_ROOT}")
            else()
                unset(VORTEX_SHARED_LIBRARY CACHE)
                unset(CHERRY_SHARED_LIBRARY CACHE)
                message(WARNING "vxpath: Vortex libraries not found in '${VXPATH_ROOT}/build/prod', falling back to lib/vortex")
            endif()
        else()
            message(WARNING "vxpath: '${VXPATH_ROOT}/build/prod' does not exist, falling back to lib/vortex")
        endif()
    endif()
endif()

find_library(VORTEX_SHARED_LIBRARY vortex_shared HINTS ${VORTEX_ROOT}/build/prod/)
find_library(CHERRY_SHARED_LIBRARY cherry imgui HINTS ${VORTEX_ROOT}/build/prod/)

include_directories("${VORTEX_ROOT}/lib/spdlog/include")
include_directories("${VORTEX_ROOT}/lib/cherry")
include_directories("${VORTEX_ROOT}/vxcore/include")

if(NOT VORTEX_SHARED_LIBRARY)
    message(FATAL_ERROR "Please build/install the vortex core before compiling modules!")
endif()
if(NOT CHERRY_SHARED_LIBRARY)
    message(FATAL_ERROR "Please build/install the cheery core before compiling modules!")
endif()

file(GLOB_RECURSE SOURCES main/*.cpp)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_library(module SHARED ${SOURCES})
if(UNIX AND NOT APPLE)
target_compile_options(module PRIVATE -Wall -Wextra)
endif()
target_link_libraries(module PRIVATE ${VORTEX_SHARED_LIBRARY} ${CHERRY_SHARED_LIBRARY})
target_include_directories(module PRIVATE ${VORTEX_ROOT}/)
target_include_directories(module PRIVATE ${VORTEX_ROOT}/)

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
"if(EXISTS \"${CMAKE_BINARY_DIR}/bin/$<CONFIG>/\")\n"
"    file(REMOVE_RECURSE \"${CMAKE_BINARY_DIR}/bin/$<CONFIG>/\")\n"
"endif()\n"
)

# Custom target to sync assets
add_custom_target(sync_assets_editor ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Synchronizing assets..."
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/remove_resources_if_exists.cmake
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:module>/
    COMMAND ${CMAKE_COMMAND} -E echo "Assets synchronized."
    DEPENDS ${ASSET_FILES} module
)