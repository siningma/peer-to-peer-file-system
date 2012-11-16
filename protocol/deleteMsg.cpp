#include "deleteMsg.h"
using namespace std;

DeleteMsg:: DeleteMsg()
{
	memset(deleteFilename,0,256);
	memset(deleteSHA1,0,SHA_DIGEST_LENGTH);
	memset(deleteNonce,0,UOID_LEN);
	memset(deletePass, 0, UOID_LEN);
}

void DeleteMsg:: create_payload_data(char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce, unsigned char* deletePass)
{
	uint32_t len = strlen(deleteFilename);
	memcpy(this->deleteFilename, deleteFilename, len);
	memcpy(this->deleteSHA1, deleteSHA1, SHA_DIGEST_LENGTH);
	memcpy(this->deleteNonce, deleteNonce, UOID_LEN);
	memcpy(this->deletePass, deletePass, UOID_LEN);
	header.dataLen = len + 60;
}	

void DeleteMsg:: parse_payload(char* msg_data_buf)
{
	uint32_t start = header.dataLen - 60;
	memcpy(deleteFilename, msg_data_buf, start);
	memcpy(deleteSHA1, msg_data_buf + start, SHA_DIGEST_LENGTH);
	memcpy(deleteNonce, msg_data_buf + start + 20, UOID_LEN);
	memcpy(deletePass, msg_data_buf + start + 40, UOID_LEN);	
}

void DeleteMsg:: get_payload(int which, char* msg_data)
{
	uint32_t start = header.dataLen - 60;
	if(which == LOG_DATA)
	{}
	else
	{
		memcpy(msg_data, deleteFilename, start);
		memcpy(msg_data + start, deleteSHA1, SHA_DIGEST_LENGTH);
		memcpy(msg_data + start + 20, deleteNonce, UOID_LEN);
		memcpy(msg_data + start + 40, deletePass, UOID_LEN);
	}
	
}

void DeleteMsg:: send_file_payload(int sockfd, char* homeDir) {}
