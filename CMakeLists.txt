cmake_minimum_required(VERSION 3.10)
project(Model_O_N)

# Specify the output directory for the executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Specify where to look for header files
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 20)

# Automatically find HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX)

if(NOT HDF5_FOUND)
    message(FATAL_ERROR "HDF5 was not found.")
endif()

# Source files
set(SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Include HDF5 directories
include_directories(${HDF5_INCLUDE_DIRS})

# Add executable
add_executable(main ${SOURCES})

# Link with the HDF5 libraries
target_link_libraries(main ${HDF5_LIBRARIES})
