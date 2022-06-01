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

option(ADDITIONAL_OPTIMIZATION_FLAGS "Enable optimizations?" OFF)
if(ADDITIONAL_OPTIMIZATION_FLAGS)
  message( STATUS "Enabled additional optimization flags for MSVC.")
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ob2) # inline any suitable functions
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Oi)  # enable intrasic functions
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Ot)  # favor fast code
  add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /GL)  # whole program optimization
endif()

option(ENABLE_VALIDATION_LOG_TO_CONSOLE "Log to console?" ON)
if(ENABLE_VALIDATION_LOG_TO_CONSOLE)
  message( STATUS "Logging to console")
  add_definitions(-DVALIDATION_LOG_TO_CONSOLE)
endif()

option(ENABLE_DEBUG_LOG_IN_RELEASE "Enable debug logs in Release?" OFF)
if(ENABLE_DEBUG_LOG_IN_RELEASE)
  message( STATUS "Enabled debug logs in Release")
  add_definitions(-DDEBUG_LOG_IN_RELEASE)
endif()

option(ENABLE_CONTRACTS_IN_RELEASE "Enable contract validation in Release?" OFF)
if(ENABLE_CONTRACTS_IN_RELEASE)
  message( STATUS "Enabled contract validation in Release")
  add_definitions(-DENABLE_CONTRACTS_IN_RELEASE)
endif()

# logging flags
set(LOGGING_FLAGS_ 0x0)

option(LOGGING_FLAGS_GRAPHICS "Enable graphics loggers?" ON)
if(LOGGING_FLAGS_GRAPHICS)
  math(EXPR LOGGING_FLAGS_ "${LOGGING_FLAGS_} | 0x1")
endif()

option(LOGGING_FLAGS_CLIENT_HOOKS "Enable client hooks loggers?" ON)
if(LOGGING_FLAGS_CLIENT_HOOKS)
  math(EXPR LOGGING_FLAGS_ "${LOGGING_FLAGS_} | 0x2")
endif()

add_definitions(-DLOGGING_FLAGS=${LOGGING_FLAGS_})

# contract flags
set(CONTRACT_FLAGS_ 0x0)

option(CONTRACT_FLAGS_FILE_IO "Enable file IO contracts?" ON)
if(CONTRACT_FLAGS_FILE_IO)
  math(EXPR CONTRACT_FLAGS_ "${CONTRACT_FLAGS_} | 0x1")
endif()

add_definitions(-DCONTRACT_FLAGS=${CONTRACT_FLAGS_})

# define base source dir path to use in compile time
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

collect_files(sources_files src TRUE "*.c;*.cpp;" "")
collect_files(headers_files src TRUE "*.h;*.hpp;*.inl" "")

collect_files(backward_sources_files "${EXTERNAL_SOURCE_DIR}/backward-cpp" FALSE "*.c;*.cpp;" "")
collect_files(backward_headers_files "${EXTERNAL_SOURCE_DIR}/backward-cpp" FALSE "*.h;*.hpp;*.inl" "")

assign_source_group(
  ${sources_files} 
  ${headers_files}
)

# dependencies
add_subdirectory("${EXTERNAL_SOURCE_DIR}/MinHook")

add_library(EpsilonAddon SHARED ${sources_files} ${headers_files} ${backward_sources_files} ${backward_headers_files})
target_link_libraries(EpsilonAddon d3d11.lib minhook)
target_include_directories(EpsilonAddon PRIVATE "${EXTERNAL_SOURCE_DIR}/MinHook/include" "${EXTERNAL_SOURCE_DIR}/bertrand/include" "${EXTERNAL_SOURCE_DIR}/backward-cpp" "src")

# post build hooks
#[[
add_custom_command(TARGET EpsilonAddon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/src/shaders"
        $<TARGET_FILE_DIR:EpsilonAddon>/shaders
        COMMAND ${CMAKE_COMMAND} -E remove -f 
        $<TARGET_FILE_DIR:EpsilonAddon>/shaders/.git)

]]



# tests
add_executable(logging_test "tests/LoggingTest.cpp" ${sources_files} ${headers_files} ${backward_sources_files} ${backward_headers_files})
target_link_libraries(logging_test d3d11.lib minhook)
target_include_directories(logging_test PRIVATE "${EXTERNAL_SOURCE_DIR}/MinHook/include" "${EXTERNAL_SOURCE_DIR}/bertrand/include" "${EXTERNAL_SOURCE_DIR}/backward-cpp" "src")

add_executable(contract_test "tests/ContractsTest.cpp" ${sources_files} ${headers_files} ${backward_sources_files} ${backward_headers_files})
target_link_libraries(contract_test d3d11.lib minhook)
target_include_directories(contract_test PRIVATE "${EXTERNAL_SOURCE_DIR}/MinHook/include" "${EXTERNAL_SOURCE_DIR}/bertrand/include" "${EXTERNAL_SOURCE_DIR}/backward-cpp" "src")

