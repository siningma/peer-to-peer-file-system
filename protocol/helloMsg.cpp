#include "helloMsg.h"

using namespace std;

HelloMsg:: HelloMsg():port(0) {}

HelloMsg:: ~HelloMsg()
{
	delete hostname;
}

void HelloMsg:: parse_payload(char* msg_data_buf)
{
	uint16_t msg_port = 0;
	memcpy(&msg_port, msg_data_buf, 2);
	port = ntohs(msg_port);
	int len = header.dataLen - 2;
	hostname = new char[len + 1];
	if(hostname == NULL)
	{
		fprintf(stderr, "new hostname failed\n");
		exit(1);
	}	
	memset(hostname, 0, len + 1);
	memcpy(hostname, msg_data_buf + 2, len);
}

void HelloMsg:: create_payload_data(char* host_port, char* hostname)
{
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
	header.dataLen = 2 + len;
}

void HelloMsg:: get_payload(int which,char* msg_data)
{	
	if(which == LOG_DATA)
	{	
		memset(msg_data, 0, header.dataLen + 20);
		sprintf(msg_data, "<%d> <%s>", port, hostname);
	}	
	else
	{
		memset(msg_data, 0, header.dataLen);
		memcpy(msg_data, &htons(port), 2);
		memcpy(msg_data + 2, hostname, header.dataLen - 2);
	}	

}

void HelloMsg:: send_file_payload(int sockfd, char* homeDir) {}
