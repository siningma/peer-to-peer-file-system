#include "joinMsg.h"

using namespace std;

JoinMsg::JoinMsg(): location(0), port(0) {}

JoinMsg::~JoinMsg()
{
	delete hostname;
}

void JoinMsg:: parse_payload(char* msg_data_buf)
{
	uint32_t msg_location = 0;
	uint16_t msg_port = 0;
	memcpy(&msg_location, msg_data_buf, 4);
	location = ntohl(msg_location);
	memcpy(&msg_port, msg_data_buf + 4, 2);
	port = ntohs(msg_port);
	int len = header.dataLen - 6;
	hostname = new char[len + 1];
	if(hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");	
		exit(1);
	}
	memset(hostname, 0, len + 1);
	memcpy(hostname, msg_data_buf + 6, len);
}

void JoinMsg:: create_payload_data(uint32_t location, char* host_port, char* hostname)
{
	this->location = location;
	uint16_t port_number = (uint16_t)atoi(host_port);
	memcpy(&port, &port_number, 2);
	int len = strlen(hostname);
	this->hostname = new char[len + 1];
	if(this->hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");
		exit(1);
	}
	memset(this->hostname, 0, len + 1);	
	memcpy(this->hostname, hostname, len);
	header.dataLen = 6 + len;
}

void JoinMsg:: get_payload(int which,char* msg_data)
{	
	if(which == LOG_DATA)
	{
		memset(msg_data, 0, header.dataLen + 20);
		sprintf(msg_data, "<%d> <%s>", port, hostname);
	}	
	else
	{
		memset(msg_data, 0, header.dataLen);
		memcpy(msg_data, &htonl(location), 4);
		memcpy(&msg_data[4], &htons(port), 2);
		memcpy(&msg_data[6], hostname, header.dataLen - 6);
	}	

}

void JoinMsg:: send_file_payload(int sockfd, char* homeDir) {}
