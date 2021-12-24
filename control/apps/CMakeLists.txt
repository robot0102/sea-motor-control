###################################################################
# Data file macros generator:
###################################################################

add_executable(test_data_file_macros_gen
  _main_file_gen.c
  data_file_macros_gen.c)  
    
target_link_libraries(test_data_file_macros_gen LINK_PUBLIC
  m)

###################################################################
# Control program - basic functions:
###################################################################

if(FALSE) # fake a block comment

  add_executable(test_ctrl_basic
    _main_ctrl_basic.cpp 
    data_file_macros_gen.c 
    _test_ctrl_multi_mode.cpp 
    atidaq_help.c
    innfoscan_help.cpp
    spdlog_init.cpp)  
  
  target_link_libraries(test_ctrl_basic LINK_PUBLIC
    m
    s526_lib
    rtsetup_lib
    rtutils_lib
    thread_service_lib
    daq_lib
    innfos_can_lib
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog)
  
endif()
  
###################################################################
# Control program - impedance control:
###################################################################

if(FALSE) # fake a block comment

  add_executable(test_ctrl_imped
    _main_ctrl_imped.cpp 
    data_file_macros_gen.c 
    _test_ctrl_imped.cpp 
    digital_filters.c
    control_funcs.c
    atidaq_help.c
    innfoscan_help.cpp
    spdlog_init.cpp)  
  
  target_link_libraries(test_ctrl_imped LINK_PUBLIC
    m
    s526_lib
    rtsetup_lib
    rtutils_lib
    thread_service_lib
    daq_lib
    innfos_can_lib
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog)
  
endif()
  
###################################################################
# Control program - SEA trajectory control:
###################################################################

if(TRUE) # fake a block comment

  add_executable(test_ctrl_SEA
    _main_ctrl_SEA.cpp 
    data_file_macros_gen.c 
    _test_ctrl_SEA.cpp 
    digital_filters.c
    control_funcs.c
    atidaq_help.c
    innfoscan_help.cpp
    spdlog_init.cpp
    SEA_model.c)  
  
  target_link_libraries(test_ctrl_SEA LINK_PUBLIC
    m
    s526_lib
    rtsetup_lib
    rtutils_lib
    thread_service_lib
    daq_lib
    innfos_can_lib
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog)
  
endif()
  
  

  
