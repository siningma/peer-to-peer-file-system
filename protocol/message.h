#ifndef MESSAGE_H
#define MESSAGE_H

#define JNRQ 0xFC
#define JNRS 0xFB
#define HLLO 0xFA
#define KPAV 0xF8
#define NTFY 0xF7
#define CKRQ 0xF6
#define CKRS 0xF5
#define SHRQ 0xEC
#define SHRS 0xEB
#define GTRQ 0xDC
#define GTRS 0xDB
#define STOR 0xCC
#define DELT 0xBC
#define STRQ 0xAC
#define STRS 0xAB

#define HEADER_LEN 27
#define UOID_LEN 20

#include "../system/tool.h"

typedef struct Msg_header
{
	unsigned char msgType;
	char UOID[20];
	unsigned char TTL;
	unsigned char reserved;
	uint32_t dataLen;
} MsgHeader;

class Msg
{
	public:
		MsgHeader header;
		
		Msg();
		virtual ~Msg();		
		//function implemented by parent class
		void create_header(MsgHeader msg_Header);
		void create_header(unsigned char msgType, char* UOID, unsigned char TTL);
		void get_header(char* msg_buf);
		void send_header(int sockfd);
		void send_payload(int sockfd, char* msg_data_buf);
		
		//subclass override following pure virtual function 
		virtual void parse_payload(char* msg_data_buf) = 0;			
		virtual void get_payload(int which, char* msg_data) = 0;
		virtual void send_file_payload(int sockfd, char* homeDir) = 0;
};

class KeepAliveMsg : public Msg 
{
	public: 
		void parse_payload(char* msg_data_buf) {};
		void get_payload(int which,char* msg_data) {};
		void send_file_payload(int sockfd, char* homeDir) {};
};

extern void get_header_from_msg(char* header_buf, MsgHeader& header);
extern void print_header(MsgHeader header);

class Neighbor_node
{
	public:
		char* hostname;
		uint16_t neighbor_port;
		
		Neighbor_node(char* hostname, uint16_t neighbor_port)
		{
			this->hostname = new char[strlen(hostname)];
			memset(this->hostname, 0, strlen(hostname));
			strncpy(this->hostname, hostname, strlen(hostname));
			this->neighbor_port = neighbor_port;
		}
		
		~Neighbor_node()
		{
			//delete[] hostname;
		}
		
		bool equals(char* hostname, uint16_t neighbor_port)
		{
			return (this->neighbor_port == neighbor_port) && !memcmp(this->hostname, hostname, strlen(this->hostname));
		}
		
		bool equals(Neighbor_node *neighbor)
		{
			return (this->neighbor_port == neighbor->neighbor_port) && !memcmp(this->hostname, neighbor->hostname, strlen(this->hostname));
		}
};

extern void send_file_data(int sockfd, char* filename, uint32_t fileSize);


#endif
