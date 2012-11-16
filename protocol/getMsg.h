#ifndef GETMSG_H
#define GETMSG_H

#include "message.h"

class GetMsg : public Msg
{
	public:	
		unsigned char FileID[UOID_LEN];
		unsigned char msg_SHA1[SHA_DIGEST_LENGTH];
		 
		GetMsg();
		
		void create_payload_data(unsigned char* FileID, unsigned char* msg_SHA1);	//create a new get msg payload
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir); 
};

#endif
