#ifndef LOGGER_H
#define LOGGER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE* open_log(char* name);
void log_msg(FILE* log, char* msg);
#endif
