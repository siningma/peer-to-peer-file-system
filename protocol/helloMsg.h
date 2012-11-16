#ifndef HELLOMSG_H
#define HELLOMSG_H

#include "message.h"

class HelloMsg : public Msg
{
	public:	
		uint16_t port;
		char* hostname;
		
		HelloMsg();
		~HelloMsg();
		
		void create_payload_data(char* host_port, char* hostname);	//create a new hello msg payload
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
