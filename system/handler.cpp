#include "node.h"

bool Node :: handle_recv_helloMsg(MsgHeader header, char* msg_data, int sockfd)
{
	HelloMsg *helloMsg = new HelloMsg();
	helloMsg->create_header(header);
	helloMsg->parse_payload(msg_data);
	return handle_helloMsg(helloMsg, sockfd);
}

void Node :: handle_recv_notifyMsg(MsgHeader header, char* msg_data, int sockfd)
{
	NotifyMsg *notifyMsg = new NotifyMsg();
	notifyMsg->create_header(header);
	notifyMsg->parse_payload(msg_data);
	handle_notifyMsg(notifyMsg, sockfd);
	
	//receive notify message, send out check message
	send_check_message(sockfd);
}

Msg* Node :: handle_recv_msg(MsgHeader header, char* msg_data, int sockfd)
{
	Msg* msg;
	unsigned char msgType = header.msgType;
	if(msgType == JNRQ)
	{	
		JoinMsg *joinMsg = new JoinMsg();
		joinMsg->create_header(header);
		joinMsg->parse_payload(msg_data);
		handle_joinMsg(joinMsg, sockfd);
		msg = (Msg *)joinMsg;
	}	
	else if(msgType == JNRS)
	{
		JoinResMsg *joinResMsg = new JoinResMsg();
		joinResMsg->create_header(header);
		joinResMsg->parse_payload(msg_data);
		handle_joinResMsg(joinResMsg, sockfd);
		msg = (Msg *)joinResMsg;
	}	
	else if(msgType == KPAV)
	{
		KeepAliveMsg *keepAliveMsg = new KeepAliveMsg();
		keepAliveMsg->create_header(header);
		handle_keepAliveMsg(keepAliveMsg, sockfd);
		msg = (Msg *)keepAliveMsg;
	}		
	else if(msgType == CKRQ)
	{
		CheckMsg *checkMsg = new CheckMsg();
		checkMsg->create_header(header);
		handle_checkMsg(checkMsg, sockfd);
		msg = (Msg *)checkMsg;
	}
	else if(msgType == CKRS)
	{
		CheckResMsg *checkResMsg = new CheckResMsg();
		checkResMsg->create_header(header);
		checkResMsg->parse_payload(msg_data);
		handle_checkResMsg(checkResMsg, sockfd);
		msg = (Msg *)checkResMsg;
	}
	else if(msgType == STRQ)
	{
		StatusMsg *statusMsg = new StatusMsg();
		statusMsg->create_header(header);
		statusMsg->parse_payload(msg_data);
		handle_statusMsg(statusMsg, sockfd);
		msg = (Msg *)statusMsg;
	}
	else if(msgType == STRS)
	{
		StatusResMsg *statusResMsg = new StatusResMsg();
		statusResMsg->create_header(header);
		statusResMsg->parse_payload(msg_data);
		handle_statusResMsg(statusResMsg, sockfd);
		msg = (Msg *)statusResMsg;
	}
	else if(msgType == SHRQ)
	{
		SearchMsg *searchMsg = new SearchMsg();
		searchMsg->create_header(header);
		searchMsg->parse_payload(msg_data);
		handle_searchMsg(searchMsg, sockfd);
		msg = (Msg *)searchMsg;
	}
	else if(msgType == GTRQ)
	{
		GetMsg *getMsg = new GetMsg();
		getMsg->create_header(header);
		getMsg->parse_payload(msg_data);
		handle_getMsg(getMsg, sockfd);
		msg = (Msg *)getMsg;
	}
	else if(msgType == DELT)
	{
		DeleteMsg *deleteMsg = new DeleteMsg();
		deleteMsg->create_header(header);
		deleteMsg->parse_payload(msg_data);
		handle_deleteMsg(deleteMsg, sockfd);
		msg = (Msg *)deleteMsg;
	}
	
	return msg;
}

HelloMsg* Node :: create_helloMsg()
{
	HelloMsg* helloMsg = new HelloMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	helloMsg->create_header(HLLO, UOID, 1);
	helloMsg->create_payload_data(port, hostname);	
	return helloMsg;
}

JoinMsg* Node :: create_joinMsg()
{
	JoinMsg* joinMsg = new JoinMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	joinMsg->create_header(JNRQ, UOID, TTL);
	joinMsg->create_payload_data(location, port, hostname);	
	return joinMsg;
}

JoinResMsg* Node :: create_joinResMsg(char* join_UOID, uint32_t distance)
{
	JoinResMsg* joinResMsg = new JoinResMsg();
	char UOID[UOID_LEN];
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	joinResMsg->create_header(JNRS, UOID, TTL);
	uint16_t host_port = (uint16_t)atoi(port);
	joinResMsg->create_payload_data(join_UOID, distance, host_port, hostname);	
	return joinResMsg;
}

KeepAliveMsg* Node :: create_keepAliveMsg()
{
	KeepAliveMsg *keepAliveMsg = new KeepAliveMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	keepAliveMsg->create_header(KPAV, UOID, 1);
	return keepAliveMsg;
}

NotifyMsg* Node :: create_notifyMsg(unsigned char error_code)
{
	NotifyMsg *notifyMsg = new NotifyMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	notifyMsg->create_header(NTFY, UOID, 1);
	notifyMsg->create_payload_data(error_code);
	return notifyMsg;
}

CheckMsg* Node :: create_checkMsg()
{
	CheckMsg *checkMsg = new CheckMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	checkMsg->create_header(CKRQ, UOID, TTL);
	return checkMsg;
}

CheckResMsg* Node :: create_checkResMsg(char* check_UOID)
{
	CheckResMsg *checkResMsg = new CheckResMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	checkResMsg->create_header(CKRS, UOID, TTL);
	checkResMsg->create_payload_data(check_UOID);
	return checkResMsg;
}

StatusMsg* Node :: create_statusMsg(unsigned char status_TTL, unsigned char report_type)
{
	StatusMsg *statusMsg = new StatusMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);

	if(report_type == 0x01)
	{
		//add local neighbors info
		uint16_t local_port = (uint16_t)atoi(port);
		vector<uint16_t> neighbor_ports;
		pthread_mutex_lock(&nb_map_lock);
		map<int, Neighbor_node*>:: iterator iter;
		for(iter = nb_map.begin(); iter != nb_map.end(); ++iter)
			neighbor_ports.push_back(iter->second->neighbor_port);	
		pthread_mutex_unlock(&nb_map_lock);
		
		pthread_mutex_lock(&status_file_lock);
		issent_status = true;
		status_sent_time = currentTime();
		
		fprintf(statusFile, "V -t * -v 1.0a5\n");
		fflush(statusFile);	
		
		write_to_statusFile(local_port, neighbor_ports);	
		pthread_mutex_unlock(&status_file_lock);	
	}
	else
	{
		vector<uint32_t> filesys_files;
		pthread_mutex_lock(&filesys_lock);
		for(multimap<string, uint32_t>::iterator iter = name_map.begin(); iter != name_map.end(); ++iter)
		{	
			filesys_files.push_back(iter->second);
		}
		pthread_mutex_unlock(&filesys_lock);
	
		pthread_mutex_lock(&status_file_lock);
		issent_status = true;
		status_sent_time = currentTime();
		
		write_to_statusFile(filesys_files);
		pthread_mutex_unlock(&status_file_lock);
	}
	
	statusMsg->create_header(STRQ, UOID, status_TTL);
	statusMsg->create_payload_data(report_type);
	return statusMsg;
}

StatusResMsg* Node :: create_statusResMsg(char* status_UOID)
{
	StatusResMsg *statusResMsg = new StatusResMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	statusResMsg->create_header(STRS, UOID, TTL);
	uint16_t hostInfoLen = 2 + strlen(hostname);
	uint16_t hostPort = (uint16_t)atoi(port);
	statusResMsg->create_payload_data(status_UOID, hostInfoLen, hostPort, hostname);
	pthread_mutex_lock(&nb_map_lock);
	for(map<int, Neighbor_node*>::iterator it = nb_map.begin(); it != nb_map.end(); ++it)
		statusResMsg->create_payload_neighbor_data(it->second);
	pthread_mutex_unlock(&nb_map_lock);
	return statusResMsg;
}

StatusResMsg* Node :: create_statusResMsg_File(char* status_UOID)
{
	StatusResMsg *statusResMsg = new StatusResMsg();
	char UOID[UOID_LEN];	
	char obj[] = "msg";
	GetUOID(node_instance_id, obj, UOID, UOID_LEN);
	statusResMsg->create_header(STRS, UOID, TTL);
	statusResMsg->header.reserved = 1;
	uint16_t hostInfoLen = 2 + strlen(hostname);
	uint16_t hostPort = (uint16_t)atoi(port);
	statusResMsg->create_payload_data(status_UOID, hostInfoLen, hostPort, hostname);
	
	pthread_mutex_lock(&filesys_lock);
	for(multimap<string, uint32_t>::iterator iter = name_map.begin(); iter != name_map.end(); ++iter)
	{	
		statusResMsg->create_payload_file_data(homeDir, iter->second);
	}
	pthread_mutex_unlock(&filesys_lock);
	
	return statusResMsg;
}

void Node :: handle_joinMsg(JoinMsg* joinMsg, int sockfd)
{	
	char other_port[6];
	memset(other_port, 0, 6);
	sprintf(other_port, "%d", joinMsg->port);
	//this is a join socket, do not need to send KeepAliveMsg
	add_to_join_socket(sockfd);
	
	//add to neighbor table temporarily
	add_to_neighbor_table(sockfd, joinMsg->hostname, other_port);	
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), joinMsg);
	
	uint32_t distance = 0;
	if(joinMsg->location > location)
		distance = joinMsg->location - location;
	else
		distance = location - joinMsg->location;	
	//create joinResMsg, and put the reply msg into the dispatch queue	
	JoinResMsg* joinResMsg = create_joinResMsg(joinMsg->header.UOID, distance);
	DispatchElement* dispatchElm = new DispatchElement(joinResMsg, sockfd, SEND_MSG);	
	push_dispatch_queue(dispatchElm);
	
	//add to routing table for forwarding other node's joinResMsg
	if(joinMsg->header.TTL == 0)
		return;
		
	pthread_mutex_lock(&routing_map_lock);
	UOID_Map routing_map_UOID((unsigned char*)joinMsg->header.UOID);
	routing_map.insert(pair<UOID_Map, int>(routing_map_UOID, sockfd));
	pthread_mutex_unlock(&routing_map_lock);
	
	//this message needs to be flooded				
	flood_msg(joinMsg, sockfd);
}

void Node :: handle_joinResMsg(JoinResMsg* joinResMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), joinResMsg);
	if(joinResMsg->header.TTL == 0)
		return;
	
	//forward joinResMsg
	bool isInRoutingMap = false;
	int forward_sockfd = 0;
	pthread_mutex_lock(&routing_map_lock);
	map<UOID_Map, int>::iterator it;
	for(it = routing_map.begin(); it != routing_map.end(); ++it)
	{
    	if(!memcmp(it->first.m_UOID, joinResMsg->UOID, UOID_LEN))
    	{
    		isInRoutingMap = true;
    		forward_sockfd = it->second;
    		break;
    	}	
	}	
	if(!isInRoutingMap)	//not in routing table
	{
		pthread_mutex_unlock(&routing_map_lock);
		return;
	}	
	pthread_mutex_unlock(&routing_map_lock);
	DispatchElement* dispatchElm = new DispatchElement(joinResMsg, forward_sockfd, FORWARD_MSG);	
	push_dispatch_queue(dispatchElm);
}

bool Node :: handle_helloMsg(HelloMsg* helloMsg, int sockfd)
{
	char other_port[6];
	memset(other_port, 0, 6);
	sprintf(other_port, "%d", helloMsg->port);
	string s(helloMsg->hostname);
	s += "_";
	s += other_port;
	log->write_to_file('r', s.c_str(), helloMsg);
	
	//check if this node is my neighbor right now
	bool isSentHello = false;
	pthread_mutex_lock(&nb_map_lock);
	map<int, Neighbor_node*>::iterator iter;
	for(iter = nb_map.begin(); iter != nb_map.end(); ++iter)
	{
		if(iter->second->equals(helloMsg->hostname, helloMsg->port))
		{
			isSentHello = true;
			break;
		}	
	}		
	pthread_mutex_unlock(&nb_map_lock);	
	
	//not sent hello message before
	add_to_neighbor_table(sockfd, helloMsg->hostname, other_port);
	if(!isSentHello)
		send_hello_message(sockfd);
	
	//this hello message is from beacon node
	bool from_beacon = false;
	for(int i = 0; i < beaconNodes_count; ++i)
	{
		uint16_t beacon_port = (uint16_t)atoi(beaconNodes_port[i]);
		if(!strcmp(helloMsg->hostname, beaconNodes_hostname[i]) && (beacon_port == helloMsg->port))
			from_beacon = true;
	}
	
	printf("Receive Hello message from node: %s:%d\nservant:%s> ", helloMsg->hostname, helloMsg->port, port);
	fflush(stdout);
	
	//code to handle break tie
	if(node_type == BEACON_NODE && from_beacon)
	{
		bool isBreakTie = false;
		pthread_mutex_lock(&nb_map_lock);
		map<int, Neighbor_node*>::iterator iter1;
		map<int, Neighbor_node*>::iterator iter2;
		for(iter1 = nb_map.begin(); iter1 != nb_map.end(); ++iter1)
		{
			int count = 0;
			for(iter2 = iter1; iter2 != nb_map.end(); ++iter2)
			{
				if(iter1->second->equals(iter2->second))
				{
					count++;
					if(count > 1)
					{
						isBreakTie = true;
						break;
					}
				}	
			}
			if(isBreakTie)
				break;
		}
		pthread_mutex_unlock(&nb_map_lock);	
		//need to break tie
		if(isBreakTie)
		{
			uint16_t my_port = (uint16_t)atoi(port);
			if((my_port < helloMsg->port) || ((my_port == helloMsg->port) && strcmp(hostname, helloMsg->hostname) < 0))
			{
				#ifdef _DEBUG_
					printf("Server: break tie between %s_%d and %s_%d\n", hostname, my_port, helloMsg->hostname, helloMsg->port);
				#endif
				return true;
			}
			else
				send_hello_message(sockfd);
		}
	}	
	return false;
}

void Node :: handle_keepAliveMsg(KeepAliveMsg *keepAliveMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), keepAliveMsg);
}

void Node :: handle_notifyMsg(NotifyMsg* notifyMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), notifyMsg);
}

void Node :: handle_checkMsg(CheckMsg* checkMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), checkMsg);
	
	if(node_type == BEACON_NODE)	//response to this check message for beacon node
	{
		//create checkResMsg, and put the reply msg into the dispatch queue	
		CheckResMsg* checkResMsg = create_checkResMsg(checkMsg->header.UOID);
		DispatchElement* dispatchElm = new DispatchElement(checkResMsg, sockfd, SEND_MSG);	
		push_dispatch_queue(dispatchElm);
	}	
	else	//non beacon node, forward this check message
	{
		//add to routing table for forwarding other node's checkResMsg
		if(checkMsg->header.TTL == 0)
			return;
		
		pthread_mutex_lock(&routing_map_lock);
		UOID_Map routing_map_UOID((unsigned char*)checkMsg->header.UOID);
		routing_map.insert(pair<UOID_Map, int>(routing_map_UOID, sockfd));
		pthread_mutex_unlock(&routing_map_lock);
		
		//this message needs to be flooded				
		flood_msg(checkMsg, sockfd);
	}
}

void Node :: handle_checkResMsg(CheckResMsg* checkResMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), checkResMsg);
	
	if(checkResMsg->header.TTL == 0)
		return;
	
	if(!noCheck && (node_type == NON_BEACON_NODE))
	{
		//this checkResMsg response of the check message sent by this non_beacon_node
		pthread_mutex_lock(&check_lock);
		if(issent_check)
		{
			if(!memcmp(check_msg_UOID, checkResMsg->UOID, UOID_LEN))
			{
				checkRes_recv = true;
				#ifdef _DEBUG_
					char str[10];
					memset(str, 0, 10);
					checkResMsg->get_payload(LOG_DATA, str);
					printf("%c", str[0]);
					for(int i = 1; i < 5; i++)
						printf("%02x", (unsigned char)str[i]);
					printf("%c checkMsg is sent by this node\n", str[5]);
				#endif	
				pthread_mutex_unlock(&check_lock);
				return;
			}	
		}
		pthread_mutex_unlock(&check_lock);	
	}
			
	//forward checkResMsg
	bool isForwardCheckRes = false;
	int forwardSockfd = 0;
	pthread_mutex_lock(&routing_map_lock);
	map<UOID_Map, int>::iterator it;
	for(it = routing_map.begin(); it != routing_map.end(); ++it)
	{
		if(!memcmp(it->first.m_UOID, checkResMsg->UOID, UOID_LEN))
		{
			isForwardCheckRes = true;
			forwardSockfd = it->second;
			break;
		}	
	}	
	if(!isForwardCheckRes)	//not in my routing table
	{
		pthread_mutex_unlock(&routing_map_lock);
		return;
	}
	pthread_mutex_unlock(&routing_map_lock);
	DispatchElement* dispatchElm = new DispatchElement(checkResMsg, forwardSockfd, FORWARD_MSG);	
	push_dispatch_queue(dispatchElm);
}

void Node :: handle_statusMsg(StatusMsg *statusMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), statusMsg);
	
	StatusResMsg* statusResMsg;
	if(statusMsg->report_type == 0x01)
	{
		//create statusResMsg for neighbors status command
		statusResMsg = create_statusResMsg(statusMsg->header.UOID);
	}
	else
	{
		//create statusResMsg for files status command
		statusResMsg = create_statusResMsg_File(statusMsg->header.UOID);	
	}
	
	DispatchElement* dispatchElm = new DispatchElement(statusResMsg, sockfd, SEND_MSG);	
	push_dispatch_queue(dispatchElm);
	
	//add to routing table for forwarding other node's statusResMsg
	if(statusMsg->header.TTL == 0)
		return;
	
	pthread_mutex_lock(&routing_map_lock);
	UOID_Map routing_map_UOID((unsigned char*)statusMsg->header.UOID);
	routing_map.insert(pair<UOID_Map, int>(routing_map_UOID, sockfd));
	pthread_mutex_unlock(&routing_map_lock);
	
	//this message needs to be flooded				
	flood_msg(statusMsg, sockfd);
}

void Node :: handle_statusResMsg(StatusResMsg *statusResMsg, int sockfd)
{
	string s = get_neighbor(sockfd);
	log->write_to_file('r', s.c_str(), statusResMsg);
	//check if this statusResMsg needs to store or forward 
	pthread_mutex_lock(&status_file_lock);
	bool issent = issent_status;
	pthread_mutex_unlock(&status_file_lock);
		
	//forward statusResMsg
	bool isForwardStatusRes = false;
	int forwardsockfd = 0;
	pthread_mutex_lock(&routing_map_lock);
	map<UOID_Map, int>::iterator it;
	for(it = routing_map.begin(); it != routing_map.end(); ++it)
	{
		if(!memcmp(it->first.m_UOID, statusResMsg->UOID, UOID_LEN))
		{
			isForwardStatusRes = true;
			forwardsockfd = it->second;
			break;
		}	
	}	
	pthread_mutex_unlock(&routing_map_lock);
	
	if(!isForwardStatusRes)	//not in routing table
	{
		if(issent)
		{
			//store statusResMsg info
			pthread_mutex_lock(&status_file_lock);
			vector<uint16_t> neighbor_ports;
			for(int i = 0; i < (int)statusResMsg->neighbors.size(); ++i)
			{
				neighbor_ports.push_back(statusResMsg->neighbors[i]->neighbor_port);
			}	
			write_to_statusFile(statusResMsg->hostPort, neighbor_ports);
			pthread_mutex_unlock(&status_file_lock);
		}
	}
	else	//in routing table
	{			
		if(statusResMsg->header.TTL == 0)
			return;
		DispatchElement* dispatchElm = new DispatchElement(statusResMsg, forwardsockfd, FORWARD_MSG);	
		push_dispatch_queue(dispatchElm);
	}
}

bool Node:: handle_recv_statusResFileMsg(int sockfd, MsgHeader header)
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
	
	uint16_t msg_hostInfoLen = 0;
	rv = recv_givenLenData(sockfd, &msg_hostInfoLen, 2);
	if(rv == -1)
		return true;
	uint16_t hostInfoLen = ntohs(msg_hostInfoLen);
		
	uint16_t msg_hostPort = 0;
	rv = recv_givenLenData(sockfd, &msg_hostPort, 2);
	if(rv == -1)
		return true;
	uint16_t hostPort = ntohs(msg_hostPort);
	
	char msg_hostname[msg_hostInfoLen - 2];	
	memset(msg_hostname, 0, msg_hostInfoLen - 2);
	rv = recv_givenLenData(sockfd, &msg_hostname, msg_hostInfoLen - 2);
	if(rv == -1)
		return true;
	
	StatusResMsg *statusResMsg = new StatusResMsg();
	statusResMsg->create_header(STRS, header.UOID, header.TTL);
	statusResMsg->header.reserved = 1;
	statusResMsg->create_payload_data(msg_data_UOID, hostInfoLen, hostPort, msg_hostname);
	
	//check if this statusResMsg needs to store or forward 
	pthread_mutex_lock(&status_file_lock);
	bool issent = issent_status;
	pthread_mutex_unlock(&status_file_lock);
		
	//forward statusResMsg	
	bool isForwardStatusRes = false;
	int forwardsockfd = 0;
	pthread_mutex_lock(&routing_map_lock);
	map<UOID_Map, int>::iterator iter;
	for(iter = routing_map.begin(); iter != routing_map.end(); ++iter)
	{	
		if(!memcmp(iter->first.m_UOID, statusResMsg->UOID, UOID_LEN))
		{
			isForwardStatusRes = true;
			forwardsockfd = iter->second;
			break;
		}	
	}	
	pthread_mutex_unlock(&routing_map_lock);
	
	//store the metadata file in tempMeta directory in advance 
	uint32_t temp_len = 22 + hostInfoLen;
	uint32_t start = 0;
	while((temp_len + start) != header.dataLen)
	{
		uint32_t msg_recordLen = 0;
		rv = recv_givenLenData(sockfd, &msg_recordLen, 4);
		if(rv == -1)
			return true;
		uint32_t recordLen = ntohl(msg_recordLen);
		start += 4;		
		
		statusResMsg->header.dataLen += 4;
		statusResMsg->header.dataLen += recordLen;
		statusResMsg->recordLengths.push_back(recordLen);
		
		pthread_mutex_lock(&tempMeta_lock);
		uint32_t tempMetaDataFileIndex = tempMata_index;
		tempMata_index++;
		pthread_mutex_unlock(&tempMeta_lock);
		
		char tempMetaDataFileName[256];
		memset(tempMetaDataFileName, 0, 256);
		create_tempMetaDataFilename(tempMetaDataFileName, homeDir, tempMetaDataFileIndex);
		statusResMsg->fileIndexes.push_back(tempMetaDataFileIndex);
		rv = recv_metaDataFile(sockfd, tempMetaDataFileName, recordLen, recv_before);	
		if(rv == -1)
			return true;
			
		start += recordLen;
	}
	
	if(!isForwardStatusRes)	//not in routing table
	{
		if(issent)
		{
			//store statusResMsg info
			pthread_mutex_lock(&status_file_lock);
			write_to_statusFile(statusResMsg);
			pthread_mutex_unlock(&status_file_lock);
		}
	}
	else	//in routing table
	{		
		if(statusResMsg->header.TTL == 0)
			return false;
			
		statusResMsg->fileState = true;	
		DispatchElement* dispatchElm = new DispatchElement(statusResMsg, forwardsockfd, FORWARD_MSG);	
		push_dispatch_queue(dispatchElm);
	}
	
	return false;
}

void Node :: flood_msg(Msg* msg, int sockfd)
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
		DispatchElement* dispatchElm = new DispatchElement(msg, neighbor_sockfds[i], FORWARD_MSG);	
		//put the reply msg into the dispatch queue
		push_dispatch_queue(dispatchElm);	
	}
}

void Node :: send_all_neighbors_msg(Msg* msg)
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
		DispatchElement* dispatchElm = new DispatchElement(msg, neighbor_sockfds[i], SEND_MSG);	
		//put the reply msg into the dispatch queue
		push_dispatch_queue(dispatchElm);	
	}
}

void Node :: send_other_neighbors_msg(Msg* msg, int sockfd)
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
		DispatchElement* dispatchElm = new DispatchElement(msg, neighbor_sockfds[i], SEND_MSG);	
		//put the reply msg into the dispatch queue
		push_dispatch_queue(dispatchElm);	
	}
}

string Node :: get_neighbor(int sockfd)
{
	char neighbor_hostname[256];
	memset(neighbor_hostname, 0, 256);
	uint16_t neighbor_port = 0;
	
	pthread_mutex_lock(&nb_map_lock);
	map<int, Neighbor_node*>:: iterator iter = nb_map.find(sockfd);
	memcpy(neighbor_hostname, iter->second->hostname, strlen(iter->second->hostname));
	neighbor_port = iter->second->neighbor_port;
	pthread_mutex_unlock(&nb_map_lock);
	
	char str[strlen(neighbor_hostname) + 10];
	memset(str, 0, strlen(neighbor_hostname) + 10);
	sprintf(str, "%s_%d", neighbor_hostname, neighbor_port);
	string s(str);
	return s;
}

void Node :: send_notify_message(unsigned char error_code)
{
	NotifyMsg *notifyMsg = create_notifyMsg(error_code);
	send_all_neighbors_msg(notifyMsg);
}

void Node :: send_check_message(int sockfd)
{	
	//non beacon node, if there is no neighbors, soft restart immediately
	if(node_type == NON_BEACON_NODE)
	{
		pthread_mutex_lock(&nb_map_lock);
		int nb_map_size = (int)nb_map.size();
		pthread_mutex_unlock(&nb_map_lock);
		if(!nb_map_size)
		{
			#ifdef _DEBUG_
				printf("3 Soft Restart!\n");
			#endif
			pthread_mutex_lock(&shut_down_lock);
			gnShutdown = false;
			pthread_mutex_unlock(&shut_down_lock);
			exit_threads();
			pthread_cancel(signal_thread_id);
			return;
		}
	}
	
	//send out check messages only when this node is a non_beacon_node and noCheck == 0
	if(!noCheck && (node_type == NON_BEACON_NODE))
	{			
		CheckMsg* checkMsg = create_checkMsg();		
		//this node has sent out a check message
		pthread_mutex_lock(&check_lock);
		issent_check = true;
		memset(check_msg_UOID, 0, UOID_LEN);
		memcpy(check_msg_UOID, checkMsg->header.UOID, UOID_LEN);
		check_sent_time = currentTime();
		checkRes_recv = false;
		pthread_mutex_unlock(&check_lock);	
		send_other_neighbors_msg(checkMsg, sockfd);
	}
}

void Node :: write_to_statusFile(uint16_t local_port, vector<uint16_t> neighbor_ports)
{
	bool foundNode = false;
	for(int j = 0; j < (int)statusPorts.size(); ++j)
	{
		if(local_port == statusPorts[j])
		{
			foundNode = true;
			break;
		}
	}
	if(!foundNode)
	{
		statusPorts.push_back(local_port);
		write_to_statusFile_node(local_port);
	}	
	for(int i = 0; i < (int)neighbor_ports.size(); ++i)	
	{	
		bool found = false;
		for(int j = 0; j < (int)statusPorts.size(); ++j)
		{
			if(neighbor_ports[i] == statusPorts[j])
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			statusPorts.push_back(neighbor_ports[i]);
			write_to_statusFile_node(neighbor_ports[i]);
		}
		write_to_statusFile_link(local_port, neighbor_ports[i]);	
	}
}

void Node :: write_to_statusFile_node(uint16_t node_port)
{
		fprintf(statusFile, "n -t * -s ");
		fprintf(statusFile, "%d ", node_port);
		fprintf(statusFile, "-c red -i black\n");
		fflush(statusFile);
}		
		
void Node :: write_to_statusFile_link(uint16_t node_first, uint16_t node_second)
{
	fprintf(statusFile, "l -t * -s ");
	fprintf(statusFile, "%d ", node_first);
	fprintf(statusFile, "-d ");
	fprintf(statusFile, "%d ", node_second);
	fprintf(statusFile, "-c blue\n");
	fflush(statusFile);
}

int Node:: write_to_statusFile(vector<uint32_t> filesys_files)
{
	char read_buf[MAX_BUF_SIZE];
	memset(read_buf, 0, MAX_BUF_SIZE);
	
	if(filesys_files.size() > 1)
	{
		fprintf(statusFile, "%s:%d has %d files\n", hostname, atoi(port), filesys_files.size());
	}
	else if(filesys_files.size() == 1)
	{
		fprintf(statusFile, "%s:%d has the following file\n", hostname, atoi(port));
	}
	else
	{
		fprintf(statusFile, "%s:%d has no file\n", hostname, atoi(port));
	}
	fflush(statusFile);
	for(size_t i = 0; i < filesys_files.size(); ++i)
	{
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, filesys_files[i]);
		
		FILE *pf = fopen(metaDataFilename, "r");
		assert(pf);
		uint32_t fileSize = getFilesize(metaDataFilename);
		fseek(pf, 0, SEEK_SET);	
		fread(read_buf, 1, fileSize, pf);
		
		//write metadata to status file	
		size_t num = fwrite(read_buf, 1, fileSize, statusFile);
		if(num < fileSize)
		{
			fprintf(stderr, "Error: there is no space to store files in your disk\n");
			return -2;
		}
		fflush(statusFile);
	}
	return 0;	
}

int Node:: write_to_statusFile(StatusResMsg *statusResMsg)
{
	char read_buf[MAX_BUF_SIZE];
	memset(read_buf, 0, MAX_BUF_SIZE);
	
	size_t count = statusResMsg->fileIndexes.size();
	if(count > 1)
	{
		fprintf(statusFile, "%s:%d has %d files\n", statusResMsg->hostname, statusResMsg->hostPort, count);
	}
	else if(count == 1)
	{
		fprintf(statusFile, "%s:%d has the following file\n", statusResMsg->hostname, statusResMsg->hostPort);
	}
	else
	{
		fprintf(statusFile, "%s:%d has no file\n", statusResMsg->hostname, statusResMsg->hostPort);
	}
	fflush(statusFile);
	for(size_t i = 0; i < count; ++i)
	{
		char tempMetaDataFileName[256];
		memset(tempMetaDataFileName, 0, 256);
		create_tempMetaDataFilename(tempMetaDataFileName, homeDir, statusResMsg->fileIndexes[i]);
		
		FILE *pf = fopen(tempMetaDataFileName, "r");
		assert(pf);
		uint32_t fileSize = getFilesize(tempMetaDataFileName);
		fseek(pf, 0, SEEK_SET);	
		fread(read_buf, 1, fileSize, pf);
		
		//write metadata to status file	
		size_t num = fwrite(read_buf, 1, fileSize, statusFile);
		if(num < fileSize)
		{
			fprintf(stderr, "Error: there is no space to store files in your disk\n");
			return -2;
		}
		fflush(statusFile);
		remove(tempMetaDataFileName);
	}
	return 0;
}
