///////////////////////////////////////////////////////////////////////////                                                                     
// spdlog_init.cpp
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

#include "spdlog_init.hpp"

std::shared_ptr<spdlog::logger>
async_log_init(char* file_path) {

	char* logger_id = file_path; // file path string doubles as logger ID

	std::shared_ptr<spdlog::logger> async_log;

	spdlog::init_thread_pool(LOG_Q_SIZE, 1); 
	
	printf("\nfile_path = [%s]\n\n", (char*)file_path); 
	
	async_log = spdlog::create_async_nb<spdlog::sinks::basic_file_sink_mt>(logger_id, file_path);  //was ("async_file_logger", file_path)
	async_log->set_level(spdlog::level::trace);  
	
	async_log->set_pattern("%v");  

	return async_log;
}
