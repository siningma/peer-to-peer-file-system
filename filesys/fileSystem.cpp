#include "fileSystem.h"

using namespace std;

char kwrd_filename[FILENAME_LEN];
char name_filename[FILENAME_LEN];
char sha1_filename[FILENAME_LEN];
char index_filename[FILENAME_LEN];
char cacheList_filename[FILENAME_LEN];

multimap<string, uint32_t> name_map;
multimap<UOID_Map, uint32_t> sha1_map;
multimap<BitVector, uint32_t> kwrd_map;
pthread_mutex_t filesys_lock;

list<uint32_t> cacheFile_index_list;	
pthread_mutex_t cacheFile_list_lock;

uint32_t filesys_index = 0;
uint32_t currentCacheSize_Bytes = 0;

uint32_t tempMata_index = 0;
pthread_mutex_t tempMeta_lock;

void init_filesys(char* homeDir)
{
	memset(kwrd_filename, 0, FILENAME_LEN);
	strcpy(kwrd_filename, homeDir);
	strcat(kwrd_filename, "/kwrd_index");
	
	memset(name_filename, 0, FILENAME_LEN);
	strcpy(name_filename, homeDir);
	strcat(name_filename, "/name_index");
	
	memset(sha1_filename, 0, FILENAME_LEN);
	strcpy(sha1_filename, homeDir);
	strcat(sha1_filename, "/sha1_index");
	
	memset(index_filename, 0, FILENAME_LEN);
	strcpy(index_filename, homeDir);
	strcat(index_filename, "/index");
	
	memset(cacheList_filename, 0, FILENAME_LEN);
	strcpy(cacheList_filename, homeDir);
	strcat(cacheList_filename, "/cacheList");
	
	FILE* kwrd_pFile = fopen(kwrd_filename, "r");
	if(!kwrd_pFile)
		kwrd_pFile = fopen(kwrd_filename, "w+");
	fclose(kwrd_pFile);	
	
	FILE* name_pFile = fopen(name_filename, "r");
	if(!name_pFile)
		name_pFile = fopen(name_filename, "w+");	
	fclose(name_pFile);		
		
	FILE* sha1_pFile = fopen(sha1_filename, "r");
	if(!sha1_pFile)
		sha1_pFile = fopen(sha1_filename, "w+");
	fclose(sha1_pFile);	
		
	FILE* index_pFile = fopen(index_filename, "r");
	if(!index_pFile)
		index_pFile = fopen(index_filename, "w+");	
	fclose(index_pFile);	
		
	FILE* cacheList_pFile = fopen(cacheList_filename, "r");
	if(!cacheList_pFile)
		cacheList_pFile = fopen(cacheList_filename, "w+");	
	fclose(cacheList_pFile);	
	
	char files[FILENAME_LEN];
	memset(files, 0, FILENAME_LEN);
	strcpy(files, homeDir);
	strcat(files, "/files");	
	mkdir(files, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	char tempMeta[FILENAME_LEN];
	memset(tempMeta, 0, FILENAME_LEN);
	strcpy(tempMeta, homeDir);
	strcat(tempMeta, "/tempMeta");	
	mkdir(tempMeta, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		
	if(pthread_mutex_init(&filesys_lock, NULL) != 0)
	{
		printf("filesys lock init fail\n");
		exit(1);
	}	
	if(pthread_mutex_init(&cacheFile_list_lock, NULL) != 0)
	{
		printf("cacheFile list lock init fail\n");
		exit(1);
	}		
}

//read filesys at the beginning of the program starts
void read_filesys(char* homeDir)
{
	vector<string> file_data;
	char prefix[200];
	char data[20];
	
	//get name index file
	file_data = readFile(name_filename);
	for(int i = 0; i < (int)file_data.size(); ++i)
	{
		memset(prefix, 0, 200);
		memset(data, 0, 20);
		parseLine(file_data[i].c_str(), prefix, data);
		string name(prefix);
		uint32_t index_data= (uint32_t)atoi(data);
		name_map.insert(pair<string, uint32_t>(name, index_data)); 
	}	
	
	//get sha1 index file
	file_data.clear();
	file_data = readFile(sha1_filename);
	for(int i = 0; i < (int)file_data.size(); ++i)
	{
		memset(prefix, 0, 200);
		memset(data, 0, 20);
		parseLine(file_data[i].c_str(), prefix, data);
		
		//parse file data
		unsigned char hex[UOID_LEN];
		memset(hex, 0, UOID_LEN);
		string str(prefix);
		stringToHex(str, hex);
		UOID_Map sha1_data(hex);
		uint32_t index_data = (uint32_t)atoi(data);
		sha1_map.insert(pair<UOID_Map, uint32_t>(sha1_data, index_data));
	}
	
	//get keyword index file 
	file_data.clear();
	file_data = readFile(kwrd_filename);
	for(int i = 0; i < (int)file_data.size(); ++i)
	{
		memset(prefix, 0, 200);
		memset(data, 0, 20);
		parseLine(file_data[i].c_str(), prefix, data);
		
		//parse file data
		unsigned char hex[BITVECTOR_LEN];
		memset(hex, 0, BITVECTOR_LEN);
		string str(prefix);
		stringToHex(str, hex);
		BitVector kwrd_data(hex);
		uint32_t index_data = (uint32_t)atoi(data);
		kwrd_map.insert(pair<BitVector, uint32_t>(kwrd_data, index_data));
	}	
	
	//get last index number
	file_data.clear();
	file_data = readFile(index_filename);
	if(file_data.size())
		filesys_index = (uint32_t)atoi(file_data[0].c_str());
	
	//get cacheList cache file index
	file_data.clear();
	file_data = readFile(cacheList_filename);
	for(int i = 0; i < (int)file_data.size(); ++i)
	{
		uint32_t cacheFile_index = (uint32_t)atoi(file_data[i].c_str());
		cacheFile_index_list.push_back(cacheFile_index);
		
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, cacheFile_index);
								
		uint32_t data_fileSize = getFilesize(dataFilename);
		currentCacheSize_Bytes += data_fileSize;
	}
	#ifdef DEBUG
		printf("When start currentCacheSize_Bytes: %d\r\n", currentCacheSize_Bytes);
	#endif	
	#ifdef _DEBUG_
		print_filesys();
	#endif	
}

//store file into filesys by store command
int storeFile_filesys(const char* filename, vector<string>& keywords, char* node_instance_id, char* homeDir)
{
	FILE* pf = fopen(filename, "r");
	uint32_t tmpFileIndex = 0;
	if(pf)
	{
		pthread_mutex_lock(&filesys_lock);
		//generate .meta file
		tmpFileIndex = filesys_index;
		++filesys_index;
		pthread_mutex_unlock(&filesys_lock);
		
		//copy file to .data file at the same time
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, tmpFileIndex);
		
		ifstream input(filename, ios::in |ios::binary);
		ofstream output(dataFilename, ios::out |ios::binary);
		output << input.rdbuf(); 
		
		pthread_mutex_lock(&filesys_lock);
		Meta meta(filename);
		int rv = meta.init(node_instance_id, keywords);		
		if(rv == -1)
		{
			pthread_mutex_unlock(&filesys_lock);
			return -1;
		}
		meta.write_to_metadata(tmpFileIndex, homeDir);
		meta.write_to_passFile(tmpFileIndex, homeDir);
		
		string name(filename);
		name_map.insert( pair<string, uint32_t>(name, tmpFileIndex) ); 
		UOID_Map uoidMap(meta.fileHash);
		sha1_map.insert( pair<UOID_Map, uint32_t>(uoidMap, tmpFileIndex) );
		BitVector bv = meta.bitvector;
		kwrd_map.insert( pair<BitVector, uint32_t>(bv, tmpFileIndex) );
		pthread_mutex_unlock(&filesys_lock);
	}
	else
	{
		printf("Error: this file %s does not exist\n", filename);
		fclose(pf);
		return -1;
	}	
	fclose(pf);
	return (int)tmpFileIndex;
};

//message create file, for permanent and cache file
void createFile_filesys(const char* metaDataFilename, uint32_t createFile_index)
{
	//get three index from meta data file
	char filename[256];
	memset(filename, 0, 256);
	
	unsigned char createfile_SHA1[SHA_DIGEST_LENGTH];
	memset(createfile_SHA1, 0, SHA_DIGEST_LENGTH);
	
	BitVector bv;
		
	get_threeIndex(metaDataFilename, filename, createfile_SHA1, bv);
	UOID_Map uoidMap(createfile_SHA1);
	
	pthread_mutex_lock(&filesys_lock);
	string name(filename);
	name_map.insert(pair<string, uint32_t>(name, createFile_index)); 
	sha1_map.insert(pair<UOID_Map, uint32_t>(uoidMap, createFile_index));
	kwrd_map.insert(pair<BitVector, uint32_t>(bv, createFile_index));
	pthread_mutex_unlock(&filesys_lock);
}

//update 4 index files in the homeDir, when there is a new file added
int update_filesys()
{
	pthread_mutex_lock(&filesys_lock);
	FILE* pf = fopen(name_filename, "w");
	if(!pf)
	{
		fprintf(stderr, "cannot open or create file %s\n", name_filename);
		pthread_mutex_unlock(&filesys_lock);
		return -1;
	}
	for(multimap<string, uint32_t>::iterator iter = name_map.begin(); iter != name_map.end(); ++iter)
	{	
		fprintf(pf, "%s=%d\r\n", iter->first.c_str(), iter->second);
		fflush(pf);
	}	
	fclose(pf);
	
	pf = fopen(sha1_filename, "w");
	if(!pf)
	{
		fprintf(stderr, "cannot open or create file %s\n", sha1_filename);
		pthread_mutex_unlock(&filesys_lock);
		return -1;
	}
	for(multimap<UOID_Map, uint32_t>::iterator iter = sha1_map.begin(); iter != sha1_map.end(); ++iter)
	{
		for(int i = 0; i < UOID_LEN; ++i)
			fprintf(pf, "%02x", iter->first.m_UOID[i]);	
		fprintf(pf, "=%d\r\n", iter->second);
		fflush(pf);
	}	
	fclose(pf);
	
	pf = fopen(kwrd_filename, "w");
	if(!pf)
	{
		fprintf(stderr, "cannot open or create file %s\n", kwrd_filename);
		pthread_mutex_unlock(&filesys_lock);
		return -1;
	}
	for(multimap<BitVector, uint32_t>::iterator iter = kwrd_map.begin(); iter != kwrd_map.end(); ++iter)
	{
		for(int i = 0; i < BITVECTOR_LEN; ++i)
			fprintf(pf, "%02x", iter->first.bit_vector[i]);	
		fprintf(pf, "=%d\r\n", iter->second);
		fflush(pf);
	}	
	fclose(pf);
	
	pf = fopen(index_filename, "w");
	if(!pf)
	{
		fprintf(stderr, "cannot open or create file %s\n", index_filename);
		pthread_mutex_unlock(&filesys_lock);
		return -1;
	}
	fprintf(pf, "%d\r\n", filesys_index);
	fflush(pf);
	fclose(pf);
	pthread_mutex_unlock(&filesys_lock);	
	return 0;
}

void deleteFile_filesys(uint32_t deleteFile_index)
{
	pthread_mutex_lock(&filesys_lock);
	//if there is a match, remove from data structrue
	for(multimap<string, uint32_t>::iterator it = name_map.begin(); it != name_map.end();)
	{
		if(deleteFile_index == it->second)
		{
			name_map.erase(it++);
			break;
		}
		else
			it++;	
	}
	
	for(multimap<UOID_Map, uint32_t>::iterator it = sha1_map.begin(); it != sha1_map.end();)
	{
		if(deleteFile_index == it->second)
		{
			sha1_map.erase(it++);
			break;
		}
		else
			it++;	
	}
	
	for(multimap<BitVector, uint32_t>::iterator it = kwrd_map.begin(); it != kwrd_map.end();)
	{
		if(deleteFile_index == it->second)
		{
			kwrd_map.erase(it++);
			break;
		}
		else
			it++;	
	}
	pthread_mutex_unlock(&filesys_lock);		
}

void deleteDiskFile(const char* metaDataFilename, const char* dataFilename)
{
	if(fopen(metaDataFilename, "r"))
		remove(metaDataFilename);
	else
	{
		printf("Server: delete filename %s does not exist\n", metaDataFilename);
		assert(false);
	}		
	
	if(fopen(dataFilename, "r"))
		remove(dataFilename);
	else
	{
		printf("Server: delete filename %s does not exist\n", dataFilename);
		assert(false);
	}		
}

void deleteFile_command(uint32_t deleteFile_index, char* homeDir)
{
	deleteFile_filesys(deleteFile_index);
	//update file system
	update_filesys();

	remove_cacheList_if_needed(deleteFile_index, homeDir);
	
	//remove from disk
	char metaDataFilename[256];
	memset(metaDataFilename, 0, 256);
	create_metaDataFilename(metaDataFilename, homeDir, deleteFile_index);
	char dataFilename[256];
	memset(dataFilename, 0, 256);
	create_dataFilename(dataFilename, homeDir, deleteFile_index);
	
	deleteDiskFile(metaDataFilename, dataFilename);
}

int close_filesys(char* homeDir)
{
	//update 5 files in the filesys and do clean up
	int rv1 = update_filesys();
	int rv2 = update_cacheList_filesys();
	
	char command[FILENAME_LEN];
	memset(command, 0, FILENAME_LEN);
	strcpy(command, "rm -f ");
	strcat(command, homeDir);
	strcat(command, "/tempMeta/*");	
	system(command);	
	
	pthread_mutex_destroy(&filesys_lock);
	pthread_mutex_destroy(&cacheFile_list_lock);
	if(rv1 ==  -1 || rv2 == -1)
		return -1;
		
	return 0;	
}

void reset_filesys(char* homeDir)
{
	memset(kwrd_filename, 0, FILENAME_LEN);
	strcpy(kwrd_filename, homeDir);
	strcat(kwrd_filename, "/kwrd_index");
	
	memset(name_filename, 0, FILENAME_LEN);
	strcpy(name_filename, homeDir);
	strcat(name_filename, "/name_index");
	
	memset(sha1_filename, 0, FILENAME_LEN);
	strcpy(sha1_filename, homeDir);
	strcat(sha1_filename, "/sha1_index");
	
	memset(index_filename, 0, FILENAME_LEN);
	strcpy(index_filename, homeDir);
	strcat(index_filename, "/index");
	
	memset(cacheList_filename, 0, FILENAME_LEN);
	strcpy(cacheList_filename, homeDir);
	strcat(cacheList_filename, "/cacheList");
	
	if(fopen(kwrd_filename, "r"))
		remove(kwrd_filename);
	
	if(fopen(name_filename, "r"))
		remove(name_filename);	
		
	if(fopen(sha1_filename, "r"))
		remove(sha1_filename);
	
	if(fopen(index_filename, "r"))
		remove(index_filename);
		
	if(fopen(cacheList_filename, "r"))
		remove(cacheList_filename);
	
	//remove all cached and permanent files
	char command[FILENAME_LEN];
	memset(command, 0, FILENAME_LEN);
	strcpy(command, "rm -f ");
	strcat(command, homeDir);
	strcat(command, "/files/*");	
	system(command);

	memset(command, 0, FILENAME_LEN);
	strcpy(command, "rm -f ");
	strcat(command, homeDir);
	strcat(command, "/tempMeta/*");	
	system(command);	
}

//algorithm to decide if needs to cache this file
bool doCacheFile(char* homeDir, uint32_t data_fileSize, uint32_t totalCacheSize, uint32_t cacheFile_index)
{
	uint32_t totalCacheSize_Bytes = totalCacheSize * 1024;
	if(data_fileSize > totalCacheSize_Bytes)
	{
		//if (filesize > CacheSize), do not store it
		return false;
	}
	
	vector<uint32_t> remove_cacheFiles;
	pthread_mutex_lock(&cacheFile_list_lock);
	//cache file algorithm
	while((data_fileSize + currentCacheSize_Bytes) > totalCacheSize_Bytes)
	{
		//start deleting files from the head of the LRU list
		uint32_t old_cacheFile_index = cacheFile_index_list.front();
		remove_cacheFiles.push_back(old_cacheFile_index);
		cacheFile_index_list.pop_front();	
		
		assert(currentCacheSize_Bytes >= data_fileSize);	
		currentCacheSize_Bytes -= data_fileSize;
		
		#ifdef DEBUG
			printf("currentCacheSize_Bytes: %d\n", currentCacheSize_Bytes);
		#endif
	}	
	cacheFile_index_list.push_back(cacheFile_index);
	currentCacheSize_Bytes += data_fileSize;
	pthread_mutex_unlock(&cacheFile_list_lock);
	
	//remove old cache file if needed	
	for(int i = 0; i < (int)remove_cacheFiles.size(); ++i)
	{
		#ifdef DEBUG
			printf("Server: to cache file %d, remove_cacheFiles include: %d\n", cacheFile_index, remove_cacheFiles[i]);
		#endif
		deleteFile_filesys(remove_cacheFiles[i]);				
		char metaDataFilename[256];
		memset(metaDataFilename, 0, 256);
		create_metaDataFilename(metaDataFilename, homeDir, remove_cacheFiles[i]);
		
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, remove_cacheFiles[i]);
		//delete these file from filesys
		deleteDiskFile(metaDataFilename, dataFilename);
	}	
	
	char cacheMetaDataFilename[256];
	memset(cacheMetaDataFilename, 0, 256);
	create_metaDataFilename(cacheMetaDataFilename, homeDir, cacheFile_index);
	createFile_filesys(cacheMetaDataFilename, cacheFile_index);	
	
	//update file system
	update_filesys();
	update_cacheList_filesys();
	return true;
}

int update_cacheList_filesys()
{
	pthread_mutex_lock(&cacheFile_list_lock);
	FILE* pf = fopen(cacheList_filename, "w");
	if(!pf)
	{
		fprintf(stderr, "cannot open or create file %s\n", cacheList_filename);
		pthread_mutex_unlock(&cacheFile_list_lock);
		return -1;
	}
	for(list<uint32_t>::iterator it = cacheFile_index_list.begin(); it != cacheFile_index_list.end(); ++it)
	{
		fprintf(pf, "%d\r\n", *it);
		fflush(pf);
	}
	fclose(pf);
	pthread_mutex_unlock(&cacheFile_list_lock);
	return 0;
}

void remove_cacheList_if_needed(uint32_t remove_index, char* homeDir)
{
	bool isCached = false;
	pthread_mutex_lock(&cacheFile_list_lock);
	if( find(cacheFile_index_list.begin(), cacheFile_index_list.end(), remove_index) != cacheFile_index_list.end() )
	{
		isCached = true;
		char dataFilename[256];
		memset(dataFilename, 0, 256);
		create_dataFilename(dataFilename, homeDir, remove_index);
		
		uint32_t remove_fileSize = getFilesize(dataFilename);
		cacheFile_index_list.remove(remove_index);
		
		assert(currentCacheSize_Bytes >= remove_fileSize);	
		currentCacheSize_Bytes -= remove_fileSize;
		
		#ifdef DEBUG
			printf("After remove currentCacheSize_Bytes: %d\n", currentCacheSize_Bytes);
		#endif
	}
	pthread_mutex_unlock(&cacheFile_list_lock);
	
	if(isCached)
	{
		update_cacheList_filesys();
	}
}

void update_cacheList(uint32_t update_index)	//based on LRU
{
	pthread_mutex_lock(&cacheFile_list_lock);	
	assert( find(cacheFile_index_list.begin(), cacheFile_index_list.end(), update_index) != cacheFile_index_list.end());	
	
	cacheFile_index_list.remove(update_index);
	cacheFile_index_list.push_back(update_index);
	pthread_mutex_unlock(&cacheFile_list_lock);
}

void get_threeIndex(const char* metaDataFilename, char* filename, unsigned char* file_sha1, BitVector& bv)
{
	FILE* pf = fopen(metaDataFilename, "r");
	if(!pf)
	{
		fprintf(stderr, "Server: cannot open cached file %s\n", metaDataFilename);
		assert(false);
	}
	
	int count = 0;
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	char prefix[20];
	memset(prefix, 0, 20);
	char data[MAXDATASIZE];
	memset(data, 0, MAXDATASIZE);
	
	while(!feof(pf))
	{
		memset(str, 0, MAXDATASIZE);
		if(fgets(str, MAXDATASIZE, pf) != NULL)
		{
			trimString(str);
			int len = strlen(str);
			if(len == 0)
				continue;
			memset(&str[len - 2], 0, 2);
			memset(prefix, 0, 20);
			memset(data, 0, MAXDATASIZE);
			if(count == 1)	//this line is filename
			{
				parseLine(str, prefix, data);
				strcpy(filename, data);
			}
			else if(count == 3)
			{
				parseLine(str, prefix, data);
				string s(data);
				stringToHex(s, file_sha1);
			}
			else if(count == 6)
			{
				parseLine(str, prefix, data);
				string s(data);
				unsigned char bitvector_file[BITVECTOR_LEN];
				memset(bitvector_file, 0, BITVECTOR_LEN);
				stringToHex(s, bitvector_file);
				bv.setBitVector(bitvector_file);
			}
		}
		count++;
	}	
}

void getSHA1_metaData(const char* metaDataFilename, unsigned char* metaDataSHA1)
{
	FILE* pf = fopen(metaDataFilename, "r");
	if(!pf)
	{
		fprintf(stderr, "Server: cannot open cached file %s\n", metaDataFilename);
		assert(false);
	}
	
	int count = 0;
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	char prefix[20];
	memset(prefix, 0, 20);
	char data[MAXDATASIZE];
	memset(data, 0, MAXDATASIZE);
	
	while(!feof(pf))
	{
		memset(str, 0, MAXDATASIZE);
		if(fgets(str, MAXDATASIZE, pf) != NULL)
		{
			trimString(str);
			int len = strlen(str);
			if(len == 0)
				continue;
			memset(&str[len - 2], 0, 2);
			if(count == 3)
			{
				parseLine(str, prefix, data);
				string s(data);
				stringToHex(s, metaDataSHA1);
				break;
			}
		}
		count++;
	}	
}

void getNonce_metaData(const char* metaDataFilename, unsigned char* metaDataNonce)
{
	FILE* pf = fopen(metaDataFilename, "r");
	if(!pf)
	{
		fprintf(stderr, "Server: cannot open cached file %s\n", metaDataFilename);
		assert(false);
	}
	
	int count = 0;
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	char prefix[20];
	memset(prefix, 0, 20);
	char data[MAXDATASIZE];
	memset(data, 0, MAXDATASIZE);
	
	while(!feof(pf))
	{
		memset(str, 0, MAXDATASIZE);
		if(fgets(str, MAXDATASIZE, pf) != NULL)
		{
			trimString(str);
			int len = strlen(str);
			if(len == 0)
				continue;
			memset(&str[len - 2], 0, 2);
			if(count == 4)
			{
				parseLine(str, prefix, data);
				string s(data);
				stringToHex(s, metaDataNonce);
				break;
			}
		}
		count++;
	}	
}

void getPassword_PassFile(FILE* pf, unsigned char* deletePass)
{
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	if(fgets(str, MAXDATASIZE, pf) != NULL)
	{
		trimString(str);
		int len = strlen(str);
		memset(&str[len - 2], 0, 2);
		string s(str);
		stringToHex(s, deletePass);
	}
}

vector<uint32_t> search_filename(string filename)
{
	vector<uint32_t> matchIndex;
	pthread_mutex_lock(&filesys_lock);
	multimap<string, uint32_t>::iterator it;
	pair<multimap<string, uint32_t>::iterator, multimap<string, uint32_t>::iterator> ret;
	
	ret = name_map.equal_range(filename);
	for(it = ret.first; it != ret.second; ++it)
		matchIndex.push_back(it->second);
	
	pthread_mutex_unlock(&filesys_lock);
	
	return matchIndex;
}
	
vector<uint32_t> search_SHA1(unsigned char* search_sha1)
{
	vector<uint32_t> matchIndex;
	UOID_Map uoid(search_sha1);
	pthread_mutex_lock(&filesys_lock);
	multimap<UOID_Map, uint32_t>::iterator it;
	pair<multimap<UOID_Map, uint32_t>::iterator, multimap<UOID_Map, uint32_t>::iterator> ret;
	
	ret = sha1_map.equal_range(uoid);
	for(it = ret.first; it != ret.second; ++it)
		matchIndex.push_back(it->second);
		
	pthread_mutex_unlock(&filesys_lock);
	
	return matchIndex;
}

vector<uint32_t> search_kwrd(vector<string> kwrds)
{
	vector<uint32_t> matchIndex;
	pthread_mutex_lock(&filesys_lock);
	for(multimap<BitVector, uint32_t>::iterator it = kwrd_map.begin(); it != kwrd_map.end(); ++it)
	{
		bool isMatch = true;
		for(int i = 0; i < (int)kwrds.size(); ++i)
		{
			BitVector kwrd_bitVector;
			kwrd_bitVector.keyword_bitvector(kwrds[i]);
			if( !kwrd_bitVector.match(it->first) )
			{
				isMatch = false;
				break;
			}	
		}	
		if(isMatch)
			matchIndex.push_back(it->second);
	}
	
	pthread_mutex_unlock(&filesys_lock);
	
	return matchIndex;
}

void search_update_cacheList(vector<uint32_t> matchFiles)
{
	vector<uint32_t> LRU_update_index;
	bool isUpdate = false;
	static uint32_t updateListTime = 0;
 	
	pthread_mutex_lock(&cacheFile_list_lock);
	for(size_t i = 0; i < matchFiles.size(); ++i)
	{
		for(list<uint32_t>::iterator it = cacheFile_index_list.begin(); it != cacheFile_index_list.end(); ++it)
		{
			if(matchFiles[i] == *it)
			{
				LRU_update_index.push_back(matchFiles[i]);
				isUpdate = true;
				break;
			}	
		}
	}
	pthread_mutex_unlock(&cacheFile_list_lock);
	
	for(size_t i = 0; i < LRU_update_index.size(); ++i)
	{
		update_cacheList(LRU_update_index[i]);
	}
	if(isUpdate)
	{
		++updateListTime;
		if(updateListTime % 4 == 0)	//update cacheList once in four times
			update_cacheList_filesys();
	}
}

vector<uint32_t> search_filesys(unsigned char searchType, string searchQuery)
{
	vector<uint32_t> searchResults;
	switch(searchType)
	{
		case 1:
		{
			searchResults = search_filename(searchQuery);
		}
		break;
		case 2:
		{
			unsigned char searchSHA1[SHA_DIGEST_LENGTH];
			memset(searchSHA1, 0, SHA_DIGEST_LENGTH);
			stringToHex(searchQuery, searchSHA1);
			searchResults = search_SHA1(searchSHA1);
		}
		break;
		case 3:
		{
			vector<string> keywords;
			parseKeywords(searchQuery, keywords);
			searchResults = search_kwrd(keywords);
		}
		break;
		default:
		assert(false);
		break;
	}
	if(searchResults.size() > 0)
	{
		search_update_cacheList(searchResults);
	}
	return searchResults;
}

void print_filesys()
{
	pthread_mutex_lock(&filesys_lock);
	printf("name_map:\n");
	for(multimap<string, uint32_t>::iterator iter = name_map.begin(); iter != name_map.end(); ++iter)
	{	
		printf("%s=%d\r\n", iter->first.c_str(), iter->second);
	}	
	
	printf("sha1_map:\n");
	for(multimap<UOID_Map, uint32_t>::iterator iter = sha1_map.begin(); iter != sha1_map.end(); ++iter)
	{
		for(int i = 0; i < UOID_LEN; ++i)
			printf("%02x", iter->first.m_UOID[i]);	
		printf("=%d\r\n", iter->second);
	}	
	printf("kwrd_map:\n");
	for(multimap<BitVector, uint32_t>::iterator iter = kwrd_map.begin(); iter != kwrd_map.end(); ++iter)
	{
		for(int i = 0; i < BITVECTOR_LEN; ++i)
			printf("%02x", iter->first.bit_vector[i]);	
		printf("=%d\r\n", iter->second);
	}
	printf("index:\n");
	printf("%d\r\n", filesys_index);
	printf("cacheList:\n");
	for(list<uint32_t>::iterator it = cacheFile_index_list.begin(); it != cacheFile_index_list.end(); ++it)
	{
		printf("%d\r\n", *it);
	}
	printf("currentCacheSize_Bytes: %d\r\n", currentCacheSize_Bytes);
	pthread_mutex_unlock(&filesys_lock);
}

string print_search_metaDataFile(const char* metaDataFilename, uint32_t searchIndex, const char *FileID, unsigned char* searchFileSHA1)
{
	string realFilename;
	fprintf(stdout, "[%d] FileID=", searchIndex);
	for(int i = 0; i < UOID_LEN; i++)
    	fprintf(stdout, "%02x", (unsigned char)FileID[i]);
    fprintf(stdout, "\n");
    FILE *pFile = fopen(metaDataFilename, "r");	
    if(!pFile)
	{
		fprintf(stderr, "Server: cannot open file %s\n", metaDataFilename);
		assert(false);
	}
	
	stringstream ss;
	ss << searchIndex;
	string countStr;
	ss >> countStr;
	
	int count = 0;
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	char commandPrefix[15];
	char commandData[256];
	while(!feof(pFile))
	{
		memset(str, 0, MAXDATASIZE);
		if((fgets(str, MAXDATASIZE, pFile) != NULL) && count != 0 && count != 6)
		{
			trimString(str);
			int len = strlen(str);
			if(len == 0)
				continue;
			
			if(count == 1)
			{
				memset(commandPrefix, 0, 15);
				memset(commandData, 0, 256);
				parseLine(str, commandPrefix, commandData);
				size_t data_len = strlen(commandData);
				memset(&commandData[data_len - 2], 0, 2);
				realFilename = commandData;
			}
			else if(count == 3)
			{
				memset(commandPrefix, 0, 15);
				memset(commandData, 0, 256);
				parseLine(str, commandPrefix, commandData);
				size_t data_len = strlen(commandData);
				memset(&commandData[data_len - 2], 0, 2);
				string s(commandData);
				stringToHex(s, searchFileSHA1);
			}
			string spaces(countStr.length() + 3, ' ');	
			fprintf(stdout, "%s%s", spaces.c_str(), str);
		}
		count++;
	}	
	fflush(stdout);
	return realFilename;
}
