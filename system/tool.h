#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <signal.h>
#include <cctype>

#include <openssl/sha.h> 
#include <openssl/md5.h>

#include <list>
#include <vector>
#include <queue>
#include <map>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

#ifndef min
#define min(A,B) (((A)>(B)) ? (B) : (A))
#endif 

#define STDIN 0

#define HEADER_LEN 27
#define UOID_LEN 20
#define BITVECTOR_LEN 128
#define FILENAME_LEN 256

#define SHA_DIGEST_LENGTH 20
#define MD5_DIGEST_LENGTH 16
#define MAX_LINE 128
#define MAXDATASIZE 512
//#define _DEBUG_ 1
#define MAX_BUF_SIZE 8192
//#define DEBUG 1

#include <assert.h>	

#define SEND_DATA 0
#define LOG_DATA 1

#define UNKNOWN 0
#define USER_SHUTDOWN 1
#define UNEXPECTED_KILL_RECV 2
#define SELF_RESTART 3

extern void get_time(char* time);
extern int currentTime();
extern void GetUOID(char *node_inst_id, char *obj_type, char *uoid_buf, int uoid_buf_sz);
extern vector<char*> read_file(char* filename);
extern vector<string> readFile(char* filename);
extern void trimString(char* str);
extern void parseLine(const char* str, char* prefix, char* data);
extern void parseBeacon(const char* str, char* hostname, char* port);
extern vector<string> tokenize(string text);
extern uint32_t getFilesize(const char* filename);
extern void parseKeywords(const string& word, vector<string>& keywords);
extern void stringToHex(string s, unsigned char* hex);

class UOID_Map
{
	public:
		unsigned char m_UOID[20];
		
		UOID_Map();
		UOID_Map(unsigned char *uoid);
		UOID_Map(const UOID_Map& other);
		UOID_Map& operator= (const UOID_Map& other);
		bool operator<(const UOID_Map& other) const;
};

extern void create_metaDataFilename(char* filename, char* homeDir, uint32_t tmpFileIndex);
extern void create_dataFilename(char* filename, char* homeDir, uint32_t tmpFileIndex);
extern void create_passFilename(char* filename, char* homeDir, uint32_t tmpFileIndex);
extern void create_tempMetaDataFilename(char* filename, char* homeDir, uint32_t tempMataDataIndex);

class SearchRes_Elm
{
	public:
		uint32_t tempMataIndex;
		UOID_Map fileID_uoid;
		
		SearchRes_Elm();
		SearchRes_Elm(uint32_t temp_index, UOID_Map uoid_fileID);
		SearchRes_Elm(const SearchRes_Elm& other);
		SearchRes_Elm& operator= (const SearchRes_Elm& other);
};

#endif
