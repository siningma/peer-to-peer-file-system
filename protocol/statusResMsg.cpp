#include "statusResMsg.h"

StatusResMsg :: StatusResMsg():hostInfoLen(0), hostPort(0)
{
	memset(UOID, 0, UOID_LEN);
	fileState = false;
}

StatusResMsg :: ~StatusResMsg()
{
	delete hostname;
	recordLengths.clear();
	neighbors.clear();
	fileIndexes.clear();
}

void StatusResMsg :: create_payload_data(char* UOID, uint16_t hostInfoLen, uint16_t hostPort, char* hostname)
{
	memcpy(this->UOID, UOID, UOID_LEN);
	this->hostInfoLen = hostInfoLen;
	this->hostPort = hostPort;
	uint32_t len = strlen(hostname);
	this->hostname = new char[len + 1];
	if(this->hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");
		exit(1);
	}
	memset(this->hostname, 0, len + 1);
	memcpy(this->hostname, hostname, len);
	header.dataLen = 24 + len;
}

void StatusResMsg :: create_payload_neighbor_data(Neighbor_node* neighbor_node)
{
	uint32_t recordLen = strlen(neighbor_node->hostname) + 2;
	header.dataLen += 4;
	header.dataLen += recordLen;
	recordLengths.push_back(recordLen);
	neighbors.push_back(neighbor_node);
}

void StatusResMsg:: create_payload_file_data(char* homeDir, uint32_t file_index)
{
	char metaDataFileName[256];
	memset(metaDataFileName , 0, 256);
	create_metaDataFilename(metaDataFileName, homeDir, file_index);
	uint32_t recordLen = getFilesize(metaDataFileName);
	header.dataLen += 4;
	header.dataLen += recordLen;	
	recordLengths.push_back(recordLen);
	fileIndexes.push_back(file_index);
}

void StatusResMsg :: parse_payload(char* msg_data_buf)			
{
	uint16_t msg_hostInfoLen = 0;
	uint16_t msg_port = 0;
	memcpy(UOID, msg_data_buf, 20);
	memcpy(&msg_hostInfoLen, msg_data_buf + 20, 2);
	memcpy(&msg_port, msg_data_buf + 22, 2);
	hostInfoLen = ntohs(msg_hostInfoLen);
	hostPort = ntohs(msg_port);
	uint16_t len = hostInfoLen - 2;
	hostname = new char[len + 1];
	if(hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");	
		exit(1);
	}
	memset(hostname, 0, len + 1);
	memcpy(hostname, msg_data_buf + 24, len);
	uint32_t temp_len = 24 + len;
	uint32_t start = 0;
	while((temp_len + start) != header.dataLen)
	{
		uint32_t msg_recordLen = 0;
		uint16_t msg_neighbor_port = 0;
		memcpy(&msg_recordLen, msg_data_buf + temp_len + start, 4);
		uint32_t recordLen = ntohl(msg_recordLen);
		start += 4;
		recordLengths.push_back(recordLen);		
		memcpy(&msg_neighbor_port, msg_data_buf + temp_len + start, 2);
		uint16_t neighbor_port = ntohs(msg_neighbor_port);
		start += 2;
		uint32_t hostLen = recordLen - 2;
		char neighbor_hostname[hostLen];
		memset(neighbor_hostname, 0, hostLen);
		memcpy(&neighbor_hostname, msg_data_buf + temp_len + start, hostLen);
		start += hostLen;
		Neighbor_node *neighbor_node = new Neighbor_node(neighbor_hostname, neighbor_port); 
		neighbors.push_back(neighbor_node); 
	}
}

void StatusResMsg :: get_payload(int which,char* msg_data)
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
		memcpy(msg_data + 20, &htons(hostInfoLen), 2);
		memcpy(msg_data + 22, &htons(hostPort), 2);
		memcpy(msg_data + 24, hostname, hostInfoLen - 2);
		uint32_t temp_len = 22 + hostInfoLen;
		uint32_t start = 0;
		for(int i = 0; i < (int)recordLengths.size(); i++)
		{
			uint32_t recordLen = recordLengths[i];
			memcpy(msg_data + temp_len + start, &htonl(recordLen), 4);
			start += 4;
			Neighbor_node *neighbor_node = neighbors[i];
			memcpy(msg_data + temp_len + start, &htons(neighbor_node->neighbor_port), 2);
			start += 2;
			uint32_t hostLen = recordLen - 2;
			memcpy(msg_data + temp_len + start, neighbor_node->hostname, hostLen);
			start += hostLen;
		}
	}
			
}

void StatusResMsg:: send_file_payload(int sockfd, char* homeDir)
{
	char msg_data[22 + hostInfoLen];
	memset(msg_data, 0, 22 + hostInfoLen);	
	memcpy(msg_data, UOID, 20);
	memcpy(msg_data + 20, &htons(hostInfoLen), 2);
	memcpy(msg_data + 22, &htons(hostPort), 2);
	memcpy(msg_data + 24, hostname, hostInfoLen - 2);
	if(send(sockfd, msg_data, 22 + hostInfoLen, 0) == -1)
	{
		printf("Server: client close socket to server\n");
		exit(1);
	}
	
	for(size_t i = 0; i < recordLengths.size(); i++)
	{
		uint32_t recordLen = recordLengths[i];
		uint32_t file_index = fileIndexes[i];
		char msg_recordLen[4];
		memset(msg_recordLen, 0, 4);
		memcpy(msg_recordLen, &htonl(recordLen), 4);
		if(send(sockfd, msg_recordLen, 4, 0) == -1)
		{
			printf("Server: client close socket to server\n");
			exit(1);
		}
		
		if(fileState)	//fileState = true means this file is in tempMeta directory
		{
			char tempMetaDataFileName[256];
			memset(tempMetaDataFileName, 0, 256);
			create_tempMetaDataFilename(tempMetaDataFileName, homeDir, file_index);
			send_file_data(sockfd, tempMetaDataFileName, recordLen);
			
			remove(tempMetaDataFileName);
		}
		else
		{
			char metaDataFileName[256];
			memset(metaDataFileName , 0, 256);
			create_metaDataFilename(metaDataFileName, homeDir, file_index);
			send_file_data(sockfd, metaDataFileName, recordLen);
		}
	}
}
