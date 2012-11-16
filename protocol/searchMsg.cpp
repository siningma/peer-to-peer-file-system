#include "searchMsg.h"

using namespace std;

SearchMsg:: SearchMsg(): searchType(0) {}

void SearchMsg:: create_payload_data(unsigned char type, string searchQuery)
{
	this->searchType = type;
	this->searchQuery = searchQuery;
	header.dataLen = 1 + searchQuery.length();
}

void SearchMsg:: parse_payload(char* msg_data_buf)
{
	uint32_t len = header.dataLen - 1;
	searchType = (unsigned char)msg_data_buf[0];
	string s(&msg_data_buf[1], len);
	searchQuery = s;
}

void SearchMsg:: get_payload(int which,char* msg_data)
{
	if(which == LOG_DATA)
	{	
		switch(searchType)
		{
			case 1:
			sprintf(msg_data, "<filename> <%s>", searchQuery.c_str());	
			break;
			case 2:
			sprintf(msg_data, "<sha1hash> <%s>", searchQuery.c_str());	
			break;
			case 3:
			sprintf(msg_data, "<keywords> <%s>", searchQuery.c_str());	
			break;
			default:
			break;
		}
	}	
	else
	{
		memset(msg_data, 0, header.dataLen);
		msg_data[0] = searchType;
		memcpy(msg_data + 1, searchQuery.c_str(), header.dataLen - 1);
	}	
}

void SearchMsg:: send_file_payload(int sockfd, char* homeDir) {}
