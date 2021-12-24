###################################################################
# Data file macros generator:
###################################################################


###################################################################
# Control program - basic functions:
###################################################################

if(True) # fake a block comment

  add_executable(test_gyems
    test.cpp)  
  
  target_link_libraries(test_gyems LINK_PUBLIC
    m
    # -larmadillo
    mitcan_lib
    s526_lib
    rtsetup_lib
    rtutils_lib
    thread_service_lib
    daq_lib
    socketcan_lib
    gyems_can_lib
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog) 
endif()

  
