#include "node.h"

void Node:: create_storeMsg(int storeFile_index, unsigned char store_TTL)
{	
	StoreMsg* storeMsg = new StoreMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	storeMsg->create_header(STOR, UOID, store_TTL);
	storeMsg->create_payload_data((uint32_t)storeFile_index, homeDir);	
	
	//based on send probabilities to decide if store message needs to be sent out
	send_prob_all_neighbors_msg(storeMsg);
}

void Node:: create_storeMsg(int sockfd, uint32_t storeFile_index, unsigned char store_TTL, char* msg_UOID)
{
	StoreMsg* storeMsg = new StoreMsg();	
	storeMsg->create_header(STOR, msg_UOID, store_TTL);
	storeMsg->create_payload_data(storeFile_index, homeDir);	
	
	//based on send probabilities to decide if store message needs to be forwarded
	forward_prob_other_neighbors_msg(storeMsg, sockfd);
}

void Node:: create_searchMsg(unsigned char searchType, string searchQuery, char* searchMsg_UOID)
{
	SearchMsg* searchMsg = new SearchMsg();
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, searchMsg_UOID, UOID_LEN);
	searchMsg->create_header(SHRQ, searchMsg_UOID, TTL);
	searchMsg->create_payload_data(searchType, searchQuery);	
	#ifdef DEBUG
		printf("searchType: %d, searchQuery: %s\n", searchType, searchQuery.c_str());
	#endif
	pthread_mutex_lock(&search_lock);
	search_exit = false;
	searchFileID_map.clear();
	searchFileSHA1_map.clear();
	searchFileName_map.clear();
	std::queue<SearchRes_Elm> empty;
	std::swap(searchResponse_queue, empty);
	UOID_Map msg_UOID((unsigned char*)searchMsg_UOID);
	search_time_map.insert( pair<UOID_Map, int>(msg_UOID, currentTime()) );
	search_done_map.insert( pair<UOID_Map, bool>(msg_UOID, false) ); 
	pthread_mutex_unlock(&search_lock);
	
	send_all_neighbors_msg(searchMsg);
}

SearchResMsg* Node:: create_searchResMsg(char* search_UOID, vector<uint32_t> matchFiles)
{
	SearchResMsg *searchResMsg = new SearchResMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	searchResMsg->create_header(SHRS, UOID, TTL);
	searchResMsg->create_payload_data(search_UOID);
	
	for(size_t i = 0; i < matchFiles.size(); ++i)
	{
		char FileID[UOID_LEN];
		char obj[] = "file";
		GetUOID(node_instance_id, obj, FileID, UOID_LEN);
		
		#ifdef DEBUG
			printf("match FileID => index:\n");
			for(int j = 0; j < UOID_LEN; ++j)
				printf("%02x", (unsigned char)FileID[j]);	
			printf("=>%d\n", matchFiles[i]);	
		#endif
				
		UOID_Map fileID_uoid((unsigned char*)FileID);
		pthread_mutex_lock(&fileID_lock);
		fileIDIndex_map.insert( pair<UOID_Map, uint32_t>(fileID_uoid, matchFiles[i]) );
		pthread_mutex_unlock(&fileID_lock);
		
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, matchFiles[i]);
		uint32_t matchFileSize = getFilesize(metaDataFilename);
		
		searchResMsg->header.dataLen += 4;
		searchResMsg->header.dataLen += 20;
		searchResMsg->header.dataLen += matchFileSize;	
		searchResMsg->matchFile_size.push_back(matchFileSize);
		searchResMsg->matchFileID.push_back(fileID_uoid);
		searchResMsg->matchFile_index.push_back(matchFiles[i]);
	}
	
	return searchResMsg;
}

void Node:: create_getMsg(unsigned char* fileID, unsigned char* fileSHA1, string getFilename, bool isReplace)
{
	GetMsg* getMsg = new GetMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	getMsg->create_header(GTRQ, UOID, TTL);
	getMsg->create_payload_data(fileID, fileSHA1);	
	
	UOID_Map get_uoid((unsigned char*)UOID);
	pthread_mutex_lock(&get_lock);
	getMsgFilename_map.insert( pair<UOID_Map, string>(get_uoid, getFilename) );
	getReplace_map.insert( pair<UOID_Map, bool>(get_uoid, isReplace) );
	pthread_mutex_unlock(&get_lock);
	
	send_all_neighbors_msg(getMsg);
}

GetResMsg* Node:: create_getResMsg(char* get_UOID, uint32_t fileIndex)
{
	GetResMsg* getResMsg = new GetResMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	getResMsg->create_header(GTRS, UOID, TTL);
	
	char metaDataFilename[256];
	memset(metaDataFilename, 0, 256);
	create_metaDataFilename(metaDataFilename, homeDir, fileIndex);
	uint32_t metaData_fileSize = getFilesize(metaDataFilename);
	getResMsg->create_payload_data(get_UOID, fileIndex, metaData_fileSize, homeDir);
	
	return getResMsg;
}

void Node:: create_deleteMsg(char* deleteFilename, unsigned char* deleteSHA1, unsigned char* deleteNonce, unsigned char* deletePass)
{
	DeleteMsg* deleteMsg = new DeleteMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	deleteMsg->create_header(DELT, UOID, TTL);
	deleteMsg->create_payload_data(deleteFilename, deleteSHA1, deleteNonce, deletePass);	
	
	send_all_neighbors_msg(deleteMsg);
}

int Node:: recv_metaDataFile(int sockfd, char* filename, uint32_t fileSize, bool recv_before)
{
	bool socket_close = false;
	bool diskFull = false;
	//----------------------------------------------------
	//init variables for select system call
	struct timeval tv;
	tv.tv_sec = keepAliveTimeout;
	tv.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	//----------------------------------------------------
	
	char buffer[fileSize];
	memset(buffer, 0, fileSize);
	
	FILE* pf;
	if(!recv_before)
	{
		if(filename == NULL)
			assert(false);
		
		pf = fopen(filename, "a+");
		if(!pf)
		{
			fprintf(stderr, "cannot create file %s\n", filename);
			return -1;
		}	
	}
	
	int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);	
	if(rv == -1)
	{
		#ifdef DEBUG
			perror("select");
			printf("Server: other node closes conncection\n");
		#endif
		return -1;
	}
	else if(rv == 0)
	{
		#ifdef DEBUG
			printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
		#endif
		return -1;
	}
	else
	{
		if(FD_ISSET(sockfd, &readfds))
		{
			uint32_t recvLen = 0;
			while(recvLen != fileSize)
			{
				int status = recv(sockfd, buffer + recvLen, fileSize, 0);
				if(status == -1)
				{
					#ifdef DEBUG
						perror("recv");
					#endif	
					socket_close = true;
					break;
				}	
				else if(status == 0)
				{
					#ifdef DEBUG
						printf("Server: client close socket to the server, payload\n");
					#endif
					socket_close = true;
					break;
				}
				recvLen += (uint32_t)status;
			}	
			if(socket_close)
				return -1;
			//srecv_data_len++;
		}
	}
	if(!recv_before)	//not receive before, write to filesys
	{
		size_t num = fwrite(buffer, 1, fileSize, pf);
		if(num != fileSize)
		{
			fprintf(stderr, "Error: not enough room in the filesys to store the file\n");
			diskFull = true;
		}
	}

	if(!recv_before && fclose(pf) == EOF)
	{
		fprintf(stderr, "Error: not enough room in the filesys to store the file\n");
		diskFull = true;
	}
	
	if(diskFull)
		return -2;
		
	return 0;
}

int Node:: recv_fileData(int sockfd, char* filename, uint32_t fileSize, bool recv_before, const unsigned char* metaDataSHA1)
{
	bool socket_close = false;
	bool diskFull = false;
	SHA_CTX c;
	memset(&c, 0, sizeof(SHA_CTX));
	//----------------------------------------------------
	//init variables for select system call
	struct timeval tv;
	tv.tv_sec = keepAliveTimeout;
	tv.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	//----------------------------------------------------
	
	char buffer[MAX_BUF_SIZE];
	memset(buffer, 0, MAX_BUF_SIZE);	
	SHA1_Init(&c);
	unsigned char recvFileSHA1[SHA_DIGEST_LENGTH];
	memset(recvFileSHA1, 0, SHA_DIGEST_LENGTH);		
	FILE* pf;
	
	if(!recv_before)
	{
		if(filename == NULL)
			assert(false);
		
		pf = fopen(filename, "w+");
		if(!pf)
		{
			fprintf(stderr, "cannot create file %s\n", filename);
			return -1;
		}	
	}
	
	uint32_t normal_time = fileSize / MAX_BUF_SIZE;
	uint32_t last_time = fileSize % MAX_BUF_SIZE;	
	
	for(uint32_t i = 0; i < normal_time; i++)
	{
		memset(buffer, 0, MAX_BUF_SIZE);	
		//set keepAliveTimeout seconds the same as header
		int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);		
		if(rv == -1)
		{
			#ifdef DEBUG
				perror("select");
				printf("Server: other node closes conncection\n");
			#endif	
			return -1;
		}
		else if(rv == 0)
		{
			#ifdef DEBUG
				printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
			#endif
			return -1;
		}
		else
		{
			if(FD_ISSET(sockfd, &readfds))
			{
				uint32_t recvLen = 0;
				while(recvLen != MAX_BUF_SIZE)
				{
					int status = recv(sockfd, buffer + recvLen, MAX_BUF_SIZE, 0);
					if(status == -1)
					{
						#ifdef DEBUG
							perror("recv");
						#endif	
						socket_close = true;
						break;
					}	
					else if(status == 0)
					{
						#ifdef DEBUG
							printf("Server: client close socket to the server, payload\n");
						#endif
						socket_close = true;
						break;
					}
					recvLen += (uint32_t)status;
				}
				if(socket_close)
					return -1;
				//recv_data_len++;
			}
		}
		if(!recv_before)	//not receive before, write to filesys
		{
			size_t num = fwrite(buffer, 1, MAX_BUF_SIZE, pf);
			SHA1_Update(&c, buffer, MAX_BUF_SIZE);
			if(num != MAX_BUF_SIZE)
			{
				fprintf(stderr, "Error: not enough room in the filesys to store the file\n");
				diskFull = true;
			}
		}
	}

	memset(buffer, 0, MAX_BUF_SIZE);
	int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);	
	if(rv == -1)
	{
		#ifdef DEBUG
			perror("select");
			printf("Server: other node closes conncection\n");
		#endif
		return -1;
	}
	else if(rv == 0)
	{
		#ifdef DEBUG
			printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
		#endif
		return -1;
	}
	else
	{
		if(FD_ISSET(sockfd, &readfds))
		{
			uint32_t recvLen = 0;
			while(recvLen != last_time)
			{
				int status = recv(sockfd, buffer + recvLen, last_time, 0);
				if(status == -1)
				{
					#ifdef DEBUG
						perror("recv");
					#endif	
					socket_close = true;
					break;
				}	
				else if(status == 0)
				{
					#ifdef DEBUG
						printf("Server: client close socket to the server, payload\n");
					#endif
					socket_close = true;
					break;
				}
				recvLen += (uint32_t)status;
			}	
			if(socket_close)
				return -1;
			//srecv_data_len++;
		}
	}
	if(!recv_before)	//not receive before, write to filesys
	{
		size_t num = fwrite(buffer, 1, last_time, pf);
		SHA1_Update(&c, buffer, last_time);
		if(num != last_time)
		{
			fprintf(stderr, "Error: not enough room in the filesys to store the file\n");
			diskFull = true;
		}
	}
	
	if(!recv_before)
	{
		SHA1_Final(recvFileSHA1, &c);
		if(memcmp(recvFileSHA1, metaDataSHA1, SHA_DIGEST_LENGTH))	//not the same
		{
			printf("Receiving file SHA1 is not the same as metaData file SHA1\n");
			return -3;	
		}
	}
	
	if(!recv_before && fclose(pf) == EOF)
	{
		fprintf(stderr, "Error: not enough room in the filesys to store the file\n");
		diskFull = true;
	}
	
	if(diskFull)
		return -2;
		
	return 0;
}

int Node:: recv_givenLenData(int sockfd, void* data_buf, uint32_t protocol_len)
{
	bool socket_close = false;
	//----------------------------------------------------
	//init variables for select system call
	struct timeval tv;
	tv.tv_sec = keepAliveTimeout;
	tv.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	//----------------------------------------------------
	
	char buffer[protocol_len];
	memset(buffer, 0, protocol_len);
	
	int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);	
	if(rv == -1)
	{
		#ifdef DEBUG
			perror("select");
			printf("Server: other node closes conncection\n");
		#endif
		return -1;
	}
	else if(rv == 0)
	{
		#ifdef DEBUG
			printf("Server: receive no data in %d seconds, time out\n", keepAliveTimeout);
		#endif
		return -1;
	}
	else
	{
		if(FD_ISSET(sockfd, &readfds))
		{
			uint32_t recvLen = 0;
			while(recvLen != protocol_len)
			{
				int status = recv(sockfd, buffer + recvLen, protocol_len, 0);
				if(status == -1)
				{
					#ifdef DEBUG
						perror("recv");
					#endif	
					socket_close = true;
					break;
				}	
				else if(status == 0)
				{
					#ifdef DEBUG
						printf("Server: client close socket to the server, payload\n");
					#endif
					socket_close = true;
					break;
				}
				recvLen += (uint32_t)status;
			}	
			if(socket_close)
				return -1;
			//srecv_data_len++;
		}
	}	
	memcpy(data_buf, buffer, protocol_len);
	return 0;
}

void Node:: send_prob_all_neighbors_msg(Msg* msg)
{
	if(msg->header.TTL == 0)
		return;

	vector<int> neighbor_sockfds;
	pthread_mutex_lock(&nb_map_lock);
	map<int, Neighbor_node*>:: iterator iter;
	for(iter = nb_map.begin(); iter != nb_map.end(); ++iter)
		neighbor_sockfds.push_back(iter->first);	
	pthread_mutex_unlock(&nb_map_lock);	
	
	for(int i = 0; i < (int)neighbor_sockfds.size(); ++i)
	{	
		if(drand48() < neighborStoreProb)
		{
			DispatchElement* dispatchElm = new DispatchElement(msg, neighbor_sockfds[i], SEND_MSG);	
			//put the reply msg into the dispatch queue
			push_dispatch_queue(dispatchElm);
		}	
	}
}

void Node:: forward_prob_other_neighbors_msg(Msg* msg, int sockfd)
{
	if(msg->header.TTL == 0)
		return;
		
	vector<int> neighbor_sockfds;
	pthread_mutex_lock(&nb_map_lock);
	map<int, Neighbor_node*>:: iterator iter;
	for(iter = nb_map.begin(); iter != nb_map.end(); ++iter)
	{
		if(sockfd != iter->first)
			neighbor_sockfds.push_back(iter->first);
	}	
	pthread_mutex_unlock(&nb_map_lock);	
	
	for(int i = 0; i < (int)neighbor_sockfds.size(); ++i)
	{	
		if(drand48() < neighborStoreProb)
		{
			DispatchElement* dispatchElm = new DispatchElement(msg, neighbor_sockfds[i], FORWARD_MSG);	
			//put the reply msg into the dispatch queue
			push_dispatch_queue(dispatchElm);	
		}
	}
}

bool Node:: handle_recv_storeMsg(int sockfd, MsgHeader header)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), NULL, 0, header);

	//this message has seen before or not
	pthread_mutex_lock(&UOID_time_map_lock);
	bool recv_before = false;
	map<UOID_Map, int>::iterator it;
	for(it = UOID_time_map.begin(); it != UOID_time_map.end(); ++it)
	{
		if(!memcmp(it->first.m_UOID, header.UOID, UOID_LEN))
		{
			recv_before = true;
			break;
		}	
	}	
	if(!recv_before)
	{
		UOID_Map time_map_UOID((unsigned char*)header.UOID);
		UOID_time_map.insert(pair<UOID_Map, int>(time_map_UOID, currentTime()));
	}
	pthread_mutex_unlock(&UOID_time_map_lock);	
	
	uint32_t msg_metaData_fileSize = 0;
	int rv = recv_givenLenData(sockfd, &msg_metaData_fileSize, 4);
	if(rv == -1)
		return true;
	uint32_t metaData_fileSize = ntohl(msg_metaData_fileSize);
	
	int rv1 = 0;
	int rv2 = 0;
	if(!recv_before)
	{
		pthread_mutex_lock(&filesys_lock);
		uint32_t tmpFileIndex = filesys_index;
		++filesys_index;
		pthread_mutex_unlock(&filesys_lock);
	
		//receive metadata
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, tmpFileIndex);
		rv1 = recv_metaDataFile(sockfd, metaDataFilename, metaData_fileSize, recv_before);
		if(rv1 == -1)
			return true;
		
		unsigned char metaDataSHA1[SHA_DIGEST_LENGTH];
		memset(metaDataSHA1, 0, SHA_DIGEST_LENGTH); 
		getSHA1_metaData(metaDataFilename, metaDataSHA1);
				
		//receive actual file
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, tmpFileIndex);
		uint32_t data_fileSize = header.dataLen - 4 - metaData_fileSize;
		rv2 = recv_fileData(sockfd, dataFilename, data_fileSize, recv_before, metaDataSHA1);
		if(rv2 == -1)
			return true;
		
		bool isCacheFile = false;
		if(!rv1 && !rv2 && drand48() < storeProb) //store into cache
		{
			//call cache file algorithm
			isCacheFile = doCacheFile(homeDir, data_fileSize, cacheSize, tmpFileIndex);
		}
		
		if(!isCacheFile)	//do not need to cache this file
		{
			//delete this temp file from filesys on disk
			deleteDiskFile(metaDataFilename, dataFilename);
		}
		else
		{					
			//send this file to other neighbors if needed
			if(header.TTL == 0)
				return false;
			create_storeMsg(sockfd, tmpFileIndex, header.TTL, header.UOID);
		}
	}
	else	//file received before
	{
		#ifdef DEBUG
			printf("Server: store message received before\n");
		#endif	
		rv1 = recv_metaDataFile(sockfd, NULL, metaData_fileSize, recv_before);
		if(rv1 == -1)
			return true;
		
		uint32_t data_fileSize = header.dataLen - 4 - metaData_fileSize;
		rv2 = recv_fileData(sockfd, NULL, data_fileSize, recv_before, NULL);
		if(rv2 == -1)
			return true;
	}
		
	return false;	
}

void Node:: handle_searchMsg(SearchMsg *searchMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), searchMsg);
	
	//search filesys to find if there is a match
	vector<uint32_t> matchFiles = search_filesys(searchMsg->searchType, searchMsg->searchQuery);
	
	//there is a match, send search response message back
	if(matchFiles.size() > 0)
	{
		SearchResMsg* searchResMsg = create_searchResMsg(searchMsg->header.UOID, matchFiles);	
		DispatchElement* dispatchElm = new DispatchElement(searchResMsg, sockfd, SEND_MSG);	
		push_dispatch_queue(dispatchElm);
	}
	
	//add to routing table for forwarding other node's searchResMsg
	if(searchMsg->header.TTL == 0)
		return;
	
	pthread_mutex_lock(&routing_map_lock);
	UOID_Map routing_map_UOID((unsigned char*)searchMsg->header.UOID);
	routing_map.insert(pair<UOID_Map, int>(routing_map_UOID, sockfd));
	pthread_mutex_unlock(&routing_map_lock);
	
	//this message needs to be flooded				
	flood_msg(searchMsg, sockfd);
}

bool Node:: handle_recv_searchResMsg(int sockfd, MsgHeader header)
{
	//this message has seen before or not
	pthread_mutex_lock(&UOID_time_map_lock);
	bool recv_before = false;
	map<UOID_Map, int>::iterator it;
	for(it = UOID_time_map.begin(); it != UOID_time_map.end(); ++it)
	{
		if(!memcmp(it->first.m_UOID, header.UOID, UOID_LEN))
		{
			recv_before = true;
			break;
		}	
	}	
	if(!recv_before)
	{
		UOID_Map time_map_UOID((unsigned char*)header.UOID);
		UOID_time_map.insert(pair<UOID_Map, int>(time_map_UOID, currentTime()));
	}
	pthread_mutex_unlock(&UOID_time_map_lock);		
	
	char msg_data_UOID[UOID_LEN];
	memset(msg_data_UOID, 0, UOID_LEN);
	int rv = recv_givenLenData(sockfd, msg_data_UOID, UOID_LEN);
	if(rv == -1)
		return true;
	
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), (unsigned char*)msg_data_UOID, 4, header);
	
	SearchResMsg *searchResMsg = new SearchResMsg();
	searchResMsg->create_header(SHRS, header.UOID, header.TTL);
	searchResMsg->create_payload_data(msg_data_UOID);
	
	//forward searchResMsg if needed	
	bool isForwardSearchRes = false;
	int forwardsockfd = 0;
	pthread_mutex_lock(&routing_map_lock);
	map<UOID_Map, int>::iterator iter;
	for(iter = routing_map.begin(); iter != routing_map.end(); ++iter)
	{	
		if(!memcmp(iter->first.m_UOID, searchResMsg->UOID, UOID_LEN))
		{
			isForwardSearchRes = true;
			forwardsockfd = iter->second;
			break;
		}	
	}	
	pthread_mutex_unlock(&routing_map_lock);
	
	//store the metadata file in tempMeta directory in advance 
	uint32_t temp_len = 20;
	uint32_t start = 0;
	while((temp_len + start) != header.dataLen)
	{
		uint32_t msg_fileSize = 0;
		rv = recv_givenLenData(sockfd, &msg_fileSize, 4);
		if(rv == -1)
			return true;
		uint32_t fileSize = ntohl(msg_fileSize);
		start += 4;
		
		char msg_fileID[UOID_LEN];
		memset(msg_fileID, 0, UOID_LEN);
		int rv = recv_givenLenData(sockfd, msg_fileID, UOID_LEN);
		if(rv == -1)
			return true;	
		start += 20;	
		
		searchResMsg->header.dataLen += 4;
		searchResMsg->header.dataLen += 20;
		searchResMsg->header.dataLen += fileSize;
		searchResMsg->matchFile_size.push_back(fileSize);
		UOID_Map uoid_file((unsigned char*)msg_fileID);
		searchResMsg->matchFileID.push_back(uoid_file);
		
		pthread_mutex_lock(&tempMeta_lock);
		uint32_t tempMetaDataFileIndex = tempMata_index;
		tempMata_index++;
		pthread_mutex_unlock(&tempMeta_lock);
		
		char tempMetaDataFileName[256];
		memset(tempMetaDataFileName, 0, 256);
		create_tempMetaDataFilename(tempMetaDataFileName, homeDir, tempMetaDataFileIndex);
		searchResMsg->matchFile_index.push_back(tempMetaDataFileIndex);
		rv = recv_metaDataFile(sockfd, tempMetaDataFileName, fileSize, recv_before);
		if(rv == -1)
			return true;
				
		start += fileSize;
	}
	
	if(!isForwardSearchRes)	//not in routing table
	{
		UOID_Map search_uoid((unsigned char*)searchResMsg->UOID);
		pthread_mutex_lock(&search_lock);
		map<UOID_Map, bool>::iterator iter = search_done_map.find(search_uoid);
		if(iter != search_done_map.end() && !iter->second)
		{
			for(size_t i = 0; i < searchResMsg->matchFile_index.size(); ++i)
			{
				SearchRes_Elm searchRes_elm(searchResMsg->matchFile_index[i], searchResMsg->matchFileID[i]);
				searchResponse_queue.push(searchRes_elm);
			}
			pthread_cond_signal(&searchResponse_queue_cv);
		}
		else
		{
			for(size_t i = 0; i < searchResMsg->matchFile_index.size(); ++i)
			{
				char tempMetaDataFileName[256];
				memset(tempMetaDataFileName, 0, 256);
				create_tempMetaDataFilename(tempMetaDataFileName, homeDir, searchResMsg->matchFile_index[i]);
				remove(tempMetaDataFileName);
			}
		}
		pthread_mutex_unlock(&search_lock);
	}
	else	//in routing table
	{		
		if(searchResMsg->header.TTL == 0)
			return false;
			
		searchResMsg->fileState = true;	
		DispatchElement* dispatchElm = new DispatchElement(searchResMsg, forwardsockfd, FORWARD_MSG);	
		push_dispatch_queue(dispatchElm);
	}
	
	return false;
}

void Node:: show_searchResponse(unsigned char searchType, string searchQuery, char* searchMsg_UOID)
{
	vector<uint32_t> searchResults = search_filesys(searchType, searchQuery);
	
	uint32_t searchIndex = 1;
	for(size_t i = 0; i < searchResults.size(); ++i)
	{
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, searchResults[i]);
		
		char FileID[UOID_LEN];
		char obj[] = "file";
		GetUOID(node_instance_id, obj, FileID, UOID_LEN);
		
		unsigned char searchFileSHA1[SHA_DIGEST_LENGTH];
		memset(searchFileSHA1, 0, SHA_DIGEST_LENGTH);
		string realFilename = print_search_metaDataFile(metaDataFilename, searchIndex, FileID, searchFileSHA1);
		
		pthread_mutex_lock(&search_lock);
		UOID_Map file_uoid((unsigned char*)FileID);
		searchFileID_map.insert( pair<uint32_t, UOID_Map>(searchIndex, file_uoid) );
		UOID_Map sha1_uoid(searchFileSHA1);
		searchFileSHA1_map.insert( pair<uint32_t, UOID_Map>(searchIndex, sha1_uoid) );
		searchFileName_map.insert( pair<uint32_t, string>(searchIndex, realFilename) );
		pthread_mutex_unlock(&search_lock);
		
		pthread_mutex_lock(&fileID_lock);
		fileIDIndex_map.insert( pair<UOID_Map, uint32_t>(file_uoid, searchResults[i]) );
		pthread_mutex_unlock(&fileID_lock);
		
		++searchIndex;
	}
	
	//wait for search response	
	while(true)
	{
		pthread_mutex_lock(&search_lock);
		while((int)searchResponse_queue.size() == 0 && !search_exit)
			pthread_cond_wait(&searchResponse_queue_cv, &search_lock);		
		
		if(search_exit)
		{
			std::queue<SearchRes_Elm> empty;
			std::swap(searchResponse_queue, empty);
			UOID_Map search_msg_uoid((unsigned char*)searchMsg_UOID);
			#ifdef _DEBUG_
				printf("before: %d\n", search_done_map[search_msg_uoid]);
			#endif	
			search_done_map[search_msg_uoid] = true;
			#ifdef _DEBUG_
				printf("after: %d\n", search_done_map[search_msg_uoid]);
				printf("queue size: %d\n", searchResponse_queue.size());
			#endif
			pthread_mutex_unlock(&search_lock);
			break;
		}	
		SearchRes_Elm searchRes_elm = searchResponse_queue.front();
		uint32_t tempMata_index = searchRes_elm.tempMataIndex;
		UOID_Map searchRes_FileID = searchRes_elm.fileID_uoid;
		searchResponse_queue.pop();
		pthread_mutex_unlock(&search_lock);	
		
		char tempMetaDataFileName[256];
		memset(tempMetaDataFileName, 0, 256);
		create_tempMetaDataFilename(tempMetaDataFileName, homeDir, tempMata_index);
		
		unsigned char searchFileSHA1[SHA_DIGEST_LENGTH];
		memset(searchFileSHA1, 0, SHA_DIGEST_LENGTH);
		string realFilename = print_search_metaDataFile(tempMetaDataFileName, searchIndex, (char*)searchRes_FileID.m_UOID, searchFileSHA1);
		
		pthread_mutex_lock(&search_lock);		
		searchFileID_map.insert( pair<uint32_t, UOID_Map>(searchIndex, searchRes_FileID) );
		UOID_Map sha1_uoid(searchFileSHA1);
		searchFileSHA1_map.insert( pair<uint32_t, UOID_Map>(searchIndex, sha1_uoid) );
		searchFileName_map.insert( pair<uint32_t, string>(searchIndex, realFilename) );
		pthread_mutex_unlock(&search_lock);
		
		remove(tempMetaDataFileName);
		++searchIndex;
	}
	
	#ifdef DEBUG
		pthread_mutex_lock(&search_lock);
		for(map<uint32_t, UOID_Map>::iterator iter = searchFileID_map.begin(); iter != searchFileID_map.end(); ++iter)
		{
			printf("%d=>", iter->first);
			for(int i = 0; i < UOID_LEN; i++)
				printf("%02x", iter->second.m_UOID[i]);
			printf("\tFilename: %s\tSHA1: ", searchFileName_map[iter->first].c_str());
			for(int i = 0; i < UOID_LEN; i++)
				printf("%02x", searchFileSHA1_map[iter->first].m_UOID[i]);	
			printf("\n");	
		}
		pthread_mutex_unlock(&search_lock);	
	#endif
}

void Node:: handle_getMsg(GetMsg *getMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), getMsg);
	
	bool isGetFileExist = false;
	uint32_t fileIndex = 0;
	pthread_mutex_lock(&fileID_lock);
	UOID_Map fileID_uoid(getMsg->FileID);
	map<UOID_Map, uint32_t>::iterator iter = fileIDIndex_map.find(fileID_uoid);
	if( iter != fileIDIndex_map.end())
	{
		fileIndex = iter->second;
		isGetFileExist = true;
		#ifdef DEBUG
			for(int i = 0; i < UOID_LEN; i++)
				printf("%02x", iter->first.m_UOID[i]);
			printf("=%d is in port:%s file system\n", fileIndex, port);
		#endif
	}
	pthread_mutex_unlock(&fileID_lock);
	
	if(isGetFileExist)	//get file is in my file system, raise get response message
	{
		GetResMsg* getResMsg = create_getResMsg(getMsg->header.UOID, fileIndex);
		DispatchElement* dispatchElm = new DispatchElement(getResMsg, sockfd, SEND_MSG);	
		push_dispatch_queue(dispatchElm);
	}
	
	//add to routing table for forwarding other node's getResMsg
	if(getMsg->header.TTL == 0)
		return;
	
	pthread_mutex_lock(&routing_map_lock);
	UOID_Map routing_map_UOID((unsigned char*)getMsg->header.UOID);
	routing_map.insert( pair<UOID_Map, int>(routing_map_UOID, sockfd) );
	pthread_mutex_unlock(&routing_map_lock);
	
	//this message needs to be flooded				
	flood_msg(getMsg, sockfd);
}

bool Node:: handle_recv_getResMsg(int sockfd, MsgHeader header)
{
	//this message has seen before or not
	pthread_mutex_lock(&UOID_time_map_lock);
	bool recv_before = false;
	UOID_Map uoid((unsigned char*)header.UOID);
	map<UOID_Map, int>::iterator it = UOID_time_map.find(uoid);
	if(it != UOID_time_map.end())
		recv_before = true;
	if(!recv_before)
	{
		UOID_Map time_map_UOID((unsigned char*)header.UOID);
		UOID_time_map.insert(pair<UOID_Map, int>(time_map_UOID, currentTime()));
	}
	pthread_mutex_unlock(&UOID_time_map_lock);	

	char msg_data_UOID[UOID_LEN];
	memset(msg_data_UOID, 0, UOID_LEN);
	int rv = recv_givenLenData(sockfd, msg_data_UOID, UOID_LEN);
	if(rv == -1)
		return true;
		
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), (unsigned char*)msg_data_UOID, 4, header);
	
	GetResMsg *getResMsg = new GetResMsg();
	getResMsg->create_header(GTRS, header.UOID, header.TTL);
	
	uint32_t msg_metaData_fileSize = 0;
	rv = recv_givenLenData(sockfd, &msg_metaData_fileSize, 4);
	if(rv == -1)
		return true;
	uint32_t metaData_fileSize = ntohl(msg_metaData_fileSize);

	int rv1 = 0;
	int rv2 = 0;
	if(!recv_before)
	{
		pthread_mutex_lock(&filesys_lock);
		uint32_t tmpFileIndex = filesys_index;
		++filesys_index;
		pthread_mutex_unlock(&filesys_lock);
	
		//receive metadata
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, tmpFileIndex);
		rv1 = recv_metaDataFile(sockfd, metaDataFilename, metaData_fileSize, recv_before);
		if(rv1 == -1)
			return true;
		
		unsigned char metaDataSHA1[SHA_DIGEST_LENGTH];
		memset(metaDataSHA1, 0, SHA_DIGEST_LENGTH); 
		getSHA1_metaData(metaDataFilename, metaDataSHA1);
				
		//receive actual file
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, tmpFileIndex);
		uint32_t data_fileSize = header.dataLen - 4 - metaData_fileSize;
		rv2 = recv_fileData(sockfd, dataFilename, data_fileSize, recv_before, metaDataSHA1);
		if(rv2 == -1)
			return true;
				
		getResMsg->create_payload_data(msg_data_UOID, tmpFileIndex, metaData_fileSize, homeDir);	
		
		if(rv1 || rv2)
		{
			deleteDiskFile(metaDataFilename, dataFilename);
			return false;
		}	
		
		//forward statusResMsg
		bool isForwardGetRes = false;
		int forwardsockfd = 0;
		UOID_Map get_uoid((unsigned char*)getResMsg->UOID);
		pthread_mutex_lock(&routing_map_lock);
		map<UOID_Map, int>::iterator it = routing_map.find(get_uoid);
		if( it != routing_map.end() )
		{
			isForwardGetRes = true;
			forwardsockfd = it->second;
		}	
		pthread_mutex_unlock(&routing_map_lock);
		
		//--------------------------------------------------------------------------------
		//not sure, this file is already stored, therefore no need to store it again
		bool isStored = false;
		unsigned char metaDataNonce[UOID_LEN];
		memset(metaDataNonce, 0, UOID_LEN);
		getNonce_metaData(metaDataFilename, metaDataNonce);
		
		vector<uint32_t> storedFiles_index = search_SHA1(metaDataSHA1);
		for(size_t i = 0; i < storedFiles_index.size(); ++i)
		{
			char storedmetaDataFilename[256];
			memset(storedmetaDataFilename, 0, 256);
			create_metaDataFilename(storedmetaDataFilename, homeDir, storedFiles_index[i]);
			
			unsigned char storedmetaDataNonce[UOID_LEN];
			memset(storedmetaDataNonce, 0, UOID_LEN);
			getNonce_metaData(storedmetaDataFilename, storedmetaDataNonce);
			
			if(!memcmp(storedmetaDataNonce, metaDataNonce, UOID_LEN))
			{
				#ifdef DEBUG
					printf("Server: This get request file is stored in the file system, drop it\n");
				#endif
				isStored = true;
			}
		}
		//--------------------------------------------------------------------------------
		
		if(!isForwardGetRes)	//not in routing table
		{
			bool isSentGet = false;
			string getFilename;
			bool isReplace = true;
			pthread_mutex_lock(&get_lock);
			map<UOID_Map, string>::iterator iter = getMsgFilename_map.find(get_uoid);
			map<UOID_Map, bool>::iterator iter2 = getReplace_map.find(get_uoid);
			if( iter != getMsgFilename_map.end() )
			{
				isSentGet = true;
				getFilename = iter->second;
				isReplace = iter2->second;
				getMsgFilename_map.erase(iter);
				getReplace_map.erase(iter2);
			}	
			pthread_mutex_unlock(&get_lock);
			
			if(isSentGet && !isStored)	//this node sends out get request, store the file into permanent spaces
			{
				char getMetaDataFilename[256];
				memset(getMetaDataFilename, 0, 256);
				create_metaDataFilename(getMetaDataFilename, homeDir, tmpFileIndex);
				createFile_filesys(getMetaDataFilename, tmpFileIndex);	
				
				//update file system
				update_filesys();
			
				if(isReplace)
				{
					//copy data file to current directory
					ifstream input(dataFilename, ios::in |ios::binary);
					ofstream output(getFilename.c_str(), ios::out |ios::binary);
					output << input.rdbuf(); 
				}
			}
			else
			{
				if(isReplace)
				{
					//copy data file to current directory
					ifstream input(dataFilename, ios::in |ios::binary);
					ofstream output(getFilename.c_str(), ios::out |ios::binary);
					output << input.rdbuf(); 
				}
			
				deleteDiskFile(metaDataFilename, dataFilename);
			}
		}
		else	//needs to forward this getResMsg in routing table
		{
			bool isCacheFile = false;
			if(!isStored && drand48() < cacheProb) //store into cache if needed
			{
				//call cache file algorithm
				isCacheFile = doCacheFile(homeDir, data_fileSize, cacheSize, tmpFileIndex);
			}
			
			if(header.TTL == 0)
			{
				if(!isCacheFile)	//do not need to cache this file
				{
					//delete this temp file from filesys on disk
					deleteDiskFile(metaDataFilename, dataFilename);
				}
				return false;
			}	
			
			if(!isCacheFile)	//do not need to cache this file, after send getResMsg delete meta and data file
			{			
				getResMsg->fileState = true;
			}	

			DispatchElement* dispatchElm = new DispatchElement(getResMsg, forwardsockfd, FORWARD_MSG);	
			push_dispatch_queue(dispatchElm);	
		}			
	}
	else	//file received before
	{
		#ifdef DEBUG
			printf("Server: store message received before\n");
		#endif	
		rv1 = recv_metaDataFile(sockfd, NULL, metaData_fileSize, recv_before);
		if(rv1 == -1)
			return true;
		
		uint32_t data_fileSize = header.dataLen - 4 - metaData_fileSize;
		rv2 = recv_fileData(sockfd, NULL, data_fileSize, recv_before, NULL);
		if(rv2 == -1)
			return true;
	}

	return false;
}

void Node:: handle_deleteMsg(DeleteMsg* deleteMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), deleteMsg);

	flood_msg(deleteMsg, sockfd);
	
	vector<uint32_t> matchResults = get_matchFileIndex(deleteMsg->deleteFilename, deleteMsg->deleteSHA1, deleteMsg->deleteNonce);

	if( matchResults.size() == 1)
	{
		unsigned char passNonce[UOID_LEN];
		memset(passNonce, 0, UOID_LEN);
		SHA1(deleteMsg->deletePass, UOID_LEN, passNonce);
		
		if(!memcmp(passNonce, deleteMsg->deleteNonce, UOID_LEN))
		{
			//delete this file from file system
			deleteFile_command(matchResults[0], homeDir);
		}
	}
}
