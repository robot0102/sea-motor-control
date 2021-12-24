///////////////////////////////////////////////////////////////////////////                                                                     
// _main_file_gen.c
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

#include <stdio.h>

int 
main(int argc, char *argv[])
{
	// Macros generating function:
	file_mgmt_macros_gen(); 

	printf("\n\n\n\n\n\n\n\n");
	printf("======================================================\n\n");
	printf("CRITICAL: DELETE [CMakeCache.txt] and RUN [cmake .] and [cmake --build .] AGAIN            \n\n");
	printf("======================================================\n\n");
	
	return 0;
}
