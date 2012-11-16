#include "statusMsg.h"

StatusMsg:: StatusMsg():report_type(0) {}

void StatusMsg:: create_payload_data(unsigned char report_type)
{
	this->report_type = report_type;
	header.dataLen = 1;
}
	
void StatusMsg:: parse_payload(char* msg_data_buf)
{
	report_type = msg_data_buf[0];
}		

void StatusMsg:: get_payload(int which,char* msg_data)
{
	if(which == LOG_DATA)
	{
		switch(report_type)
		{
			case 1:
			strcpy(msg_data, "<neighbors>");
			break;
			case 2:
			strcpy(msg_data, "<files>");
			break;
			default:
			break;	
		}
	}
	else
	{
		memset(msg_data, 0, 1);	
		msg_data[0] = report_type;
	}
	
}

void StatusMsg:: send_file_payload(int sockfd, char* homeDir) {}
