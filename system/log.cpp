#include "log.h"

using namespace std;

Log :: Log(char* homeDir)
{
	if(pthread_mutex_init(&write_lock, NULL) != 0)
	{
		printf("write lock init fail\n");
		exit(1);
	}
	char dir[strlen(homeDir) + strlen("/servant.log")];
	strcpy(dir, homeDir);
	strcat(dir, "/servant.log");
	pFile = fopen(dir, "a+");
}

Log :: ~Log()
{
	pthread_mutex_destroy(&write_lock);
	fclose(pFile);
}

void Log :: write_to_file(char type, const char* neighbor, Msg* msg)
{
	struct timeval tv; 
	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL); 
	long int sec = tv.tv_sec;	
	int millisec = tv.tv_usec / 1000;
	int size = HEADER_LEN + msg->header.dataLen;
	char neighbor_str[strlen(neighbor) + 1];
	memset(neighbor_str, 0, strlen(neighbor) + 1);
	memcpy(neighbor_str, neighbor, strlen(neighbor));

	pthread_mutex_lock(&write_lock);
	unsigned char msgType = msg->header.msgType;
	fprintf(pFile, "%c <%10ld.%03d> ", type, sec, millisec);
	fprintf(pFile, "<%s> ", neighbor_str);
	print_msgType(msg->header.msgType);
	fprintf(pFile, "<%d> <%d> ", size, msg->header.TTL);
	fprintf(pFile, "<0x");
	for(int i = 16; i < 20; i++)
    	fprintf(pFile, "%02x", (unsigned char)msg->header.UOID[i]);
    fprintf(pFile, ">");	
    if(msg->header.dataLen != 0)
    {	
		char str[MAXDATASIZE];
		memset(str, 0, MAXDATASIZE);
    	msg->get_payload(LOG_DATA, str);	
    	if(msgType == JNRS || msgType == CKRS || msgType == STRS || msgType == SHRS || msgType == GTRQ)
    	{
    		fprintf(pFile, " %c0x", str[0]);
    		for(int i = 1; i < 5; i++)
    			fprintf(pFile, "%02x", (unsigned char)str[i]);
    		if(msgType == JNRS)
    			fprintf(pFile, "%s", str + 5);
    		else	
    			fprintf(pFile, "%c", str[5]);	
    	}
    	else
			fprintf(pFile, " %s", str);
	}
	fprintf(pFile,"\n");
	fflush(pFile);
	pthread_mutex_unlock(&write_lock);
}

void Log :: write_to_file(char type, const char* neighbor, unsigned char* data, int logDataLen, MsgHeader header)
{
	struct timeval tv; 
	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL); 
	long int sec = tv.tv_sec;	
	int millisec = tv.tv_usec / 1000;
	int size = HEADER_LEN + header.dataLen;
	char neighbor_str[strlen(neighbor) + 1];
	memset(neighbor_str, 0, strlen(neighbor) + 1);
	memcpy(neighbor_str, neighbor, strlen(neighbor));

	pthread_mutex_lock(&write_lock);
	//unsigned char msgType = msg->header.msgType;
	fprintf(pFile, "%c <%10ld.%03d> ", type, sec, millisec);
	fprintf(pFile, "<%s> ", neighbor_str);
	print_msgType(header.msgType);
	fprintf(pFile, "<%d> <%d> ", size, header.TTL);
	fprintf(pFile, "<0x");
	for(int i = 16; i < 20; i++)
    	fprintf(pFile, "%02x", (unsigned char)header.UOID[i]);
    fprintf(pFile, ">");
    if(logDataLen > 0)
    {
    	fprintf(pFile, " <0x");
    	for(int i = 16; i < 20; i++)
    		fprintf(pFile, "%02x", data[i]);
    	fprintf(pFile, ">");
    }
	fprintf(pFile,"\n");
	fflush(pFile);
	pthread_mutex_unlock(&write_lock);
}

void Log :: close_log(char* hostname, char* port, int type)
{
	pthread_mutex_lock(&write_lock);
	if(type == USER_SHUTDOWN)
		fprintf(pFile, "Node <%s:%s> shutdown\n\n", hostname, port);
	else if(type == SELF_RESTART)
		fprintf(pFile, "Node <%s:%s> Soft Restart\n\n", hostname, port);	
	fflush(pFile);	
	pthread_mutex_unlock(&write_lock);
}

void Log:: print_msgType(unsigned char msgType)
{
	switch(msgType)
	{
		case JNRQ:
		fprintf(pFile, "<JNRQ> ");
		break;
		case JNRS:
		fprintf(pFile, "<JNRS> ");
		break;
		case HLLO:
		fprintf(pFile, "<HLLO> ");
		break;
		case KPAV:
		fprintf(pFile, "<KPAV> ");
		break;
		case NTFY:
		fprintf(pFile, "<NTFY> ");
		break;
		case CKRQ:
		fprintf(pFile, "<CKRQ> ");
		break;
		case CKRS:
		fprintf(pFile, "<CKRS> ");
		break;
		case SHRQ:
		fprintf(pFile, "<SHRQ> ");
		break;
		case SHRS:
		fprintf(pFile, "<SHRS> ");
		break;
		case GTRQ:
		fprintf(pFile, "<GTRQ> ");
		break;
		case GTRS:
		fprintf(pFile, "<GTRS> ");
		break;
		case STOR:
		fprintf(pFile, "<STOR> ");
		break;
		case DELT:
		fprintf(pFile, "<DELT> ");
		break;
		case STRQ:
		fprintf(pFile, "<STRQ> ");
		break;
		case STRS:
		fprintf(pFile, "<STRS> ");
		break;
		default:
		break;
	}
}
