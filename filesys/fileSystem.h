#ifndef FILESYS_H
#define FILESYS_H

#include "../system/tool.h"
#include "bitVector.h"
#include "meta.h"

using namespace std;

extern char kwrd_filename[FILENAME_LEN];
extern char name_filename[FILENAME_LEN];
extern char sha1_filename[FILENAME_LEN];
extern char index_filename[FILENAME_LEN];
extern char cacheList_filename[FILENAME_LEN];

//----------------------------------------------------------------------------------------
extern multimap<string, uint32_t> name_map;
extern multimap<UOID_Map, uint32_t> sha1_map;
extern multimap<BitVector, uint32_t> kwrd_map;
extern uint32_t filesys_index;
extern pthread_mutex_t filesys_lock;

//record the file index number of the cached files
//the older cache file index is at the front of this list
extern list<uint32_t> cacheFile_index_list;	
extern uint32_t currentCacheSize_Bytes;
extern pthread_mutex_t cacheFile_list_lock;
//----------------------------------------------------------------------------------------

extern uint32_t tempMata_index;
extern pthread_mutex_t tempMeta_lock;

extern void init_filesys(char* homeDir);
extern void read_filesys(char* homeDir);

extern int storeFile_filesys(const char* filename, vector<string>& keywords, char* node_instance_id, char* homeDir);
extern void createFile_filesys(const char* metaDataFilename, uint32_t createFile_index);
extern int update_filesys();
extern void deleteFile_filesys(uint32_t deleteFile_index);
extern void deleteDiskFile(const char* metaDataFilename, const char* dataFilename);
extern void deleteFile_command(uint32_t deleteFile_index, char* homeDir);
extern int close_filesys(char* homeDir);
extern void reset_filesys(char* homeDir);
extern void print_filesys();

extern bool doCacheFile(char* homeDir, uint32_t data_fileSize, uint32_t totalCacheSize, uint32_t cacheFile_index);
extern int update_cacheList_filesys();
extern void remove_cacheList_if_needed(uint32_t remove_index, char* homeDir);
extern void update_cacheList(uint32_t update_index);
extern void search_update_cacheList(vector<uint32_t> matchFiles);

extern void get_threeIndex(const char* metaDataFilename, char* filename, unsigned char* file_sha1, BitVector& bv);
extern void getSHA1_metaData(const char* metaDataFilename, unsigned char* metaDataSHA1);
extern void getNonce_metaData(const char* metaDataFilename, unsigned char* metaDataNonce);
extern void getPassword_PassFile(FILE* pf, unsigned char* deletePass);

extern vector<uint32_t> search_filename(string filename);
extern vector<uint32_t> search_SHA1(unsigned char* search_sha1);
extern vector<uint32_t> search_kwrd(vector<string> kwrds);
extern vector<uint32_t> search_filesys(unsigned char searchType, string searchQuery);

extern string print_search_metaDataFile(const char* metaDataFilename, uint32_t searchIndex, const char *FileID, unsigned char* searchFileSHA1);

#endif
