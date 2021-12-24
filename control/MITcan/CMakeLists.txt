# Create a library
add_library (mitcan_lib mitcan.cpp)

target_include_directories(mitcan_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ================= old file ====================
# ================= old file ====================
# cmake_minimum_required (VERSION 3.16)

# project(main)

# # Create a library
# # add_library (mitcan mitcan.cpp)
# aux_source_directory(. DIR_SRC)

# add_executable(main ${DIR_SRC})
# # add_executable(main test.cpp mitcan.cpp SocketCAN.cpp)

# # target_include_directories(mitcan  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
# # target_link_libraries(mitcan socketcan)