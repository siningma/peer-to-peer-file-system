#include "getResMsg.h"

GetResMsg:: GetResMsg(): fileIndex(0), get_metadatafileSize(0) 
{
	memset(UOID, 0, 20);
	fileState = false;
}

void GetResMsg:: create_payload_data(char* UOID, uint32_t fileIndex, uint32_t metaData_fileSize, char* homeDir)
{
	memcpy(this->UOID, UOID, 20);
	this->fileIndex = fileIndex;
	
	this->get_metadatafileSize = metaData_fileSize;

	char dataFilename[256];
	memset(dataFilename, 0, 256);
	create_dataFilename(dataFilename, homeDir, fileIndex);
	uint32_t data_fileSize = getFilesize(dataFilename);
	
	header.dataLen = 4 + this->get_metadatafileSize + data_fileSize;
}

void GetResMsg:: parse_payload(char* msg_data_buf) {}		
	
void GetResMsg:: get_payload(int which, char* msg_data)
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
		fprintf(stderr, "get response msg cannot use to receive payload\n");
		assert(false);
	}
}

void GetResMsg:: send_file_payload(int sockfd, char* homeDir) 
{
	char msg_data[24];
	memset(msg_data, 0, 24);
	memcpy(msg_data, UOID, 20);
	memcpy(msg_data + 20, &htonl(get_metadatafileSize), 4);
	if(send(sockfd, msg_data, 24, 0) == -1)
	{
		printf("Server: client close socket to server\n");
		exit(1);
	}
	
	char metaDataFilename[256];
	memset(metaDataFilename, 0, 256);
	create_metaDataFilename(metaDataFilename, homeDir, fileIndex);
	//send meta data file
	send_file_data(sockfd, metaDataFilename, get_metadatafileSize);

	char dataFilename[256];
	memset(dataFilename, 0, 256);
	create_dataFilename(dataFilename, homeDir, fileIndex);	
	uint32_t data_fileSize = header.dataLen - 4 - get_metadatafileSize;
	
	//send file data
	send_file_data(sockfd, dataFilename, data_fileSize);
	
	if(fileState)	//fileSate = true means this file does not need to cache in the file system
	{
		remove(metaDataFilename);
		remove(dataFilename);
	}
}
