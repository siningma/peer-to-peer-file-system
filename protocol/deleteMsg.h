#ifndef DELETEMSG_H
#define DELETEMSG_H

#include "message.h"

class DeleteMsg : public Msg
{
	public:	
		char deleteFilename[256];
		unsigned char deleteSHA1[SHA_DIGEST_LENGTH];
		unsigned char deleteNonce[UOID_LEN];
		unsigned char deletePass[UOID_LEN];
		
		DeleteMsg();
		void create_payload_data(char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce, unsigned char* deletePass);	
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
