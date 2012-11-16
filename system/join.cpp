#include "node.h"

void* Node :: join_thread_helper(void* arg)
{
	NodeIndex* node_index = (NodeIndex *)arg;
	Node *node = node_index->node;
	int index = node_index->index;
	node->join_thread(index);
	pthread_exit((void*)0);
}

void Node :: join_thread(int index)
{
	void* tret = NULL;
	//temporary connection, join network and create init_neighbor_list file
	bool connected = false;
	//printf("%d, %s, %s\n", index, beaconNodes_hostname[index], beaconNodes_port[index]);
	int sockfd = client_socket_prepare(beaconNodes_hostname[index], beaconNodes_port[index], connected);
	if(!connected)
		return;
		
	//connect successful, add to neighbor_table
	#ifdef _DEBUG_
		printf("Connect join socket %d\n", sockfd);
	#endif	
	
	add_to_neighbor_table(sockfd, beaconNodes_hostname[index], beaconNodes_port[index]);	
	pthread_t join_recv_thread_id = 0;	
	pthread_t child_send_thread_id = 0;
	join_send_recv_thread(sockfd, child_send_thread_id, join_recv_thread_id);
	join_network(sockfd);
	
	int err = pthread_join(join_recv_thread_id, &tret);	
	if(err != 0)
	{
		printf("cannot join with join recv thread: %s\n", strerror(err));
		exit(1);
	}
	printf("servant:%s> Complete creating init_neighbor_list\nservant:%s> ", port, port);
	fflush(stdout);
}

void* Node :: join_recv_thread_helper(void* arg)
{
	RecvThread_arg* recv_arg = (RecvThread_arg*)arg;
	Node* node = recv_arg->node;
	int sockfd = recv_arg->sockfd;
	pthread_t send_thread_id = recv_arg->send_thread_id;	
	node->join_recv_thread(sockfd, send_thread_id);
	pthread_exit(arg);
}

void Node :: join_recv_thread(int sockfd, pthread_t send_thread_id)
{
	char header_buf[HEADER_LEN];
	memset(header_buf, 0, HEADER_LEN);
	char data_buf[MAXDATASIZE];
	memset(data_buf, 0, MAXDATASIZE);
	map<uint32_t, string> reply_map;
	//----------------------------------------------------
	//init variables for select system call
	struct timeval tv;
	tv.tv_sec = joinTimeout;
	tv.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	//----------------------------------------------------

	bool socket_close = false;
	int joinTimeout_time = currentTime() + joinTimeout;	
	while(true)
	{
		tv.tv_sec = joinTimeout_time - currentTime();
		memset(header_buf, 0, HEADER_LEN);
		int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);
		if(rv == -1)
		{
			#ifdef _DEBUG_
				perror("select");
				printf("Server: other node closes connection\n");
			#endif	
			break;
		}
		else if(rv == 0)
		{
			#ifdef _DEBUG_
				printf("Server: join time out in %d seconds\n", joinTimeout);
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
							printf("Server: client close socket to the server\n");
						#endif	
						socket_close= true;
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
		
		uint32_t dataLen = header.dataLen;
		char msg_data[dataLen];
		memset(msg_data, 0, dataLen);
		if(dataLen != 0)
		{
			//receive payload in the message, buffer size equals MAXDATASIZE
			int last_time = dataLen % MAXDATASIZE;	
		
			memset(data_buf, 0, MAXDATASIZE);
			tv.tv_sec = joinTimeout_time - currentTime();
			rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);
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
					printf("Server: join time out in %d seconds\n", joinTimeout);
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
								printf("Server: client close socket to the server\n");
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
			memcpy(msg_data, data_buf, last_time);
		}
		
		//only handle joinResMsg when joining the network
		if(header.msgType == JNRS)
		{
			JoinResMsg *joinResMsg = new JoinResMsg();
			joinResMsg->create_header(header);
			joinResMsg->parse_payload(msg_data);
			string s = get_neighbor(sockfd);
			log->write_to_file('r', s.c_str(), joinResMsg);
			char host[strlen(joinResMsg->hostname) + 7];
			memset(host, 0, strlen(joinResMsg->hostname) + 7);
			sprintf(host, "%s:%d", joinResMsg->hostname, joinResMsg->host_port);
			#ifdef _DEBUG_
				printf("distance: %d, host: %s\n", joinResMsg->distance, host);
			#endif	
			if(reply_map.find(joinResMsg->distance) == reply_map.end())
				reply_map.insert(pair<uint32_t, string>(joinResMsg->distance, host));	
		}
	}
	
	//joinTimeout, create init_neighbor_list
	if((int)reply_map.size() < initNeighbors)
	{
		printf("Error: There are %d neighbors, not enough initNeighbors %d.\n", (int)reply_map.size(), initNeighbors);
		pthread_mutex_lock(&join_lock);
		join_shutdown = true;
		pthread_mutex_unlock(&join_lock);
	}
	else
	{
		int len = strlen(homeDir) + strlen("/init_neighbor_list");
		char path[len];
		memset(path, 0, len);
		strcpy(path, homeDir);
		strcat(path, "/init_neighbor_list");
		FILE * pFile = fopen(path, "w+");
		map<uint32_t, string>::iterator it = reply_map.begin();
		for(int i = 0; i < initNeighbors; i++)
		{	
			fprintf (pFile, "%s\n", (it->second).c_str());
			fflush(pFile);
			//printf("neighbor: %s\n", (it->second).c_str());
			++it;
		}
		fclose(pFile);
		//reply_map.clear();
		//connection to this beacon node successful
		pthread_mutex_lock(&join_lock);
		join_success = true;
		pthread_mutex_unlock(&join_lock);	
	}
	
	//close corresponding send thread
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
	#ifdef _DEBUG_
		printf("Connect join socket %d exit, Now nb_map size: %d\n", sockfd, size);	
	#endif
	close(sockfd);
}

void Node :: join_send_recv_thread(int sockfd, pthread_t& child_send_thread_id, pthread_t& join_recv_thread_id)
{
	SendThread_arg* send_arg = new SendThread_arg();
	send_arg->node = this;
	send_arg->sockfd = sockfd;	
	
	//put send thread info thread_id and resource into id_map 	
	pthread_mutex_lock(&id_map_lock);
	SendThread_Resourse *resource = new SendThread_Resourse();
	id_map.insert(pair<int, SendThread_Resourse*>(sockfd, resource));
	pthread_mutex_unlock(&id_map_lock);

	int err = pthread_create(&child_send_thread_id, NULL, child_send_thread_helper, (void*)send_arg);
	if(err != 0)
	{
		printf("cannot create as child send thread: %s\n", strerror(err));
		exit(1);
	}
	
	//fork join recv thread
	RecvThread_arg* recv_arg = new RecvThread_arg();
	recv_arg->node = this;
	recv_arg->sockfd = sockfd;
	recv_arg->send_thread_id = child_send_thread_id;
	err = pthread_create(&join_recv_thread_id, NULL, join_recv_thread_helper, (void*)recv_arg);
	if(err != 0)
	{
		printf("cannot create as join recv thread: %s\n", strerror(err));
		exit(1);
	}
}

void Node :: join_network(int sockfd)
{
	JoinMsg* joinMsg = create_joinMsg();
	DispatchElement* dispatchElm = new DispatchElement(joinMsg, sockfd, SEND_MSG);	
	push_dispatch_queue(dispatchElm);
}

void Node :: add_to_join_socket(int sockfd)
{
	bool isJoin = false;	
	pthread_mutex_lock(&nb_map_lock);
	if(nb_map.find(sockfd) == nb_map.end())
		isJoin = true;
	pthread_mutex_unlock(&nb_map_lock);
	
	if(isJoin)	//this is a join process from non beacon node
	{
		pthread_mutex_lock(&join_lock);
		join_sockets.push_back(sockfd);
		pthread_mutex_unlock(&join_lock);	
	}	
}
	
