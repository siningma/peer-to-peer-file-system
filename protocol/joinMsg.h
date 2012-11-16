#ifndef JOINMSG_H
#define JOINMSG_H

#include "message.h"

class JoinMsg : public Msg
{
	public:
		uint32_t location;	
		uint16_t port;
		char* hostname;
		
		JoinMsg();
		~JoinMsg();

		void create_payload_data(uint32_t location, char* port, char* hostname);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
