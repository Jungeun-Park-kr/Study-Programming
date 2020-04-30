

#ifndef true
    #define false 0
#endif

#ifndef false
    #define true 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif

void ssu_mntr();
void do_Prompt();
void doHelp();
void do_Monitor(char *logFile);
pid_t monitor_deamon_init();
