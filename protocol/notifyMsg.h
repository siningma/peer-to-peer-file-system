#ifndef NOTIFYMSG_H
#define NOTIFYMSG_H

#include "message.h"

class NotifyMsg : public Msg
{
	public:	
		unsigned char error_code;
		
		NotifyMsg();
		~NotifyMsg();		
		void create_payload_data(unsigned char error_code);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
