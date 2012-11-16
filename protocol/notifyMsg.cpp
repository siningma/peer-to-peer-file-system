#include "notifyMsg.h"

using namespace std;

NotifyMsg:: NotifyMsg():error_code(0)
{
	header.dataLen = 1;
}

NotifyMsg:: ~NotifyMsg() {}

void NotifyMsg:: parse_payload(char* msg_data_buf)
{
	error_code = (unsigned char)msg_data_buf[0];
}

void NotifyMsg::create_payload_data(unsigned char error_code)
{
	this->error_code = error_code;
	header.dataLen = 1;
}

void NotifyMsg:: get_payload(int which,char* msg_data)
{
	if(which == LOG_DATA)
	{
		sprintf(msg_data, "<%d>", error_code);	
	}
	else
	{
		memset(msg_data, 0, header.dataLen);	
		msg_data[0] = error_code;
	}
	
}

void NotifyMsg:: send_file_payload(int sockfd, char* homeDir) {}
