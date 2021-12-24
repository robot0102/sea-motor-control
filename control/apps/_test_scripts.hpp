///////////////////////////////////////////////////////////////////////////                                                                     
// _test_scripts.hpp
//
// Author: Gabriel Aguirre-Ollinger 
// Documentation start: 06.03.2020
// 
// Description:		 
//					 
//					 
// Modifications record:
//		
// 
///////////////////////////////////////////////////////////////////////////

#ifndef _TEST_SCRIPTS_H 
#define _TEST_SCRIPTS_H 

#include <limits.h> 
#include <pthread.h> 
#include <sched.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
#include <ctype.h> 
#include <math.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <sys/stat.h>

extern "C" {
	#include "model526.h"
	#include "ftconfig.h" 
	#include "rtsetup.h"
	#include "rtutils.h" 
	#include "atidaq_help.h" 
	#include "data_file_macros_gen.h"
	#include "control_funcs.h"  
	#include "digital_filters.h"
	#include "SEA_model.h" 
} 

#include "innfos_can_functions.hpp"
#include "innfoscan_help.hpp"
#include "thread_service.hpp"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"   

// Logger library:
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h" 
#include "spdlog/sinks/stdout_color_sinks.h" 

#include "spdlog_init.hpp" 

// Code-generated data logging  scripts (see data_file_macros_gen.c):
#include "_macros_files_sys.h" 

///////////////////////////////////////////////////////////////////////////
// Constant declarations:
///////////////////////////////////////////////////////////////////////////

// Control modes
#define	MODE_VEL	1
#define	MODE_CURR	2
#define	MODE_P_CTRL	3

extern int ctrl_mode; 

// Conversion factors: 
#define NS_PER_MS 1000000 
#define MS_PER_S  1000

///////////////////////////////////////////////////////////////////////////
// Function declarations:
///////////////////////////////////////////////////////////////////////////

void* test_ctrl_multi_mode(void* dt_ns_ref); 
void* test_ctrl_imped(void* dt_ns_ref); 
void* test_ctrl_SEA(void* dt_ns_ref); 

#endif