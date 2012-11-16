#include "checkResMsg.h"

using namespace std;

CheckResMsg:: CheckResMsg()
{
	memset(UOID, 0, 20);
}		

void CheckResMsg:: parse_payload(char* msg_data_buf)
{
	memcpy(UOID, msg_data_buf, 20);
}

void CheckResMsg:: create_payload_data(char* UOID)
{
	memcpy(this->UOID, UOID, 20);	
	header.dataLen = 20;
}

void CheckResMsg:: get_payload(int which,char* msg_data)
{
	
	if(which == LOG_DATA)
	{
		memset(msg_data, 0, 6);
		msg_data[0] = '<';
		memcpy(msg_data + 1, &UOID[16], 4);
		msg_data[5] = '>';	
	}
	else
	{
		memset(msg_data, 0, header.dataLen);	
		memcpy(msg_data, UOID, 20);
	}
	
}

void CheckResMsg:: send_file_payload(int sockfd, char* homeDir) {}
