#include "joinResMsg.h"
		
using namespace std;
		
JoinResMsg::JoinResMsg(): distance(0), host_port(0)
{
	memset(UOID, 0, 20);
}

JoinResMsg::~JoinResMsg()
{
	delete hostname;
}

void JoinResMsg:: parse_payload(char* msg_data_buf)
{
	uint32_t msg_distance = 0;
	uint16_t msg_port = 0;
	memcpy(UOID, msg_data_buf, 20);
	memcpy(&msg_distance, msg_data_buf + 20, 4);
	distance = ntohl(msg_distance);
	memcpy(&msg_port, msg_data_buf + 24, 2);
	host_port = ntohs(msg_port);
	int len = header.dataLen - 26;
	hostname = new char[len + 1];
	if(hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");	
		exit(1);
	}
	memset(hostname, 0, len + 1);
	memcpy(hostname, msg_data_buf + 26, len);
}

void JoinResMsg:: create_payload_data(char* UOID, uint32_t distance, uint16_t host_port, char* hostname)
{
	memcpy(this->UOID, UOID, UOID_LEN);
	this->distance = distance;
	this->host_port = host_port;
	int len = strlen(hostname);
	this->hostname = new char[len + 1];
	if(this->hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");	
		exit(1);
	}
	memset(this->hostname, 0, len + 1);		
	memcpy(this->hostname, hostname, len);
	header.dataLen = 26 + len;
}

void JoinResMsg:: get_payload(int which,char* msg_data)
{
	if(which == LOG_DATA)
	{	
		memset(msg_data, 0, 40);
		msg_data[0] = '<';
		memcpy(msg_data + 1, &UOID[16], 4);
		sprintf(msg_data + 5, "> <%d> <%d> <%s>", distance, host_port, hostname);
	}	
	else
	{
		memset(msg_data, 0, header.dataLen);
		memcpy(msg_data, UOID, 20);
		memcpy(msg_data + 20, &htonl(distance), 4);
		memcpy(msg_data + 24, &htons(host_port), 2);
		memcpy(msg_data + 26, hostname, header.dataLen - 26);
	}	

}

void JoinResMsg:: send_file_payload(int sockfd, char* homeDir) {}
