# Create a library
add_library (thread_service_lib thread_service.cpp)

target_include_directories(thread_service_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/rtsetup)
