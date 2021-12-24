#include "thread_service.hpp"


int 
start_periodic_thread(void*(*periodic_func) (void *), void* func_arg, int priority) {
	struct sched_param param; 
	pthread_attr_t attr; 
	pthread_t thread; 
	int ret; 

	// Lock memory:
	if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
		printf("mlockall failed: %m\n");  
		exit(-2); 
	}
	
	// Initialize pthread attributes (default values):
	ret = pthread_attr_init(&attr); 
	if (ret) {
		printf("init pthread attributes failed\n");
		goto OUT; 
	}
	
	// Set a specific stack size : 
	ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
	if (ret) { 
		printf("pthread setstacksize failed\n");
		goto OUT; 
	}

	// Set scheduler policy and priority of pthread: 
	ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (ret) { 
		printf("pthread setschedpolicy failed\n"); 
		goto OUT; 
	}

	param.sched_priority = priority; 
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) {
		printf("pthread setschedparam failed\n");
		goto OUT; 
	}
	// Use scheduling parameters of attr:
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED); 
	if (ret) { 
		printf("pthread setinheritsched failed\n");  
		goto OUT; 
	}

	// Create a pthread with specified attributes:
	printf("Calling pthread_create() ... \n\n"); 

	ret = pthread_create(&thread, &attr, periodic_func, func_arg);
	if (ret) {
		printf("create pthread failed\n");  
		goto OUT;
	}

	// Join the thread and wait until it is done:
	printf("Calling pthread_join() ... \n\n");   

	ret = pthread_join(thread, NULL);   
	if (ret)
		printf("join pthread failed: %m\n");   

	OUT: 
	return ret;
}

