file(GLOB HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
  )

# Create a library
add_library (daq_lib
  atidaq_funcs.c
  ftconfig.c
  ftrt.c
  dom.c
  expatls.c
  node.c
  stack.c
  xmlparse.c
  xmlrole.c
  xmltok.c)

target_include_directories(daq_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
