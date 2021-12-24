///////////////////////////////////////////////////////////////////////////                                                                     
// spdlog_init.hpp
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

#ifndef SPDLOG_INIT_H
#define SPDLOG_INIT_H

// Logging library:
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// Declare constants:
#define LOG_Q_SIZE 100000

///////////////////////////////////////////////////////////////////////////
// Define loggers:
///////////////////////////////////////////////////////////////////////////

// static auto console_log = spdlog::stdout_color_mt("console");

///////////////////////////////////////////////////////////////////////////
// Initialize asynchronous logger: 
///////////////////////////////////////////////////////////////////////////

std::shared_ptr<spdlog::logger>
async_log_init(char* file_path);

/*
class 
spdlog_wrap {
	private:
	   std::shared_ptr<spdlog::logger> _logger;

	public:
	   spdlog_wrap() {	   
		 // Set _logger to some existing logger:
		 _logger = spdlog::get("some_logger");

		 // Or create directly:
		 //_logger = spdlog::rotating_file_logger_mt("my_logger", ...);
	   }
};
*/	  

#endif