#ifndef GETRESMSG_H
#define GETRESMSG_H

#include "message.h"

class GetResMsg : public Msg
{
	public:	
		char UOID[20];
		uint32_t fileIndex;
		uint32_t get_metadatafileSize;
		bool fileState;
		 
		GetResMsg();
		
		void create_payload_data(char* UOID, uint32_t fileIndex, uint32_t metaData_fileSize, char* homeDir);	//create a new get msg payload
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir); 
};

#endif
