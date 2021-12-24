# Create a library
add_library (renishaw_can_lib renishaw_can_functions.cpp)

target_include_directories(renishaw_can_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
