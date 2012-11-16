#include "message.h"

using namespace std;

Msg:: Msg()
{
	memset(&header, 0, sizeof(MsgHeader));
}

Msg:: ~Msg() {}

void Msg:: create_header(MsgHeader msg_Header)
{
	header.msgType = msg_Header.msgType;
	memcpy(header.UOID, msg_Header.UOID, UOID_LEN);
	header.TTL = msg_Header.TTL;
	header.dataLen = msg_Header.dataLen;
}

void Msg:: create_header(unsigned char msgType, char* UOID, unsigned char TTL)
{
	header.msgType = msgType;
	memcpy(header.UOID, UOID, UOID_LEN);
	header.TTL = TTL;
	header.dataLen = 0;
}	

void Msg:: get_header(char* msg_buf)
{
	memset(msg_buf, 0, HEADER_LEN);	
	//construct message header
	msg_buf[0] = header.msgType;
	memcpy(&msg_buf[1], header.UOID, UOID_LEN);
	msg_buf[21] = header.TTL;
	msg_buf[22] = header.reserved;
	memcpy(&msg_buf[23], &htonl(header.dataLen), 4);
	
}

void Msg::send_header(int sockfd)
{
	
	char header_buf[HEADER_LEN];
	get_header(header_buf);
	
	//send the message header
	if(send(sockfd, header_buf, HEADER_LEN, 0) == -1)
	{
		perror("send");
		close(sockfd);
		exit(1);
	}	
	//delete[] header_buf;
}

void Msg::send_payload(int sockfd, char* msg_data_buf)
{	
	//create message data field	
	int normal_time = header.dataLen / MAXDATASIZE;
	int last_data_size = header.dataLen % MAXDATASIZE;
	
	for(int i = 0; i < normal_time; i++)
	{
		if(send(sockfd, &msg_data_buf[i * MAXDATASIZE], MAXDATASIZE, 0) == -1)
		{
			perror("send");
			close(sockfd);
			exit(1);
		}
	}
	
	if(send(sockfd, &msg_data_buf[normal_time * MAXDATASIZE], last_data_size, 0) == -1)
	{
		perror("send");
		close(sockfd);
		exit(1);
	}
}

void get_header_from_msg(char* header_buf, MsgHeader& header)
{
	uint32_t msg_dataLen = 0;
	
	header.msgType = header_buf[0];
	memcpy(header.UOID, &header_buf[1], UOID_LEN);
	header.TTL = header_buf[21] - 1;	//decrease TTL by 1 for receiving thread
	header.reserved = header_buf[22];
	memcpy(&msg_dataLen, &header_buf[23], 4);
	header.dataLen = ntohl(msg_dataLen);
}

void print_header(MsgHeader header)
{
	printf("msg header: \n");
	printf("msgType: 0x%02x\n", header.msgType);
	printf("UOID: ");
	for(int i = 0; i < UOID_LEN; i++)
		printf("%02x", (unsigned char)header.UOID[i]);
	printf("\n");	
	printf("TTL: %d\n", header.TTL);
	printf("dataLen: %u\n", header.dataLen);
}

void send_file_data(int sockfd, char* filename, uint32_t fileSize)
{
	char data_buf[MAX_BUF_SIZE];
	memset(data_buf, 0, MAX_BUF_SIZE);
	
	//create message data field	
	FILE* pFile = fopen(filename, "r");
	uint32_t normal_time = fileSize / MAX_BUF_SIZE;
	uint32_t last_time = fileSize % MAX_BUF_SIZE;
	fseek(pFile, 0, SEEK_SET);
	
	for(uint32_t i = 0; i < normal_time; i++)
	{
		memset(data_buf, 0, MAX_BUF_SIZE);
		fread(data_buf, 1, MAX_BUF_SIZE, pFile);
		if(send(sockfd, data_buf, MAX_BUF_SIZE, 0) == -1)
		{
			printf("Server: client close socket to server\n");
			exit(1);
		}
	}
		
	//the last time to send message
	memset(data_buf, 0, MAX_BUF_SIZE);
	fread(data_buf, 1, last_time, pFile);
	if(send(sockfd, data_buf, last_time, 0) == -1)
	{
		printf("Server: client close socket to server\n");
		exit(1);
	}
	fclose(pFile);
}
