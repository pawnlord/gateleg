#include "logger.h"
#include <time.h>
#include <stdarg.h>

FILE* static_log;

FILE* open_log(char* name){
	FILE* fp = fopen(name, "w+");
	static_log = fp;
	return fp;
}

void log_msg(FILE* log, char* msg){
	char* final_msg = malloc(1000);
	memset(final_msg, 0, 1000);
	time_t t = time(NULL);
	char* time_str = ctime(&t);
	time_str[strlen(time_str)-1] = 0;
	sprintf(final_msg, "[gataleg %s]: %s\n", time_str, msg);
	fputs(final_msg, log);
	free(final_msg);
	fflush(log);;
}

void stat_log_msg(char* fmt, ...){
	char* msg = malloc(1000);
	va_list args;
	va_start(args, fmt);
	sprintf(msg, fmt, args);
	log_msg(static_log, msg);
	free(msg);
}
