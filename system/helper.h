#ifndef HELPER_H
#define HELPER_H

#include "tool.h"
#include "../protocol/message.h"
#include "../protocol/checkMsg.h"
#include "../protocol/checkResMsg.h"
#include "../protocol/helloMsg.h"
#include "../protocol/joinMsg.h"
#include "../protocol/joinResMsg.h"
#include "../protocol/notifyMsg.h"
#include "../protocol/statusMsg.h"
#include "../protocol/statusResMsg.h"
#include "../protocol/getMsg.h"
#include "../protocol/getResMsg.h"
#include "../protocol/storeMsg.h"
#include "../protocol/deleteMsg.h"
#include "../protocol/searchMsg.h"
#include "../protocol/searchResMsg.h"

#define SEND_MSG 0
#define FORWARD_MSG 1

class DispatchElement
{
	public: 
		Msg* msg;
		int sockfd;
		int type;
	
		DispatchElement(Msg* msg, int sockfd, int type)
		{
			this->msg = msg;
			this->sockfd = sockfd;
			this->type = type;
		}
};

class SendThread_Resourse
{
	public: 
		pthread_mutex_t send_queue_lock;
		pthread_cond_t send_queue_cv;
		std::queue<DispatchElement*> send_queue;
		double temp;
		bool thread_exit;
		
		SendThread_Resourse()
		{
			//init lock and cv for send thread
			if(pthread_mutex_init(&send_queue_lock, NULL) != 0)
			{
				printf("send queue lock init fail\n");
				exit(1);
			}
			if(pthread_cond_init(&send_queue_cv, NULL) != 0)
			{
				printf("send queue cv init fail\n");
				exit(1);
			}
			thread_exit = false;
		}
	
		~SendThread_Resourse()
		{
			pthread_mutex_destroy(&send_queue_lock);
			pthread_cond_destroy(&send_queue_cv);
		}
};

#endif
