#ifndef SEARCHRESMSG_H
#define SEARCHRESMSG_H

#include "message.h"

class SearchResMsg : public Msg
{
	public: 
		char UOID[20];
		std::vector<uint32_t> matchFile_size;
		std::vector<UOID_Map> matchFileID;
		std::vector<uint32_t> matchFile_index;
		bool fileState;
		
		SearchResMsg();

		void create_payload_data(char* UOID);
		void parse_payload(char* msg_data_buf);			
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
