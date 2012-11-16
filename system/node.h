#ifndef NODE_H
#define NODE_H

#include "tool.h"
#include "log.h"
#include "helper.h"
#include "../filesys/fileSystem.h"

#define NON_BEACON_NODE 0
#define BEACON_NODE 1
#define BACKLOG 10

#define TEMPORARY_CONNECTION 0
#define REAL_CONNECTION 1

using namespace std;

extern bool gnShutdown;
extern pthread_mutex_t shut_down_lock;

class Node
{
	public :
		char* node_id;
		char* node_instance_id;
		char logFileName[11];
		
		//all the data from configuration file
		char* hostname;
		char* port;
		uint32_t location;
		char* homeDir;
		int autoShutDown;
		int TTL;
		int msgLifeTime;
		int getMsgLifeTime;
		int initNeighbors;
		int joinTimeout;
		int keepAliveTimeout;
		int minNeighbors;
		int noCheck;
		double cacheProb;
		double storeProb;
		double neighborStoreProb;
		uint32_t cacheSize;
		int retryTime;
		vector<char*> beaconNodes_hostname;
		vector<char*> beaconNodes_port;
		int beaconNodes_count;
		int node_type;
		//non beacon node neighbors read from init_neighbor_list
		vector<char*> non_beacon_neighbors;
		//signal mask
		pthread_t signal_thread_id;
		sigset_t mask;
		
		//log file
		Log* log;	
		
		//-----------------------------------------------------------------					
		//UOID time map to delete message after timeout, UOID=>time 
		std::map<UOID_Map, int> UOID_time_map;
		pthread_mutex_t UOID_time_map_lock;		
		//neighbor node map, socket=>Neighbor_node
		std::map<int, Neighbor_node*> nb_map;
		pthread_mutex_t nb_map_lock;		
		//routing table used for forward messages, UOID=>socket
		std::map<UOID_Map, int> routing_map;
		pthread_mutex_t routing_map_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//status file
		char* statusFilename;
		FILE* statusFile;
		bool issent_status;
		int status_sent_time;
	 	std::vector<uint16_t> statusPorts;	//this port has been seen before or not
		pthread_mutex_t status_file_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//check control data and lock
		bool issent_check;
		char check_msg_UOID[UOID_LEN];
		int check_sent_time;
		bool checkRes_recv;
		pthread_mutex_t check_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------	
		//the queue to store all messages that will be dispatched
		std::queue<DispatchElement*> dispatch_queue;
		bool dispatch_exit;
		pthread_mutex_t dispatch_queue_lock;
		pthread_cond_t dispatch_queue_cv;	
		//node table to map thread_id to SendThread_Resource, sockfd=>SendThread_Resource
		std::map<int, SendThread_Resourse*> id_map;
		pthread_mutex_t id_map_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//join network success or connect other network success
		bool join_success;
		bool join_shutdown;
		int connect_fail_count;
		vector<int> join_sockets;
		pthread_mutex_t join_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//set up search command variables
		bool search_exit;
		std::queue<SearchRes_Elm> searchResponse_queue;
		pthread_cond_t searchResponse_queue_cv;	
		std::map<uint32_t, UOID_Map> searchFileID_map; //searchIndex => FileID in the node doing search
		std::map<uint32_t, UOID_Map> searchFileSHA1_map;	//searchIndex => SHA1 in the node doing search
		std::map<uint32_t, string> searchFileName_map;	//searchIndex => Filename in the node doing  search
		std::map<UOID_Map, int> search_time_map;
		std::map<UOID_Map, bool> search_done_map;
		pthread_mutex_t search_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//store fileID, filesys index info when creating search response message
		std::map<UOID_Map, uint32_t> fileIDIndex_map;	//FileID => index	
		pthread_mutex_t fileID_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//save get msgID, ext filename info when create get message
		std::map<UOID_Map, string> getMsgFilename_map;	//msgID => extfile
		std::map<UOID_Map, bool> getReplace_map;	//msgID => isReplace
		pthread_mutex_t get_lock;
		//-----------------------------------------------------------------
		
		//-----------------------------------------------------------------
		//global variables for all threads info
		bool userShutdown;
		bool recv_thread_exit; 
		bool all_thread_exit;
		pthread_mutex_t control_thread_lock;
		//-----------------------------------------------------------------
		
		Node(char* filename, bool reset);
		~Node();
		void parse_file_data(vector<char*> file_data, bool reset);
		void run();
		bool establish_temporary_connection();
		void beacon_start();
		void non_beacon_start();
		
		//socket prepare and establish real connection
		void server_socket_prepare();
		int client_socket_prepare(char* server_hostname, char* server_port, bool& connected);
		void join_network(int sockfd);
		void add_to_join_socket(int sockfd);
				
		//fork thread helper
		static void* server_thread_helper(void* arg);
		static void* client_thread_helper(void* arg);
		static void* keyboard_thread_helper(void* arg);
		static void* child_send_thread_helper(void* arg);
		static void* child_recv_thread_helper(void* arg);
		static void* dispatch_thread_helper(void* arg);
		static void* timer_thread_helper(void* arg);
		static void* signal_thread_helper(void* arg);
		static void* join_thread_helper(void* arg);
		static void* join_recv_thread_helper(void* arg);
		
		//actual thread running function	
		void join_thread(int index);
		void client_thread(int index);
		void fork_send_recv_thread(int sockfd, pthread_t& child_send_thread_id, pthread_t& child_recv_thread_id);
		void join_send_recv_thread(int sockfd, pthread_t& child_send_thread_id, pthread_t& join_recv_thread_id);
		void child_send_thread(int sockfd);
		void child_recv_thread(int sockfd, pthread_t send_thread_id);
		void dispatch_thread();
		void keyboard_thread();
		void timer_thread();
		void signal_thread();	
		void join_recv_thread(int sockfd, pthread_t send_thread_id);
		//close recv thread corresponding send thread
		void send_thread_close(int sockfd);
		
		//create specific message
		CheckMsg* create_checkMsg();
		CheckResMsg* create_checkResMsg(char* check_UOID);
		HelloMsg* create_helloMsg();
		JoinMsg* create_joinMsg();
		JoinResMsg* create_joinResMsg(char* join_UOID, uint32_t distance);
		NotifyMsg* create_notifyMsg(unsigned char error_code);
		KeepAliveMsg* create_keepAliveMsg();
		StatusMsg* create_statusMsg(unsigned char status_TTL, unsigned char report_type);
		StatusResMsg* create_statusResMsg(char* status_UOID);
		StatusResMsg* create_statusResMsg_File(char* status_UOID);
				
		//handle message based on msgType
		bool handle_recv_helloMsg(MsgHeader header, char* msg_data, int sockfd);
		void handle_recv_notifyMsg(MsgHeader header, char* msg_data, int sockfd);
		Msg* handle_recv_msg(MsgHeader header, char* msg_data, int sockfd);	
		//handle specific message
		void handle_joinMsg(JoinMsg* joinMsg, int sockfd);
		void handle_joinResMsg(JoinResMsg* joinResMsg, int sockfd);
		bool handle_helloMsg(HelloMsg* helloMsg, int sockfd);
		void handle_keepAliveMsg(KeepAliveMsg *keepAliveMsg, int sockfd);
		void handle_notifyMsg(NotifyMsg* notifyMsg, int sockfd);
		void handle_checkMsg(CheckMsg* checkMsg, int sockfd);
		void handle_checkResMsg(CheckResMsg* checkResMsg, int sockfd);
		void handle_statusMsg(StatusMsg *statusMsg, int sockfd);
		void handle_statusResMsg(StatusResMsg *statusResMsg, int sockfd);
		bool handle_recv_statusResFileMsg(int sockfd, MsgHeader header);
		
		//work on related message
		void push_dispatch_queue(DispatchElement *dispatchElm);
		void add_to_neighbor_table(int sockfd, char* init_hostname, char* init_port);
		string get_neighbor(int sockfd);
		//send messages
		void flood_msg(Msg* msg, int sockfd);
		void send_all_neighbors_msg(Msg* msg);
		void send_other_neighbors_msg(Msg* msg, int sockfd);
		void send_hello_message(int sockfd);
		void send_check_message(int sockfd);
		void send_notify_message(unsigned char error_code);
		
		void write_to_statusFile(uint16_t local_port, vector<uint16_t> neighbor_ports);
		void write_to_statusFile_node(uint16_t node_port);
		void write_to_statusFile_link(uint16_t node_first, uint16_t node_second);
		int write_to_statusFile(vector<uint32_t> filesys_files);
		int write_to_statusFile(StatusResMsg *statusResMsg);
		
		//cleanUp functions when node shutdown
		void exit_threads();
		void cleanup();
		
		//--------------------------------------------------------------------------------
		//part 2 functions
		void create_storeMsg(int storeFile_index, unsigned char store_TTL);
		void create_storeMsg(int sockfd, uint32_t storeFile_index, unsigned char store_TTL, char* msg_UOID);
		bool handle_recv_storeMsg(int sockfd, MsgHeader header);
		
		void create_searchMsg(unsigned char searchType, string searchQuery, char* searchMsg_UOID);
		void handle_searchMsg(SearchMsg *searchMsg, int sockfd);
		SearchResMsg* create_searchResMsg(char* search_UOID, vector<uint32_t> matchFiles);
		bool handle_recv_searchResMsg(int sockfd, MsgHeader header);
		
		void create_getMsg(unsigned char* fileID, unsigned char* fileSHA1, string getFilename, bool isReplace);
		void handle_getMsg(GetMsg *getMsg, int sockfd);
		GetResMsg* create_getResMsg(char* get_UOID, uint32_t fileIndex);
		bool handle_recv_getResMsg(int sockfd, MsgHeader header);
		
		vector<uint32_t> get_matchFileIndex(const char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce);
		void create_deleteMsg(char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce, unsigned char* deletePass);
		void handle_deleteMsg(DeleteMsg* deleteMsg, int sockfd);
		
		void send_prob_all_neighbors_msg(Msg* msg);
		void forward_prob_other_neighbors_msg(Msg* msg, int sockfd);
		
		int recv_givenLenData(int sockfd, void* data_buf, uint32_t protocol_len);
		int recv_metaDataFile(int sockfd, char* filename, uint32_t fileSize, bool recv_before);
		int recv_fileData(int sockfd, char* filename, uint32_t fileSize, bool recv_before, const unsigned char* metaDataSHA1);
		void show_searchResponse(unsigned char searchType, string searchQuery, char* searchMsg_UOID);
};

struct NodeIndex
{
	Node* node;
	int index;
};

struct SendThread_arg
{
	Node* node;
	int sockfd;
};

struct RecvThread_arg
{
	Node* node;
	int sockfd;
	pthread_t send_thread_id;
};

#endif
