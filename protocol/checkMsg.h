#ifndef CHECKMSG_H
#define CHECKMSG_H

#include "message.h"

class CheckMsg : public Msg 
{
	public: 
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
