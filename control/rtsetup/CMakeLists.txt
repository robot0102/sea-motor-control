# Create a library
add_library (rtsetup_lib rtsetup.c)
add_library (rtutils_lib rtutils.c)

target_include_directories(rtsetup_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(rtutils_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
