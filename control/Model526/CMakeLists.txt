# Create a library
add_library (s526_lib model526.c)

target_include_directories(s526_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
