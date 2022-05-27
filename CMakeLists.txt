cmake_minimum_required (VERSION 3.8)
project ("WoWIngameEditor")

# check if compiler is suitable
if(NOT MSVC)
 message(FATAL_ERROR "Only MSVC compiler is currently supported.")
endif()

# check if OS is suitable
if(NOT WIN32)
  message(FATAL_ERROR "This project only supports the Windows platform.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(EXTERNAL_SOURCE_DIR "${PROJECT_SOURCE_DIR}/external")

include(CheckCXXCompilerFlag)
include("cmake/file_globbing.cmake")
include("cmake/compiler_flags.cmake")

#Better exception handling for visual studio, particularly for the asynchronous stuff
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /EHa)
#Multi core building for visual studio
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /MP)
#Allow Big obj for msvc compilation
add_compiler_flag_if_supported(CMAKE_CXX_FLAGS /bigobj)

# options

OPTION(ADDITIONAL_OPTIMIZATION_FLAGS "Enabled optimizations?" OFF)
if(ADDITIONAL_OPTIMIZATION_FLAGS)
  MESSAGE( STATUS "Enabled additional optimization flags for MSVC.")
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ob2) # inline any suitable functions
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Oi)  # enable intrasic functions
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ot)  # favor fast code
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /GL)  # whole program optimization
endif()


collect_files(sources_files src TRUE "*.c;*.cpp;" "")
collect_files(headers_files src TRUE "*.h;*.hpp;*.inl" "")

assign_source_group(
  ${sources_files} 
  ${headers_files}
)

# dependencies
add_subdirectory("${EXTERNAL_SOURCE_DIR}/MinHook")

add_library(EpsilonAddon SHARED ${sources_files} ${headers_files})
target_link_libraries(EpsilonAddon d3d11.lib minhook)
target_include_directories(EpsilonAddon PRIVATE "${EXTERNAL_SOURCE_DIR}/MinHook/include" "src")

# post build hooks
#[[
add_custom_command(TARGET EpsilonAddon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/src/shaders"
        $<TARGET_FILE_DIR:EpsilonAddon>/shaders
        COMMAND ${CMAKE_COMMAND} -E remove -f 
        $<TARGET_FILE_DIR:EpsilonAddon>/shaders/.git)

]]



