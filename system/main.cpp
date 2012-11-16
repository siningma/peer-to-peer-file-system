#include "node.h"
#include "tool.h"

using namespace std;

int main(int argc, char* argv[])
{
	//parse the commandline
	bool reset = false;
	if(pthread_mutex_init(&shut_down_lock, NULL) != 0)
	{
		printf("shut down lock init fail\n");
		exit(1);
	}
	if(pthread_mutex_init(&tempMeta_lock, NULL) != 0)
	{
		printf("tempMeta lock init fail\n");
		exit(1);
	}
	for(argc--, argv++; argc > 0; argc--, argv++)
	{
		if(!strcmp(*argv, "-reset"))
			reset = true;
		else
		{
			int len = strlen(*argv);
			char filename[len + 1];
			memset(filename, 0, len + 1);
			strcpy(filename, *argv);
			if(strstr(filename, ".ini"))
			{
				bool isShutdown = false;	
				time_t localtime=(time_t)0;
				time(&localtime);
				srand48((long)localtime);

				Node* node = new Node(filename, reset);	
				//start filesys
						
				while(!isShutdown)
				{
					node->run();
					node->cleanup();
					
					pthread_mutex_lock(&shut_down_lock);
					isShutdown = gnShutdown;
					pthread_mutex_unlock(&shut_down_lock);
					
					if(node->node_type == BEACON_NODE)
						break;
				}	
				delete node;
			}	
		}
	}
	
	//the program finishes
	pthread_mutex_destroy(&shut_down_lock);
	pthread_mutex_destroy(&tempMeta_lock);
	return 0;
}
