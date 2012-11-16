#include "getMsg.h"

using namespace std;

GetMsg:: GetMsg()
{
	memset(FileID,0,UOID_LEN);
	memset(msg_SHA1,0,SHA_DIGEST_LENGTH);
}

void GetMsg:: create_payload_data(unsigned char* FileID, unsigned char* msg_SHA1)
{
	memcpy(this->FileID, FileID, UOID_LEN);
	memcpy(this->msg_SHA1, msg_SHA1, SHA_DIGEST_LENGTH);
	header.dataLen = 40;
}

void GetMsg:: parse_payload(char* msg_data_buf)
{
	memcpy(FileID, msg_data_buf, UOID_LEN);
	memcpy(msg_SHA1, msg_data_buf + 20, SHA_DIGEST_LENGTH);
}

void GetMsg:: get_payload(int which,char* msg_data)
{
	if(which == LOG_DATA)
	{	
		memset(msg_data, 0, 6);
		msg_data[0] = '<';
		memcpy(msg_data + 1, &FileID[16], 4);
		msg_data[5] = '>';	
	}	
	else
	{
		memset(msg_data, 0, header.dataLen);	
		memcpy(msg_data, FileID, UOID_LEN);
		memcpy(msg_data + 20, msg_SHA1, SHA_DIGEST_LENGTH);
	}	
}

void GetMsg:: send_file_payload(int sockfd, char* homeDir) {}
