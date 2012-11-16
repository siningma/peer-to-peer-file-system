#ifndef STOREMSG_H
#define STOREMSG_H

#include "message.h"

class StoreMsg: public Msg
{
	public:
		uint32_t file_index;
		uint32_t meta_fileSize;
		
		StoreMsg();
		
		void create_payload_data(uint32_t inputIndex, char* homeDir);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);	
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
