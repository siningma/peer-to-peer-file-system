#ifndef JOINRESMSG_H
#define JOINRESMSG_H

#include "message.h"

class JoinResMsg : public Msg
{
	public:
		char UOID[20];
		uint32_t distance;
		uint16_t host_port;
		char* hostname;
		
		JoinResMsg();
		~JoinResMsg();
		
		void create_payload_data(char* UOID, uint32_t distance, uint16_t host_port, char* hostname);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
