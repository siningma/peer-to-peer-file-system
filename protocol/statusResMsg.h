#ifndef STATUSRESMSG_H
#define STATUSRESMSG_H

#include "message.h"
#include "../system/helper.h"

using namespace std;

class StatusResMsg : public Msg
{
	public: 
		char UOID[UOID_LEN];
		uint16_t hostInfoLen;
		uint16_t hostPort;
		char* hostname;
		vector<uint32_t> recordLengths;
		vector<Neighbor_node*> neighbors;
		vector<uint32_t> fileIndexes;
		bool fileState;
		
		StatusResMsg();
		~StatusResMsg();
		
		void create_payload_data(char* UOID, uint16_t hostInfoLen, uint16_t hostPort, char* hostname);
		void create_payload_neighbor_data(Neighbor_node* neighbor_node);
		void create_payload_file_data(char* homeDir, uint32_t file_index);
		void parse_payload(char* msg_data_buf);	
		void parse_file_payload(char* msg_data_buf);	
		void get_payload(int which,char* msg_data);
		void send_file_payload(int sockfd, char* homeDir);
};

#endif
