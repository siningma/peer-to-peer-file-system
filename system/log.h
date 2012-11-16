#ifndef LOG_H
#define LOG_H

#include "tool.h"
#include "../protocol/message.h"

class Log
{
	public: 
		pthread_mutex_t write_lock;
		FILE* pFile;
		
		Log(char* homeDir);
		~Log();
		void write_to_file(char type, const char* neighbor, Msg* msg);
		void write_to_file(char type, const char* neighbor, unsigned char* data, int logDataLen, MsgHeader header);
		void close_log(char* hostname, char* port, int type);
		void print_msgType(unsigned char msgType);
};

#endif
