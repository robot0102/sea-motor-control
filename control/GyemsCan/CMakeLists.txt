# Create a library
add_library (gyems_can_lib gyems_can_functions.cpp)
target_link_libraries(gyems_can_lib LINK_PUBLIC
                      socketcan_lib)
target_include_directories(gyems_can_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
