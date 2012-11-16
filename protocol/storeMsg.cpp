#include "storeMsg.h"

StoreMsg:: StoreMsg(): file_index(0), meta_fileSize(0) {}

void StoreMsg:: create_payload_data(uint32_t inputIndex, char* homeDir)
{
	file_index = inputIndex;
	
	char metaDataFileName[256];
	memset(metaDataFileName , 0, 256);
	create_metaDataFilename(metaDataFileName, homeDir, file_index);
	meta_fileSize = getFilesize(metaDataFileName);

	char dataFilename[256];
	memset(dataFilename, 0, 256);
	create_dataFilename(dataFilename, homeDir, file_index);
	
	uint32_t data_fileSize = getFilesize(dataFilename);
	header.dataLen = 4 + meta_fileSize + data_fileSize;
}

void StoreMsg:: parse_payload(char* msg_data_buf) {}

void StoreMsg:: get_payload(int which,char* msg_data) 
{
	if(which == LOG_DATA)
	{}
	else
	{
		fprintf(stderr, "store msg cannot use to receive payload\n");
		assert(false);
	}
}

void StoreMsg:: send_file_payload(int sockfd, char* homeDir)
{		
	char msg_metadatafileSize[4];
	memset(msg_metadatafileSize, 0, 4);
	memcpy(msg_metadatafileSize, &htonl(meta_fileSize), 4);
	if(send(sockfd, msg_metadatafileSize, 4, 0) == -1)
	{
		printf("Server: client close socket to server\n");
		exit(1);
	}
	
	char metaDataFilename[256];
	memset(metaDataFilename, 0, 256);
	create_metaDataFilename(metaDataFilename, homeDir, file_index);
	//send meta data file
	send_file_data(sockfd, metaDataFilename, meta_fileSize);

	char dataFilename[256];
	memset(dataFilename, 0, 256);
	create_dataFilename(dataFilename, homeDir, file_index);	
	uint32_t data_fileSize = header.dataLen - 4 - meta_fileSize;
	
	//send file data
	send_file_data(sockfd, dataFilename, data_fileSize);
}
