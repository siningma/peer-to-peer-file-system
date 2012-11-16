#ifndef STATUSMSG_H
#define STATUSMSG_H

#include "message.h"

class StatusMsg : public Msg
{
	public: 
		unsigned char report_type;
		
		StatusMsg();

		void create_payload_data(unsigned char report_type);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
