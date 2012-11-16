#ifndef SEARCHMSG_H
#define SEARCHMSG_H

#include "message.h"

class SearchMsg : public Msg
{
	public:	
		unsigned char searchType;
		string searchQuery;
		
		SearchMsg();

		void create_payload_data(unsigned char type, string searchQuery);	
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
