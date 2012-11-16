#include "searchResMsg.h"

using namespace std;

SearchResMsg:: SearchResMsg()
{
	memset(UOID, 0, 20);
	fileState = false;
}

void SearchResMsg:: create_payload_data(char* UOID)
{
	memcpy(this->UOID, UOID, 20);	
	header.dataLen = 20;
}
		
void SearchResMsg:: parse_payload(char* msg_data_buf) {}	
		
void SearchResMsg:: get_payload(int which,char* msg_data)
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
		fprintf(stderr, "searchResMsg cannot use to receive payload\n");
		assert(false);
	}
}
	
void SearchResMsg:: send_file_payload(int sockfd, char* homeDir)
{
	char search_UOID[20];
	memset(search_UOID, 0, 20);
	memcpy(search_UOID, UOID, 20);
	if(send(sockfd, search_UOID, 20, 0) == -1)
	{
		printf("Server: client close socket to server\n");
		exit(1);
	}
	
	for(size_t i = 0; i < matchFile_size.size(); i++)
	{
		uint32_t fileSize = matchFile_size[i];
		UOID_Map uoid_fileID = matchFileID[i];
		uint32_t file_index = matchFile_index[i];
		char msg_data[24];
		memset(msg_data, 0, 24);
		memcpy(msg_data, &htonl(fileSize), 4);
		memcpy(msg_data + 4, uoid_fileID.m_UOID, 20);
		if(send(sockfd, msg_data, 24, 0) == -1)
		{
			printf("Server: client close socket to server\n");
			exit(1);
		}
		
		if(fileState)	//fileState = true means this file is in tempMeta directory
		{
			char tempMetaDataFileName[256];
			memset(tempMetaDataFileName, 0, 256);
			create_tempMetaDataFilename(tempMetaDataFileName, homeDir, file_index);
			send_file_data(sockfd, tempMetaDataFileName, fileSize);
			
			remove(tempMetaDataFileName);
		}
		else
		{
			char metaDataFileName[256];
			memset(metaDataFileName , 0, 256);
			create_metaDataFilename(metaDataFileName, homeDir, file_index);
			send_file_data(sockfd, metaDataFileName, fileSize);
		}
	}
}
