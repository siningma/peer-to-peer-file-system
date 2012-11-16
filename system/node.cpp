#include "node.h"

using namespace std;

bool gnShutdown = true;
pthread_mutex_t shut_down_lock;

inline void sigchld_handler(int s) 
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

Node :: Node(char* filename, bool reset)
{
	autoShutDown = 900;
	TTL = 30;
	msgLifeTime = 30;
	getMsgLifeTime = 300;
	initNeighbors = 3;
	joinTimeout = 15;
	keepAliveTimeout = 60;
	minNeighbors = 2;
	noCheck = 0;
	cacheProb = 0.1;
	storeProb = 0.1;
	neighborStoreProb = 0.2;
	cacheSize = 500;
	retryTime = 30;
	node_type = NON_BEACON_NODE;
	memset(logFileName, 0, 11);
	strcpy(logFileName, "servant.log");
	
	beaconNodes_count = 0;
	dispatch_exit = false;
	userShutdown = false;
	recv_thread_exit = false;
	all_thread_exit = false;
	join_success = false;
	join_shutdown = false;
	connect_fail_count = 0;
	issent_status = false;
	status_sent_time = 0;
	issent_check = false;
	signal_thread_id = 0;
	
	memset(check_msg_UOID, 0, UOID_LEN);
	check_sent_time = 0;
	checkRes_recv = false;
	
	search_exit = true;
	
	//read data from configuration file and parse data
	vector<char*> file_data = read_file(filename);
	#ifdef _DEBUG_
		for(int i = 0; i < (int)file_data.size(); ++i)
			printf("%s\n", file_data[i]);
	#endif	
	parse_file_data(file_data, reset);
	
	//init file system
	if(reset)
		reset_filesys(homeDir);	
	init_filesys(homeDir);
	read_filesys(homeDir);
	
	//init lock and cv for node resource
	if(pthread_mutex_init(&UOID_time_map_lock, NULL) != 0)
	{
		printf("UOID time map lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&nb_map_lock, NULL) != 0)
	{
		printf("nb map lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&routing_map_lock, NULL) != 0)
	{
		printf("routing map lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&dispatch_queue_lock, NULL) != 0)
	{
		printf("dispatch queue lock init fail\n");
		exit(1);
	}
	if(pthread_cond_init(&dispatch_queue_cv, NULL) != 0)
	{
		printf("dispatch queue cv init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&id_map_lock, NULL) != 0)
	{
		printf("id map lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&join_lock, NULL) != 0)
	{
		printf("join lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&status_file_lock, NULL) != 0)
	{
		printf("status map lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&check_lock, NULL) != 0)
	{
		printf("check lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&search_lock, NULL) != 0)
	{
		printf("search lock init fail\n");
		exit(1);
	}
	if(pthread_cond_init(&searchResponse_queue_cv, NULL) != 0)
	{
		printf("search response queue cv init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&fileID_lock, NULL) != 0)
	{
		printf("fileID lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&get_lock, NULL) != 0)
	{
		printf("get lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&control_thread_lock, NULL) != 0)
	{
		printf("control thread lock init fail\n");
		exit(1);
	}
	alarm(autoShutDown);
	
	//add sigal handle mask
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGINT);
	int err = 0;
	if((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
	{
		printf("SIG_BLOCK error\n");
		exit(1);
	}
}

Node :: ~Node()
{
	delete[] homeDir;
	delete[] logFileName;
	delete log;
	delete[] node_id;
	delete[] port;
	delete[] hostname;
	// int size = 	(int)beaconNodes_hostname.size();
// 	for(int i = 0; i < size; ++i)
// 	{
// 		delete[] beaconNodes_hostname[i];
// 		beaconNodes_hostname[i] = NULL;
// 		delete[] beaconNodes_port[i];
// 		beaconNodes_port[i] = NULL;
// 	}
	beaconNodes_hostname.clear();
	beaconNodes_port.clear();
	
	pthread_mutex_destroy(&UOID_time_map_lock);
	pthread_mutex_destroy(&nb_map_lock);
	pthread_mutex_destroy(&routing_map_lock);
	pthread_mutex_destroy(&dispatch_queue_lock);
	pthread_mutex_destroy(&id_map_lock);
	pthread_mutex_destroy(&join_lock);
	pthread_mutex_destroy(&status_file_lock);
	pthread_mutex_destroy(&check_lock);
	pthread_mutex_destroy(&search_lock);
	pthread_mutex_destroy(&fileID_lock);
	pthread_mutex_destroy(&get_lock);
	pthread_mutex_destroy(&control_thread_lock);
	pthread_cond_destroy(&dispatch_queue_cv);
	pthread_cond_destroy(&searchResponse_queue_cv);
}

void Node :: parse_file_data(vector<char*> file_data, bool reset)
{
	bool beaconFile = false;
	char prefix[MAX_LINE];
	char data[MAX_LINE];
	
	#ifdef _DEBUG_
		for(vector<char*>::iterator iter = file_data.begin(); iter != file_data.end(); ++iter)
			printf("%s\n", *iter);
	#endif	
	
	for(vector<char*>::iterator iter = file_data.begin(); iter != file_data.end(); ++iter)
	{
		if(!strcmp(*iter, "[init]"))
		{
			beaconFile = false;
			continue;
		}	
		if(!strcmp(*iter, "[beacons]"))
		{
			beaconFile = true;
			continue;
		}	
		
		memset(prefix, 0, MAX_LINE);
		memset(data, 0, MAX_LINE);
		parseLine(*iter, prefix, data);
		if(!beaconFile)
		{
			if(!strcmp(prefix, "Port"))
			{
				port = new char[6];
				memset(port, 0, 6);
				strcpy(port, data);
			}
			else if(!strcmp(prefix, "Location"))
				location = (uint32_t)atoi(data);
			else if(!strcmp(prefix, "HomeDir"))
			{
				homeDir = new char[strlen(data)];
				memset(homeDir, 0, strlen(data));
				strcpy(homeDir, data);
			}
			else if(!strcmp(prefix, "AutoShutdown"))
				autoShutDown = atoi(data);
			else if(!strcmp(prefix, "TTL"))
				TTL = atoi(data);
			else if(!strcmp(prefix, "MsgLifetime"))
				msgLifeTime = atoi(data);	
			else if(!strcmp(prefix, "GetMsgLifetime"))
				getMsgLifeTime = atoi(data);
			else if(!strcmp(prefix, "InitNeighbors"))
				initNeighbors = atoi(data);
			else if(!strcmp(prefix, "JoinTimeout"))
				joinTimeout = atoi(data);
			else if(!strcmp(prefix, "KeepAliveTimeout"))
				keepAliveTimeout = atoi(data);		
			else if(!strcmp(prefix, "MinNeighbors"))
				minNeighbors = atoi(data);
			else if(!strcmp(prefix, "NoCheck"))
				noCheck = atoi(data);	
			else if(!strcmp(prefix, "CacheProb"))
				cacheProb = (double)atof(data);
			else if(!strcmp(prefix, "StoreProb"))
				storeProb = (double)atof(data);
			else if(!strcmp(prefix, "NeighborStoreProb"))
				neighborStoreProb = (double)atof(data);
			else if(!strcmp(prefix, "CacheSize"))
				cacheSize = (uint32_t)atoi(data);										
		}
		else
		{
			if(!strcmp(prefix, "Retry"))
				retryTime = atoi(data);
			else
			{
				char* beacon_hostname = new char[strlen(prefix)];
				memset(beacon_hostname, 0, strlen(prefix));
				char* beacon_port = new char[6];
				memset(beacon_port, 0, 6);
				//printf("prefix: %s\n", prefix);
				parseBeacon(prefix, beacon_hostname, beacon_port);
				if(!strcmp(beacon_port, port))
					node_type = BEACON_NODE;
				else
				{	
					//printf("hostname: %s\n", beacon_hostname);
					//printf("port: %s\n", beacon_port);
					beaconNodes_hostname.push_back(beacon_hostname);	
					beaconNodes_port.push_back(beacon_port);
					beaconNodes_count++;
				}	
			}	
		}		
	}
	
	if(reset)	//if reset is set, remove init files
	{
		int len = strlen(homeDir) + strlen("/init_neighbor_list");
		char path[len];
		memset(path, 0, len);
		strcpy(path, homeDir);
		strcat(path, "/init_neighbor_list");
		if(fopen(path, "r"))
			remove(path);
		memset(path, 0, len);	
		strcpy(path, homeDir);
		strcat(path, "/servant.log");			
		if(fopen(path, "r"))
			remove(path);		
	}
		
	mkdir(homeDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	log = new Log(homeDir);
	//clear file_data table
	//file_data.clear();
	
	#ifdef _DEBUG_
		printf("\n");
		printf("port: %s\n", port);
		printf("location: %u\n", location);
		printf("homeDir: %s\n", homeDir);
		printf("logFileName: %s\n", logFileName);
		printf("autoShutDown: %d\n", autoShutDown);
		printf("TTL: %d\n", TTL);
		printf("msgLifeTime: %d\n", msgLifeTime);
		printf("getMsgLifeTime: %d\n", getMsgLifeTime);
		printf("initNeighbors: %d\n", initNeighbors);
		printf("joinTimeout: %d\n", joinTimeout);
		printf("keepAliveTimeout: %d\n", keepAliveTimeout);
		printf("minNeighbors: %d\n", minNeighbors);
		printf("noCheck: %d\n", noCheck);
		printf("cacheProb: %g\n", cacheProb);
		printf("storeProb: %g\n", storeProb);
		printf("neighborStoreProb: %g\n", neighborStoreProb);
		printf("cacheSize: %d\n", cacheSize);
		printf("retryTime: %d\n", retryTime);
		for(int i = 0;i < (int)beaconNodes_hostname.size(); ++i)
		{
			printf("beacon_hostname: %s\n", beaconNodes_hostname[i]);
			printf("beacon_port: %s\n", beaconNodes_port[i]);
		}
		printf("beaconNodes_count: %d\n", beaconNodes_count);
		printf("node_type: %d\n", node_type);
	#endif
	
	hostname = new char[256];
	memset(hostname, 0, 256);
	if(gethostname(hostname, 256) != 0)
		perror("gethostname");
	int node_id_len = strlen(hostname) + strlen(port) + 1;
	node_id = new char[node_id_len];
	memset(node_id, 0, node_id_len);
	strcpy(node_id, hostname);
	strcat(node_id, "_");
	strcat(node_id, port);
	#ifdef _DEBUG_SUC_
		printf("node_id: %s\n", node_id);
	#endif
}

//fork all threads that are used for this node
void Node :: run()
{
	//init node instance id
	int node_id_len = strlen(node_id);
	node_instance_id = new char[node_id_len + 17];
	memset(node_instance_id, 0, node_id_len + 17);
	strcpy(node_instance_id, node_id);
	strcat(node_instance_id, "_");
	char time[10];
	get_time(time);
	strcat(node_instance_id, time);
	#ifdef _DEBUG_SUC_
		printf("node_instance_id: %s\n", node_instance_id);
	#endif

	pthread_mutex_lock(&shut_down_lock);
	bool isFirststart = gnShutdown;
	pthread_mutex_unlock(&shut_down_lock);
	if(isFirststart)
	{
		printf("servant:%s> ", port);
		fflush(stdout);
	}
	
	int err = 0;
	void* tret = NULL;
				
	//create thread that is in common between beacon nodes and non-beacon nodes
	//create dispatch thread	
	pthread_t dispatch_thread_id = 0;
	err = pthread_create(&dispatch_thread_id, NULL, dispatch_thread_helper, (void*)this);
	if(err != 0)
	{
		printf("cannot create as dispatch thread: %s\n", strerror(err));
		exit(1);
	}
	
	//create keyboard thread
	pthread_t keyboard_thread_id = 0;
	err = pthread_create(&keyboard_thread_id, NULL, keyboard_thread_helper, (void*)this);
	if(err != 0)
	{
		printf("cannot create as keyboard thread: %s\n", strerror(err));
		exit(1);
	}
	
	//create signal thread
	err = pthread_create(&signal_thread_id, NULL, signal_thread_helper, (void*)this);
	if(err != 0)
	{
		printf("cannot create as signal thread: %s\n", strerror(err));
		exit(1);
	}
	
	//create as a server listening thread
	pthread_t server_thread_id = 0;
	err = pthread_create(&server_thread_id, NULL, server_thread_helper, (void*)this);
	if(err != 0)
	{
		printf("cannot create as server thread: %s\n", strerror(err));
		exit(1);
	}

	//--------------------------------------------------------------------------------
	//if this node is non beacon node, establish temporary connection at first
	bool temp_connection = true;
	char path[strlen(homeDir) + strlen("/init_neighbor_list")];
	memset(path, 0, strlen(homeDir) + strlen("/init_neighbor_list"));
	strcpy(path, homeDir);
	strcat(path, "/init_neighbor_list");
	FILE *pInitFile = fopen(path, "r");
	if(node_type == NON_BEACON_NODE)
	{
		if(!pInitFile && !noCheck)	//init_neighbor_list not exists
			temp_connection = establish_temporary_connection();
		else	//init_neighbor_list exists, connect directly
			fclose(pInitFile);	
	}	
	//--------------------------------------------------------------------------------
		
	//create timer thread
	pthread_t timer_thread_id = 0;
	err = pthread_create(&timer_thread_id, NULL, timer_thread_helper, (void*)this);
	if(err != 0)
	{
		printf("cannot create as timer thread: %s\n", strerror(err));
		exit(1);
	}
	
	//--------------------------------------------------------------------------------
	//establish real connection
	//start connection
	if(node_type == BEACON_NODE)
		beacon_start();
	else if(node_type == NON_BEACON_NODE && temp_connection)
		non_beacon_start();
	//--------------------------------------------------------------------------------
	
	if(node_type == NON_BEACON_NODE && !temp_connection)
	{
		printf("\n");
 		fflush(stdout);
		pthread_kill(signal_thread_id, SIGUSR1);	
	}	
	
	err = pthread_join(signal_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with signal thread: %s\n", strerror(err));
		exit(1);
	}
	err = pthread_join(keyboard_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with keyboard thread: %s\n", strerror(err));
		exit(1);
	}
	err = pthread_join(timer_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with timer thread: %s\n", strerror(err));
		exit(1);
	}
	pthread_mutex_lock(&dispatch_queue_lock);
	dispatch_exit = true;
	pthread_cond_signal(&dispatch_queue_cv);
	pthread_mutex_unlock(&dispatch_queue_lock);
	err = pthread_join(dispatch_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with dispatch thread: %s\n", strerror(err));
		exit(1);
	}
	pthread_kill(server_thread_id, SIGUSR2);
	err = pthread_join(server_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with server thread: %s\n", strerror(err));
		exit(1);
	}
}

bool Node :: establish_temporary_connection()
{
	void* tret = NULL;
	int i = 0;
	
	printf("Start to join the network...\n");
	fflush(stdout);
	
	for(; i < beaconNodes_count; ++i)
	{
		pthread_t join_thread_id = 0;
		NodeIndex *node_index = new NodeIndex();
		node_index->node = this;
		node_index->index = i;
		int err = pthread_create(&join_thread_id, NULL, join_thread_helper, (void *)node_index);
		if(err != 0)
		{
			printf("cannot create as join thread: %s\n", strerror(err));
			exit(1);
		}
		err = pthread_join(join_thread_id, &tret);	
		if(err != 0)
		{
			printf("cannot join with join thread: %s\n", strerror(err));
			exit(1);
		}
		pthread_mutex_lock(&join_lock);
		if(join_success)
		{
			pthread_mutex_unlock(&join_lock);
			//delete node_index;
			break;
		}
		else if(join_shutdown)
		{
			pthread_mutex_unlock(&join_lock);
			//delete node_index;
			return false;
		}
		pthread_mutex_unlock(&join_lock);
		//delete node_index;
	}
	if(i == beaconNodes_count)
		return false;
	else
		return true;	
}

void Node :: beacon_start()
{
	//run node as a beacon node
	for(int i = 0; i < beaconNodes_count; ++i)
	{
		pthread_t client_thread_id = 0;
		NodeIndex *node_index = new NodeIndex();
		node_index->node = this;
		node_index->index = i;
		int err = pthread_create(&client_thread_id, NULL, client_thread_helper, (void *)node_index);
		if(err != 0)
		{
			printf("cannot create as client thread: %s\n", strerror(err));
			exit(1);
		}
	}
}

void Node :: non_beacon_start()
{
	//run code as non_beacon_node, open init_neighbor_list
	ifstream inData;
	int len = strlen(homeDir) + strlen("/init_neighbor_list");
	char path[len];
	memset(path, 0, len);
	strcpy(path, homeDir);
	strcat(path, "/init_neighbor_list");	
	non_beacon_neighbors = read_file(path);
	for(int i = 0; i < initNeighbors; ++i)
	{
		pthread_t client_thread_id = 0;	
		NodeIndex *non_node_index = new NodeIndex();
		non_node_index->node = this;
		non_node_index->index = i;
		int err = pthread_create(&client_thread_id, NULL, client_thread_helper, (void *)non_node_index);
		if(err != 0)
		{
			printf("cannot create as client thread: %s\n", strerror(err));
			exit(1);
		}
	}
}

void Node :: send_hello_message(int sockfd)
{	
	//connect establish successfully, send hello message 
	HelloMsg* helloMsg = create_helloMsg();
	DispatchElement* dispatchElm = new DispatchElement(helloMsg, sockfd, SEND_MSG);	
	//put hello message into the dispatch queue
	push_dispatch_queue(dispatchElm);
}

int Node :: client_socket_prepare(char* server_hostname, char* server_port, bool& connected)
{
	//init socket descriptor
	struct addrinfo hints, *servinfo = NULL, *p = NULL;
	int rv = 0;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int sockfd = 0;
	connected = false;
	
	/*
     * Begin code I did not write.
     * The code is downloaded from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
     * The copyright is Beej's Guide to Network Programming Using Internet Sockets
     */
	//get login server information
	#ifdef _DEBUG_
		printf("server_hostname: %s, server_port: %s\n", server_hostname, server_port);
	#endif
	if ((rv = getaddrinfo(server_hostname, server_port, &hints, &servinfo)) != 0) 
	{
		printf("getaddrinfo() fails in listener\n");
		exit(1);
	}
	// loop through all the results and connect to the first we can
	while(!connected)
	{
		pthread_mutex_lock(&control_thread_lock);
		if(all_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
		
		//I am a beacon node, I have already connected with this beacon node, no need to connect
		if(node_type == BEACON_NODE)
		{
			pthread_mutex_lock(&nb_map_lock);
			uint16_t serverPort = (uint16_t)atoi(server_port);
			map<int, Neighbor_node*>::iterator iter;
			for(iter = nb_map.begin(); iter != nb_map.end(); ++iter)
			{
				if(iter->second->equals(server_hostname, serverPort))
				{
					#ifdef _DEBUG_
						printf("Server: connect with %s_%d already establish\n", server_hostname, serverPort);
					#endif
					pthread_mutex_unlock(&nb_map_lock);
					connected = false;
					return 0;
				}	
			}	
			pthread_mutex_unlock(&nb_map_lock);
		}
	
		for(p = servinfo; p != NULL; p = p->ai_next) 
		{
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1)
			{
				#ifdef _DEBUG_
					perror("client: socket");
				#endif	
				continue;
			}
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sockfd);
				//perror("client: connect");
				continue;
			}
			connected = true;
			break;
		}	
		
		if(p == NULL)
		{
			#ifdef _DEBUG_
				fprintf(stderr, "client: failed to connect\n");
			#endif	
		}	
		 /*
		 * End code I did not write.
		 */			
		if(!connected && node_type == BEACON_NODE)
			sleep(retryTime);
		if(!connected && node_type == NON_BEACON_NODE)
			break;	
	}
	
	//the connection with Login server has established or quit connect
	freeaddrinfo(servinfo);
	return sockfd;
}

void Node :: server_socket_prepare()
{
	struct addrinfo hints, *servinfo = NULL, *p = NULL;
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int rv = 0;
    int sockfd = 0;
	
	/*
     * Begin code I did not write.
     * The code is downloaded from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
     * The copyright is Beej's Guide to Network Programming Using Internet Sockets
     */
	if ((rv = getaddrinfo(NULL, this->port, &hints, &servinfo)) != 0) 
	{
        printf("getaddrinfo() fails in listener\n");
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
    	if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        int reuse_addr = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) == -1) 
		{
			close(sockfd);
			perror("setsockopt");
			exit(1);
		}
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL)  
    {
        fprintf(stderr, "server: failed to bind\n");
		exit(1);
    }
    freeaddrinfo(servinfo); // all done with this structure
			
	if(listen(sockfd, BACKLOG) == -1)
	{
		close(sockfd);
        perror("listen");
        exit(1);
    }
    /*
     * End code I did not write.
     */
     
    // reap all dead processes
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigchld_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;
    sa.sa_flags |= SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) 
    {
        perror("sigaction");
        exit(1);
    }
     
    struct sockaddr_storage their_addr; 
	while(1) 
	{
		pthread_mutex_lock(&control_thread_lock);
		if(all_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
		
		int newsockfd=0;
		socklen_t sin_size = sizeof(their_addr);
		newsockfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);	
		if (newsockfd < 0) 
		{
			//perror("accept");
			continue;
		} 
		else 
		{
			pthread_t child_send_thread_id = 0;
			pthread_t child_recv_thread_id = 0;
			#ifdef _DEBUG_
				printf("Listening socket %d creates\n", newsockfd);
			#endif	
			fork_send_recv_thread(newsockfd, child_send_thread_id, child_recv_thread_id);
		}
	}
	close(sockfd);	
}

void* Node :: server_thread_helper(void* arg)
{
	Node* node = (Node*)arg;
	node->server_socket_prepare();	
	pthread_exit((void*)0);
}

void* Node :: client_thread_helper(void* arg)
{
	NodeIndex* node_index = (NodeIndex *)arg;
	Node *node = node_index->node;
	int index = node_index->index;
	node->client_thread(index);	
	pthread_exit((void*)0);
}

void Node :: client_thread(int index)
{
	//connect to other beacon nodes and create client recv and send threads 
	bool connected = false;
	void* tret = NULL;
	int sockfd = 0;
	char* init_hostname = NULL;
	char* init_port = NULL;
	if(node_type == BEACON_NODE)
	{
		init_hostname = new char[strlen(beaconNodes_hostname[index]) + 1];
		memset(init_hostname, 0, strlen(beaconNodes_hostname[index]) + 1);
		init_port = new char[6];
		memset(init_port, 0, 6);
		strncpy(init_hostname, beaconNodes_hostname[index], strlen(beaconNodes_hostname[index]));
		strcpy(init_port, beaconNodes_port[index]);
		sockfd = client_socket_prepare(init_hostname, init_port, connected);
	}	
	else	//non beacon node connect to beacon nodes in init_neighbor_list
	{
		init_hostname = new char[strlen(non_beacon_neighbors[index]) + 1];
		memset(init_hostname, 0, strlen(non_beacon_neighbors[index]) + 1);
		init_port = new char[6];
		memset(init_port, 0, 6);
		parseBeacon(non_beacon_neighbors[index], init_hostname, init_port);
		sockfd = client_socket_prepare(init_hostname, init_port, connected);	
	}	
	if(!connected)
	{
		if(node_type == NON_BEACON_NODE)
		{
			bool rejoin = false;
			pthread_mutex_lock(&join_lock);
			connect_fail_count++;
			if((initNeighbors - connect_fail_count) < minNeighbors)
				rejoin = true;
			pthread_mutex_unlock(&join_lock);
			
			if(rejoin)	//cannot connect to minNeighbors, soft-restart
			{
				#ifdef _DEBUG_
					printf("2 Soft Restart!\n");
				#endif
				pthread_mutex_lock(&shut_down_lock);
				gnShutdown = false;
				pthread_mutex_unlock(&shut_down_lock);
				exit_threads();
				pthread_cancel(signal_thread_id);
			}	
		}	
		return;
	}	
	//connect successful, add to neighbor_table	
	#ifdef _DEBUG_
		printf("Connecting socket %d creates\n", sockfd);
	#endif	
	add_to_neighbor_table(sockfd, init_hostname, init_port);
	
	//fork child recv and send thread	
	pthread_t child_send_thread_id = 0;
	pthread_t child_recv_thread_id = 0;
	fork_send_recv_thread(sockfd, child_send_thread_id, child_recv_thread_id);
	
	//send hello message for real connection
	send_hello_message(sockfd);
	
	int err = pthread_join(child_recv_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with child recv thread: %s\n", strerror(err));
		exit(1);
	}	
}

void Node :: fork_send_recv_thread(int sockfd, pthread_t& child_send_thread_id, pthread_t& child_recv_thread_id)
{
	//fork send thread
	SendThread_arg* send_arg = new SendThread_arg();
	send_arg->node = this;
	send_arg->sockfd = sockfd;	
	
	//put send thread info thread_id and resource into id_map 	
	pthread_mutex_lock(&id_map_lock);
	SendThread_Resourse *resource = new SendThread_Resourse();
	if(id_map.find(sockfd) == id_map.end())	//this socket has not seen before
		id_map.insert(pair<int, SendThread_Resourse*>(sockfd, resource));
	else	//this socket is in id_map
	{
		#ifdef _DEBUG_
			printf("Error: sockfd %d has already used by other send thread\n", sockfd);
			assert(false);
		#endif	
	}
	pthread_mutex_unlock(&id_map_lock);

	int err = pthread_create(&child_send_thread_id, NULL, child_send_thread_helper, (void*)send_arg);
	if(err != 0)
	{
		printf("cannot create as child send thread: %s\n", strerror(err));
		exit(1);
	}
	
	//fork recv thread
	RecvThread_arg* recv_arg = new RecvThread_arg();
	recv_arg->node = this;
	recv_arg->sockfd = sockfd;
	recv_arg->send_thread_id = child_send_thread_id;
	err = pthread_create(&child_recv_thread_id, NULL, child_recv_thread_helper, (void*)recv_arg);
	if(err != 0)
	{
		printf("cannot create as child recv thread: %s\n", strerror(err));
		exit(1);
	}
}

void* Node :: child_send_thread_helper(void* arg)
{
	SendThread_arg* send_arg = (SendThread_arg*)arg;
	Node* node = send_arg->node;
	int sockfd = send_arg->sockfd;
	node->child_send_thread(sockfd);
	pthread_exit((void*)0);
}

void* Node :: child_recv_thread_helper(void* arg)
{
	RecvThread_arg* recv_arg = (RecvThread_arg*)arg;
	Node* node = recv_arg->node;
	int sockfd = recv_arg->sockfd;
	pthread_t send_thread_id = recv_arg->send_thread_id;	
	node->child_recv_thread(sockfd, send_thread_id);
	pthread_exit((void*)0);
}

void* Node :: dispatch_thread_helper(void* arg)
{
	Node* node = (Node*)arg;
	node->dispatch_thread();	
	pthread_exit((void*)0);
}

void* Node :: keyboard_thread_helper(void* arg)
{
	Node* node = (Node*)arg;
	node->keyboard_thread();
	pthread_exit((void*)0);
}

void* Node :: timer_thread_helper(void* arg)
{
	Node* node = (Node*)arg;
	node->timer_thread();	
	pthread_exit((void*)0);
}

void* Node :: signal_thread_helper(void* arg)
{
	Node* node = (Node *)arg;
	node->signal_thread();
	#ifdef _DEBUG_
		//printf("signal_thread exit\n");
	#endif
	pthread_exit((void*)0);
}

void Node :: child_recv_thread(int sockfd, pthread_t send_thread_id)
{
	char header_buf[HEADER_LEN];
	memset(header_buf, 0, HEADER_LEN);
	char data_buf[MAXDATASIZE];
	memset(data_buf, 0, MAXDATASIZE);
	//----------------------------------------------------
	//init variables for select system call
	struct timeval tv;
	tv.tv_sec = keepAliveTimeout;
	tv.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	//----------------------------------------------------
	
	bool recv_helloMsg = false;
	bool socket_close = false;
	while(true)
	{
		memset(header_buf, 0, HEADER_LEN);
		int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);		
		//-----------------------------------------------------------------------------
		//check if it is time to exit
		pthread_mutex_lock(&control_thread_lock);
		if(recv_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
		//-----------------------------------------------------------------------------
		
		if(rv == -1)
		{
			#ifdef _DEBUG_
				perror("select");
				printf("Server: other node closes conncection\n");
			#endif	
			break;
		}
		else if(rv == 0)
		{
			#ifdef _DEBUG_
				printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
			#endif
			break;
		}
		else
		{
			if(FD_ISSET(sockfd, &readfds))	//receive data
			{
				int recvLen = 0;
				while(recvLen != HEADER_LEN)
				{
					int status = recv(sockfd, header_buf + recvLen, HEADER_LEN, 0);
					if(status == -1)
					{
						#ifdef _DEBUG_
							perror("recv");
						#endif
						socket_close = true;	
						break;
					}
					else if(status == 0)	//the client to server connect close
					{
						#ifdef _DEBUG_
							printf("Server: client close socket to the server, header\n");
						#endif
						socket_close = true;
						break;
					}
					recvLen += status;
				}
				if(socket_close)
					break;
				//recv_data_len++;
			}
		}	
		MsgHeader header;
		get_header_from_msg(header_buf, header);
		
		if(header.msgType == STOR)
		{
			bool recv_exit = handle_recv_storeMsg(sockfd, header);
			if(recv_exit)
				break;
		}
		else if(header.msgType == SHRS)
		{
			bool recv_exit = handle_recv_searchResMsg(sockfd, header);
			if(recv_exit)
				break;
		}
		else if(header.msgType == GTRS)
		{
			bool recv_exit = handle_recv_getResMsg(sockfd, header);
			if(recv_exit)
				break;
		}
		else if(header.msgType == STRS && header.reserved == 1)
		{
			bool recv_exit = handle_recv_statusResFileMsg(sockfd, header);
			if(recv_exit)
				break;
		}
		else
		{
			//receive payload in the message
			uint32_t dataLen = header.dataLen;
			char msg_data[dataLen];
			memset(msg_data, 0, dataLen);
			if(dataLen != 0)
			{
				int normal_time = dataLen / MAXDATASIZE;
				int last_time = dataLen % MAXDATASIZE;	
				
				for(int i = 0; i < normal_time; i++)
				{
					memset(data_buf, 0, MAXDATASIZE);	
					//set keepAliveTimeout seconds the same as header
					rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);
					//-----------------------------------------------------------------------------
					//check if it is time to exit
					pthread_mutex_lock(&control_thread_lock);
					if(recv_thread_exit)
					{
						pthread_mutex_unlock(&control_thread_lock);
						break;
					}
					pthread_mutex_unlock(&control_thread_lock);
					//-----------------------------------------------------------------------------
					
					if(rv == -1)
					{
						#ifdef _DEBUG_
							perror("select");
							printf("Server: other node closes conncection\n");
						#endif	
						break;
					}
					else if(rv == 0)
					{
						#ifdef _DEBUG_
							printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
						#endif
						break;
					}
					else
					{
						if(FD_ISSET(sockfd, &readfds))
						{
							int recvLen = 0;
							while(recvLen != MAXDATASIZE)
							{
								int status = recv(sockfd, data_buf + recvLen, MAXDATASIZE, 0);
								if(status == -1)
								{
									#ifdef _DEBUG_
										perror("recv");
									#endif	
									socket_close = true;
									break;
								}	
								else if(status == 0)
								{
									#ifdef _DEBUG_
										printf("Server: client close socket to the server, payload\n");
									#endif
									socket_close = true;
									break;
								}
								recvLen += status;
							}
							if(socket_close)
								break;
							//recv_data_len++;
						}
					}
					memcpy(&msg_data[i * MAXDATASIZE], data_buf, MAXDATASIZE);
				}
			
				memset(data_buf, 0, MAXDATASIZE);
				rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);
				//-----------------------------------------------------------------------------
				//check if it is time to exit
				pthread_mutex_lock(&control_thread_lock);
				if(recv_thread_exit)
				{
					pthread_mutex_unlock(&control_thread_lock);
					break;
				}
				pthread_mutex_unlock(&control_thread_lock);
				//-----------------------------------------------------------------------------
				
				if(rv == -1)
				{
					#ifdef _DEBUG_
						perror("select");
						printf("Server: other node closes conncection\n");
					#endif
					break;
				}
				else if(rv == 0)
				{
					#ifdef _DEBUG_
						printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
					#endif
					break;
				}
				else
				{
					if(FD_ISSET(sockfd, &readfds))
					{
						int recvLen = 0;
						while(recvLen != last_time)
						{
							int status = recv(sockfd, data_buf + recvLen, last_time, 0);
							if(status == -1)
							{
								#ifdef _DEBUG_
									perror("recv");
								#endif	
								socket_close = true;
								break;
							}	
							else if(status == 0)
							{
								#ifdef _DEBUG_
									printf("Server: client close socket to the server, payload\n");
								#endif
								socket_close = true;
								break;
							}
							recvLen += status;
						}	
						if(socket_close)
							break;
						//srecv_data_len++;
					}
				}
				memcpy(&msg_data[normal_time * MAXDATASIZE], data_buf, last_time);	
			}				
				
			//this message has seen before or not
			pthread_mutex_lock(&UOID_time_map_lock);
			bool received_before = false;
			map<UOID_Map, int>::iterator it;
			for(it = UOID_time_map.begin(); it != UOID_time_map.end(); ++it)
			{
				if(!memcmp(it->first.m_UOID, header.UOID, UOID_LEN))
				{
					received_before = true;
					break;
				}	
			}	
			
			if(received_before)	//this message has received before
			{
				//printf("Server: message received before\n");	
				pthread_mutex_unlock(&UOID_time_map_lock);	
			}	
			else	//this message has not seen before
			{
				UOID_Map time_map_UOID((unsigned char*)header.UOID);
				UOID_time_map.insert(pair<UOID_Map, int>(time_map_UOID, currentTime()));
				pthread_mutex_unlock(&UOID_time_map_lock);		
				
				if(header.msgType == HLLO)
				{
					bool recv_exit = handle_recv_helloMsg(header, msg_data, sockfd);
					recv_helloMsg = true;
					if(recv_exit)
						break;
				}
				else if(header.msgType == NTFY)
				{
					handle_recv_notifyMsg(header, msg_data, sockfd);
					break;
				}
				else	//handled other messages
				{
					handle_recv_msg(header, msg_data, sockfd);
				}
			}
		}	
	}	
	//keep alive time out, close responding send thread 
	send_thread_close(sockfd);	
	void* tret = NULL;
	int err = pthread_join(send_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with child send thread: %s\n", strerror(err));
		exit(1);
	}
	
	//close this connection
	pthread_mutex_lock(&id_map_lock);	
	id_map.erase(sockfd);	
	pthread_mutex_unlock(&id_map_lock);
	pthread_mutex_lock(&nb_map_lock);
	nb_map.erase(sockfd);
	#ifdef _DEBUG_
		int size = (int)nb_map.size();
	#endif	
	pthread_mutex_unlock(&nb_map_lock);	
	pthread_mutex_lock(&join_lock);	
	remove(join_sockets.begin(), join_sockets.end(), sockfd);
	pthread_mutex_unlock(&join_lock);
	
	#ifdef _DEBUG_
		printf("Connection socket %d exit, Now nb_map size: %d\n", sockfd, size);	
	#endif
	if(node_type == NON_BEACON_NODE && recv_helloMsg && socket_close)
		send_check_message(sockfd);	
	close(sockfd);
}

void Node :: child_send_thread(int sockfd)
{	
	//get resource from resource table
	pthread_mutex_lock(&id_map_lock);	
	SendThread_Resourse* res = id_map.find(sockfd)->second;	
	pthread_mutex_unlock(&id_map_lock);

	while(true)
	{
		//get send message from send_msg queue
		pthread_mutex_lock(&res->send_queue_lock);
		while((int)res->send_queue.size() == 0 && !res->thread_exit)
			pthread_cond_wait(&res->send_queue_cv, &res->send_queue_lock);
		if(res->thread_exit)
		{
			std::queue<DispatchElement*> empty;
			std::swap(res->send_queue, empty);
			pthread_mutex_unlock(&res->send_queue_lock);
			break;	
		}	
		
		DispatchElement* dispatchElm = res->send_queue.front();
		Msg* msg = dispatchElm->msg;
		res->send_queue.pop();	
		
		if(msg->header.msgType == STOR || msg->header.msgType == SHRS || msg->header.msgType == GTRS)
		{
			msg->send_header(sockfd);
			msg->send_file_payload(sockfd, homeDir);
		}
		else if(msg->header.msgType == STRS && msg->header.reserved == 1)
		{
			msg->send_header(sockfd);
			msg->send_file_payload(sockfd, homeDir);
		}
		else
		{			
			//send the message out
			msg->send_header(sockfd);		
			//send payload
			char msg_data_buf[msg->header.dataLen];
			memset(msg_data_buf, 0, msg->header.dataLen);
			msg->get_payload(SEND_DATA, msg_data_buf);
			msg->send_payload(sockfd, msg_data_buf);	
		}
		pthread_mutex_unlock(&res->send_queue_lock);
		
		string s = get_neighbor(sockfd);
		if(dispatchElm->type == SEND_MSG)
			log->write_to_file('s', s.c_str(), msg);
		else
			log->write_to_file('f', s.c_str(), msg);
		//delete dispatchElm;
	}
}

void Node :: dispatch_thread()
{
	while(true)
	{	
		pthread_mutex_lock(&dispatch_queue_lock);
		while((int)dispatch_queue.size() == 0 && !dispatch_exit)
			pthread_cond_wait(&dispatch_queue_cv, &dispatch_queue_lock);		
		
		if(dispatch_exit)
		{
			std::queue<DispatchElement*> empty;
			std::swap(dispatch_queue, empty);
			pthread_mutex_unlock(&dispatch_queue_lock);
			break;
		}			
		//there is message in the dispatch_queue, read them
		DispatchElement* dispatchElm = dispatch_queue.front();
		int sockfd = dispatchElm->sockfd;
		dispatch_queue.pop();
		pthread_mutex_unlock(&dispatch_queue_lock);	
		
		//put the resource into corresponding send queue
		pthread_mutex_lock(&id_map_lock);	
		SendThread_Resourse* res = id_map.find(sockfd)->second;
		pthread_mutex_lock(&res->send_queue_lock);	
		pthread_mutex_unlock(&id_map_lock);		
		res->send_queue.push(dispatchElm);
		pthread_cond_signal(&res->send_queue_cv);
		pthread_mutex_unlock(&res->send_queue_lock);
	}
}

void Node :: keyboard_thread()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	string str;
	
	bool printHint = true;
	bool isDoSearch = false;
	fd_set readfds;
	while(true)
	{	
		pthread_mutex_lock(&control_thread_lock);
		if(all_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
		   
		FD_ZERO(&readfds);
		FD_SET(STDIN, &readfds);
		int rv = select(STDIN+1, &readfds, NULL, NULL, &tv);
		if(rv == -1) 
		{
			perror("select");
			exit(1);
		}			 
		else if(rv == 0){} 
		else
		{				
			if(FD_ISSET(STDIN, &readfds))
			{
				string str;
				getline(cin,str);
				
				//parse string
				vector<string> words = tokenize(str);
				if(words[0].compare("status") == 0)	//status command
				{
					isDoSearch = false;
					if((int)words.size() < 4)
					{
						printf("Error: invalid command\n");
						continue;
					}
					
					int len = (int)words[3].length();
					statusFilename = new char[len];
					memset(statusFilename, 0, len);
					strcpy(statusFilename, words[3].c_str());
					char command_TTL[3];
					memset(command_TTL, 0, 3);
					strcpy(command_TTL, words[2].c_str());
					unsigned char status_TTL = atoi(command_TTL);
																
					pthread_mutex_lock(&status_file_lock);
					bool issent = issent_status;
					pthread_mutex_unlock(&status_file_lock);
					
					if(!issent)
					{
						statusFile = fopen(statusFilename, "w+");
						if(words[1].compare("neighbors") == 0)
						{
							StatusMsg *statusMsg = create_statusMsg(status_TTL, 0x01);
							send_all_neighbors_msg(statusMsg);	
						}	
						else if(words[1].compare("files") == 0)
						{
							StatusMsg *statusMsg = create_statusMsg(status_TTL, 0x02);
							send_all_neighbors_msg(statusMsg);	
						}
						else
						{
							printf("Error: invalid command\n");								
						}		
					}					
				}
				else if(words[0].compare("shutdown") == 0)	//shutdown command
				{
					isDoSearch = false;
					printHint = false;
					pthread_mutex_lock(&control_thread_lock);
					userShutdown = true;
					pthread_mutex_unlock(&control_thread_lock);
					pthread_kill(signal_thread_id, SIGUSR1);
				}
				else if(words[0].compare("store") == 0)	//store command
				{
					isDoSearch = false;
					vector<string> keywords;
					unsigned char store_TTL = (unsigned char)atoi(words[2].c_str());
					for(int i = 3; i < (int)words.size(); ++i)
					{
						//parse keywords into a container
						parseKeywords(words[i], keywords);
					}
					//there is keywords for this file
					if(keywords.size() > 0)
					{
						int storeFile_index = storeFile_filesys(words[1].c_str(), keywords, node_instance_id, homeDir);
						update_filesys();
						if(storeFile_index != -1 && store_TTL > 0)
							create_storeMsg(storeFile_index, store_TTL);
					}	
				}
				else if(words[0].compare("search") == 0)
				{
					isDoSearch = true;
					char commandPrefix[15];
					memset(commandPrefix, 0, 15);
					char commandData[256];
					memset(commandData, 0, 256);
					parseLine(words[1].c_str(), commandPrefix, commandData);
					
					string searchQuery;
					unsigned char searchType = 0;
					char searchMsg_UOID[UOID_LEN];
					memset(searchMsg_UOID, 0, UOID_LEN);
					if( !strcmp(commandPrefix, "filename") )
					{
						searchType = 1;
						searchQuery = commandData;
						create_searchMsg(searchType, searchQuery, searchMsg_UOID);
					}
					else if( !strcmp(commandPrefix, "sha1hash") )
					{
						searchType = 2;
						searchQuery = commandData;
						create_searchMsg(searchType, searchQuery, searchMsg_UOID);
					} 
					else if( !strcmp(commandPrefix, "keywords") ) 
					{
						searchType = 3;
						searchQuery += commandData;
						for(size_t i = 2; i < words.size(); ++i)
						{
							searchQuery += (" " + words[i]);
						}
						create_searchMsg(searchType, searchQuery, searchMsg_UOID);
					}
					else
					{
						searchType = 0;
						printf("Error: invalid command\n");
					}
					
					if(searchType)				
						show_searchResponse(searchType, searchQuery, searchMsg_UOID);
				}
				else if(words[0].compare("get") == 0 && isDoSearch)
				{
					isDoSearch = true;
					uint32_t search_file_index = (uint32_t)atoi(words[1].c_str());
					
					bool isValid = false;
					pthread_mutex_lock(&search_lock);
					if( searchFileID_map.find(search_file_index) != searchFileID_map.end())
						isValid = true;
					pthread_mutex_unlock(&search_lock);
					//enter file index is valid	
					if(isValid)
					{
						string getFilename;
						if(words.size() >= 3)
							getFilename = words[2];
						else
						{
							pthread_mutex_lock(&search_lock);
							getFilename = searchFileName_map[search_file_index];
							pthread_mutex_unlock(&search_lock);
						}	
						#ifdef DEBUG
							printf("getFilename: %s\n", getFilename.c_str());	
						#endif
							
						//this get filename exists or not
						bool isReplace = true;
						FILE* pf = fopen( getFilename.c_str(), "r" );
						if(pf)
						{
							while(true)
							{
								select(0, NULL, NULL, NULL, &tv);
								
								printf("File %s exists, do you want to replace it[yes/no]?", getFilename.c_str());
								fflush(stdout);
								string input;
								getline(cin, input);
								if(input[0] == 'y')
								{	
									isReplace = true;
									break;
								}
								else if(input[0] == 'n')
								{
									isReplace = false;
									break;
								}
							}
							fclose(pf);
						}
						
						bool isLocal = false;
						uint32_t dataFileIndex = 0;
						pthread_mutex_lock(&search_lock);
						UOID_Map file_uoid = searchFileID_map.find(search_file_index)->second;
						UOID_Map sha1_uoid = searchFileSHA1_map.find(search_file_index)->second;
						pthread_mutex_unlock(&search_lock);
						//check if this fileID is local
						pthread_mutex_lock(&fileID_lock);
						map<UOID_Map, uint32_t>::iterator iter = fileIDIndex_map.find(file_uoid);
						if( iter != fileIDIndex_map.end())
						{
							isLocal = true;
							dataFileIndex = iter->second;
						}	
						pthread_mutex_unlock(&fileID_lock);
						
						if(isLocal)
						{
							//if the file is in the cache, remove it from cache in advance
							remove_cacheList_if_needed(dataFileIndex, homeDir);
							
							//copy file to .data file to current working directory
							char dataFilename[256];
							memset(dataFilename, 0, 256);
							create_dataFilename(dataFilename, homeDir, dataFileIndex);
							
							if(isReplace)
							{
								ifstream input(dataFilename, ios::in |ios::binary);
								ofstream output(getFilename.c_str(), ios::out |ios::binary);
								output << input.rdbuf(); 
							}
						}
						else
						{
							//this file is not in local file system, create get message
							create_getMsg(file_uoid.m_UOID, sha1_uoid.m_UOID, getFilename, isReplace);
						}
					}
					else
					{
						printf("Error: get command index is not in search results\n");
					}
					
				}
				else if(words[0].compare("delete") == 0)
				{
					isDoSearch = false;
					char deleteFilename[256];
					memset(deleteFilename, 0, 256);
					unsigned char deleteSHA1[SHA_DIGEST_LENGTH];
					memset(deleteSHA1, 0, SHA_DIGEST_LENGTH);
					unsigned char deleteNonce[SHA_DIGEST_LENGTH];
					memset(deleteNonce, 0, SHA_DIGEST_LENGTH);
					bool havePass = false;
					
					for(int i = 1; i < (int)words.size(); ++i)
					{
						char commandPrefix[20];
						memset(commandPrefix, 0, 20);
						char commandData[256];
						memset(commandData, 0, 256);
						parseLine(words[i].c_str(), commandPrefix, commandData);
						
						if(!strcmp(commandPrefix, "FileName"))
						{
							strcpy(deleteFilename, commandData);
						}
						else if(!strcmp(commandPrefix, "SHA1"))
						{
							string str(commandData);
							stringToHex(str, deleteSHA1);
						}
						else if(!strcmp(commandPrefix, "Nonce"))
						{
							string str(commandData);
							stringToHex(str, deleteNonce);
						}
					}
					
					vector<uint32_t> threeMatchResults = get_matchFileIndex(deleteFilename, deleteSHA1, deleteNonce);					
					if(threeMatchResults.size() == 1)
					{
						char passFileName[256];
						memset(passFileName, 0, 256);
						create_passFilename(passFileName, homeDir, threeMatchResults[0]);
						FILE* pf = fopen(passFileName, "r");
						if(pf)	//.pass file exists in the file system
						{
							havePass = true;
							unsigned char deletePass[UOID_LEN];
							memset(deletePass, 0, UOID_LEN);
							getPassword_PassFile(pf, deletePass);
							
							deleteFile_command(threeMatchResults[0], homeDir);
							remove(passFileName);
							create_deleteMsg(deleteFilename, deleteSHA1, deleteNonce, deletePass);
						}
						fclose(pf);
						
						//not this file store owner
						if(!havePass)
						{
							bool isRandom = false;
							while(true)
							{
								printf("No one-time password found.\n");
								printf("Okay to use a random password [yes/no]?");
								fflush(stdout);
								string input;
								getline(cin, input);
								if(input[0] == 'y')
								{
									isRandom = true;
									break;
								}
								else if(input[0] == 'n')
								{
									isRandom = false;
									break;
								}
							}
							if(isRandom)
							{
								unsigned char deletePass[UOID_LEN];
								memset(deletePass, 0, UOID_LEN);
								char obj[] = "random";
								GetUOID(node_instance_id, obj, (char*)deletePass, UOID_LEN);
								create_deleteMsg(deleteFilename, deleteSHA1, deleteNonce, deletePass);
							}
						}
					}
					else
					{
						printf("Error: There is no match file in this node's file system\n");					
					}
				}
				else if(words[0].compare("\0") == 0)
				{}
				else if(words[0].compare("get") == 0 && !isDoSearch)
				{}
				else
				{
					printf("Error: invalid command\n");
				}
			}
			
			if(printHint)
			{
				printf("servant:%s> ", port);
				fflush(stdout);
			}	
		}
	}
}

void Node :: timer_thread()
{
	int last_keepAlive_time = currentTime();
	while(true)
	{
		pthread_mutex_lock(&control_thread_lock);
		if(all_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
			
		pthread_mutex_lock(&join_lock);
		vector<int> joining(join_sockets);
		pthread_mutex_unlock(&join_lock);
		int current_time = currentTime();		
		//if more than half time of the last keepAliveMsg, send this message
		if((current_time - last_keepAlive_time) >= keepAliveTimeout / 2)
		{
			vector<int> neighbor_sockfds;
			pthread_mutex_lock(&nb_map_lock);
			map<int, Neighbor_node*>::iterator it;
			for(it = nb_map.begin(); it != nb_map.end(); ++it)
			{
				vector<int>::iterator join_iter = find(joining.begin(), joining.end(), it->first);
				if(join_iter == joining.end())	
					neighbor_sockfds.push_back(it->first);
			}	
			pthread_mutex_unlock(&nb_map_lock);
			
			for(int i = 0; i < (int)neighbor_sockfds.size(); ++i)
			{
				KeepAliveMsg* keepAliveMsg = create_keepAliveMsg();
				DispatchElement* dispatchElm = new DispatchElement(keepAliveMsg, neighbor_sockfds[i], SEND_MSG);	
				push_dispatch_queue(dispatchElm);
			}	
			last_keepAlive_time = currentTime();
		}
		
		//find if any message stays longer than msgLifeTime
		//vector<string> UOID_timeOut;
		pthread_mutex_lock(&UOID_time_map_lock);
		for(map<UOID_Map, int>::iterator it = UOID_time_map.begin(); it != UOID_time_map.end();)
		{
			if((current_time - it->second) >= msgLifeTime)
			{
				//UOID_timeOut.push_back(it->first);
				UOID_time_map.erase(it++);
			}
			else
				it++;	
		}
		pthread_mutex_unlock(&UOID_time_map_lock);	
		
// 		pthread_mutex_lock(&routing_map_lock);
// 		for(int i = 0; i < (int)UOID_timeOut.size(); ++i)
// 		{
// 			for(map<string, int>::iterator iter_rout = routing_map.begin(); iter_rout != routing_map.end();)
// 			{
// 				if(!memcmp(UOID_timeOut[i].c_str(), iter_rout->first.c_str(), UOID_LEN))
// 					routing_map.erase(iter_rout++);	
// 				else
// 					iter_rout++;	
// 			}	
// 		}
// 		pthread_mutex_unlock(&routing_map_lock);
// 		UOID_timeOut.clear();
		
		//check if there is statusMsg timeout
		pthread_mutex_lock(&status_file_lock);
		if(issent_status)
		{
			if((current_time - status_sent_time) >= msgLifeTime)	//write to extfile
			{
				fclose(statusFile);
				statusPorts.clear();
				//delete[] statusFilename;
				issent_status = false;
			}	
		}
		pthread_mutex_unlock(&status_file_lock);
		
		if(!noCheck && (node_type == NON_BEACON_NODE))
		{
			//check if check sent out
			bool soft_restart = false;
			pthread_mutex_lock(&check_lock);
			if(issent_check)
			{
				if((current_time - check_sent_time) >= joinTimeout)
				{
					if(!checkRes_recv)	//not receive checkResMsg, do soft-restart
					{
						soft_restart = true;
					}	
					else
					{
						//printf("Received check response\n");
						#ifdef _DEBUG_
							printf("Received check response\n");
						#endif	
					} 	
					issent_check = false;				
				}	
			}
			pthread_mutex_unlock(&check_lock);
			//need to soft restart, because not receive checkResMsg	
			if(soft_restart)
			{
				send_notify_message(SELF_RESTART);
				#ifdef _DEBUG_
					printf("1 Soft Restart!\n");
				#endif	
				pthread_mutex_lock(&shut_down_lock);
				gnShutdown = false;
				pthread_mutex_unlock(&shut_down_lock);
				exit_threads();
				pthread_cancel(signal_thread_id);
			}
		}
		
		pthread_mutex_lock(&search_lock);
		if(!search_exit)
		{
			map<UOID_Map, bool>::iterator iter1 = search_done_map.begin();
			map<UOID_Map, int>::iterator iter2 = search_time_map.begin();
			while( iter1 != search_done_map.end() )
			{
				if(!iter1->second)
				{
					if( (current_time - iter2->second) >= msgLifeTime)
					{
						search_exit = true;
						iter1->second = true;
						pthread_cond_signal(&searchResponse_queue_cv);
					}
					break;
				}
				
				++iter1;
				++iter2;
			}
		}
		pthread_mutex_unlock(&search_lock);
		
		sleep(1);
	}
}

void Node :: signal_thread()
{
	int err = 0;
	int signo = 0;
	while(true)
	{
		err = sigwait(&mask, &signo);	//thread waits for some signal
 		if(err != 0)
 		{
 			printf("sigwait failed\n");
 			exit(1);
 		}
 		
 		pthread_mutex_lock(&control_thread_lock);
		if(all_thread_exit)
		{
			pthread_mutex_unlock(&control_thread_lock);
			break;
		}
		pthread_mutex_unlock(&control_thread_lock);
		
 		if(signo == SIGALRM)	//auto-shutdown
 		{
 			exit_threads();
 			pthread_mutex_lock(&shut_down_lock);
			gnShutdown = true;
			pthread_mutex_unlock(&shut_down_lock);
			printf("\n");
 			fflush(stdout);
 			break;
 		}
 		else if(signo == SIGUSR1)	//user shut down or non_beacon_node join fail
 		{
 			pthread_mutex_lock(&control_thread_lock);
 			bool isShutdown = userShutdown;
 			pthread_mutex_unlock(&control_thread_lock);
 			if(isShutdown)
 				send_notify_message(USER_SHUTDOWN);
 				
 			exit_threads();
 			pthread_mutex_lock(&shut_down_lock);
			gnShutdown = true;
			pthread_mutex_unlock(&shut_down_lock);
 			break;
 		} 
 		else if(signo == SIGINT) 
 		{
 			pthread_mutex_lock(&search_lock);
 			if(!search_exit)
 			{
 				search_exit = true;
 				pthread_cond_signal(&searchResponse_queue_cv);
 			}
 			else
 			{
 				printf("\nservant:%s> ", port);
				fflush(stdout);
 			}		
 			pthread_mutex_unlock(&search_lock);
 		}		
	}
}

void Node :: send_thread_close(int sockfd)
{
	pthread_mutex_lock(&id_map_lock);	
	SendThread_Resourse* res = id_map.find(sockfd)->second;	
	res->thread_exit = true;
	pthread_cond_signal(&res->send_queue_cv);
	pthread_mutex_unlock(&id_map_lock);
}

void Node :: push_dispatch_queue(DispatchElement *dispatchElm)
{
	pthread_mutex_lock(&dispatch_queue_lock);
	dispatch_queue.push(dispatchElm);
	pthread_cond_signal(&dispatch_queue_cv);
	pthread_mutex_unlock(&dispatch_queue_lock);
}

void Node :: add_to_neighbor_table(int sockfd, char* init_hostname, char* init_port)
{
	//connect successful, add to neighbor_table
	pthread_mutex_lock(&nb_map_lock);
	uint16_t init_neighbor_port = (uint16_t)atoi(init_port);
	Neighbor_node *nb_node = new Neighbor_node(init_hostname, init_neighbor_port);
	if(nb_map.find(sockfd) == nb_map.end())
		nb_map.insert(pair<int, Neighbor_node*>(sockfd, nb_node));	
	else
	{
		//printf("This neighbor socket %d exists\n", sockfd);
	}		
	pthread_mutex_unlock(&nb_map_lock);	
}

vector<uint32_t> Node:: get_matchFileIndex(const char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce)
{
	//get match results form filename and SHA1
	vector<uint32_t> filenameResults = search_filename(deleteFilename);
	vector<uint32_t> SHA1Resutls = search_SHA1(deleteSHA1);
	vector<uint32_t> twoMatchResults;
	for(size_t i = 0; i < filenameResults.size(); ++i)
	{
		for(size_t j = 0; j < SHA1Resutls.size(); ++j)
		{
			if(filenameResults[i] == SHA1Resutls[j])
			{
				twoMatchResults.push_back(filenameResults[i]);
				break;
			}	
		}
	}
	//match filename, SHA1, and Nonce
	vector<uint32_t> threeMatchResults;
	for(size_t k = 0; k < twoMatchResults.size(); ++k)
	{
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, twoMatchResults[k]);
		
		unsigned char storedMetaDataNonce[UOID_LEN];
		memset(storedMetaDataNonce, 0, UOID_LEN);
		getNonce_metaData(metaDataFilename, storedMetaDataNonce);
		
		if(!memcmp(deleteNonce, storedMetaDataNonce, UOID_LEN))
		{
			threeMatchResults.push_back(twoMatchResults[k]);
		}
	}
	
	assert(threeMatchResults.size() <= 1);
	return threeMatchResults;
}

void Node :: exit_threads()	//close all non control thread
{
	pthread_mutex_lock(&control_thread_lock);
	recv_thread_exit = true;
	all_thread_exit = true;
	pthread_mutex_unlock(&control_thread_lock);
}

void Node :: cleanup()
{
	if(node_type == NON_BEACON_NODE)
	{
		pthread_mutex_lock(&shut_down_lock);
		bool notSoftRestart = gnShutdown;
		pthread_mutex_unlock(&shut_down_lock);
		
		if(!notSoftRestart)
		{
			char path[strlen(homeDir) + strlen("/init_neighbor_list")];
			memset(path, 0, strlen(homeDir) + strlen("/init_neighbor_list"));
			strcpy(path, homeDir);
			strcat(path, "/init_neighbor_list");
			if(fopen(path, "r"))
				remove(path);
		}
	}
	
	pthread_mutex_lock(&UOID_time_map_lock);
	UOID_time_map.clear();
	pthread_mutex_unlock(&UOID_time_map_lock);
		
	pthread_mutex_lock(&nb_map_lock);
	nb_map.clear();
	pthread_mutex_unlock(&nb_map_lock);
	
	pthread_mutex_lock(&routing_map_lock);
	routing_map.clear();
	pthread_mutex_unlock(&routing_map_lock);
	
	pthread_mutex_lock(&status_file_lock);
	issent_status = false;
	statusPorts.clear();
	pthread_mutex_unlock(&status_file_lock);
	
	pthread_mutex_lock(&check_lock);
	issent_check = false;
	pthread_mutex_unlock(&check_lock);
	
	pthread_mutex_lock(&id_map_lock);
	id_map.clear();
	pthread_mutex_unlock(&id_map_lock);
	
	pthread_mutex_lock(&dispatch_queue_lock);
	dispatch_exit = false;
	pthread_mutex_unlock(&dispatch_queue_lock);
	
	pthread_mutex_lock(&control_thread_lock);
	userShutdown = false;
	recv_thread_exit = false;
	all_thread_exit = false;
	pthread_mutex_unlock(&control_thread_lock);
	
	pthread_mutex_lock(&join_lock);
	join_success = false;
	join_shutdown = false;
	connect_fail_count = 0;
	join_sockets.clear();
	pthread_mutex_unlock(&join_lock);
	
	pthread_mutex_lock(&search_lock);
	search_exit = true;
	searchFileID_map.clear();
	searchFileSHA1_map.clear();
	searchFileName_map.clear();
	search_time_map.clear();
	search_done_map.clear();
	pthread_mutex_unlock(&search_lock);
	
	pthread_mutex_lock(&fileID_lock);
	fileIDIndex_map.clear();
	pthread_mutex_unlock(&fileID_lock);
	
	pthread_mutex_lock(&get_lock);
	getMsgFilename_map.clear();
	pthread_mutex_unlock(&get_lock);
	
	pthread_mutex_lock(&shut_down_lock);
	bool type = gnShutdown;
	pthread_mutex_unlock(&shut_down_lock);
	
	if(type)
		log->close_log(hostname, port, USER_SHUTDOWN);
	else
		log->close_log(hostname, port, SELF_RESTART);
	
	//close filesys	
	close_filesys(homeDir);	
		
	delete[] node_instance_id;	
}
