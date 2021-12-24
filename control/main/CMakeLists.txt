###################################################################
# Data file macros generator:
###################################################################


###################################################################
# Control program - basic functions:
###################################################################

if(True) # fake a block comment

  add_executable(test_ctrl_mit
    test_mit_can_zhimin.cpp
    digital_filters.c  
    control_funcs.c  
    atidaq_help.c  
    SEA_model.c)  
  
  target_link_libraries(test_ctrl_mit LINK_PUBLIC
    m
    mitcan_lib
    s526_lib
    rtsetup_lib
    rtutils_lib
    thread_service_lib
    daq_lib
    socketcan_lib
    renishaw_can_lib
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog) 
endif()

  
