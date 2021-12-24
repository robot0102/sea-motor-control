
# Create a library
add_library (innfos_can_lib innfos_can_functions.cpp)

target_include_directories(innfos_can_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
