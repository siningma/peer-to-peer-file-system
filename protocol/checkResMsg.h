#ifndef CHECKRESMSG_H
#define CHECKRESMSG_H

#include "message.h"

class CheckResMsg : public Msg
{
	public:	
		char UOID[20];
			
		CheckResMsg();		
		void create_payload_data(char* UOID);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
