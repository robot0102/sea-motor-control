# Create a library
add_library (socketcan_lib SocketCAN.cpp)

target_include_directories(socketcan_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})