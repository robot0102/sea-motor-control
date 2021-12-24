cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

# Project name
project (ATIDAQ LANGUAGES CXX C)

# Other build flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -march=atom -mtune=atom")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DFDEBUG -DNDEBUG")

# Link to required libraries:
find_package(Threads REQUIRED) 
find_package(spdlog REQUIRED) 
find_package(Eigen3 3.3 REQUIRED NO_MODULE) # Matrix library

# Redirect binaries to bin folder:
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Add sub directories
add_subdirectory(Model526) 
add_subdirectory(SocketCAN) 
add_subdirectory(rtsetup) 
add_subdirectory(thread_service) 
add_subdirectory(ATIDAQ) 
# add_subdirectory(InnfosCAN) 
add_subdirectory(MITcan) 
add_subdirectory(RenishawCAN)
# add_subdirectory(calibration)
# add_subdirectory(apps)
add_subdirectory(main)
add_subdirectory(GyemsCan)
add_subdirectory(test_gyems)
