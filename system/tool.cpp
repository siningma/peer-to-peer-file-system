#include "tool.h"

//get current time for this system
void get_time(char* time)
{
	struct timeval tv; 
	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL); 
		
	double sec = tv.tv_sec % 10000;	
	double usec = tv.tv_usec;
	memset(time, 0, 10);
	sprintf(time, "%.0f%0.f", sec, usec);
}

int currentTime()
{
	struct timeval tv; 
	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL);  	
	return tv.tv_sec % 10000000;
}

void GetUOID(char *node_inst_id, char *obj_type, char *uoid_buf, int uoid_buf_sz)
{
	static unsigned long seq_no=(unsigned long)1;
	char sha1_buf[SHA_DIGEST_LENGTH], str_buf[104];
	
	snprintf(str_buf, sizeof(str_buf), "%s_%s_%1ld", 
					node_inst_id, obj_type, (long)seq_no++);
	SHA1((unsigned char*)str_buf, strlen(str_buf), (unsigned char*)sha1_buf);
	memset(uoid_buf, 0, uoid_buf_sz);
	memcpy(uoid_buf, sha1_buf, min((unsigned int)uoid_buf_sz, sizeof(sha1_buf)));
}

vector<char*> read_file(char* filename)
{
	vector<char*> file_data;
	ifstream inData;

	inData.open(filename, ios::in);
	if(!inData)
	{
		printf("the file %s can not be opened\n", filename);
		inData.close();
		exit(1); 
	}
	//buffer to hold one line data 
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	while(inData.getline(str, MAXDATASIZE))
	{
		trimString(str);
		int len = strlen(str);
		if(len == 0)
			continue;
		char* data_str = new char[len];
		memset(data_str, 0, len);
		strcpy(data_str, str);
		file_data.push_back(data_str);
		memset(str, 0, MAXDATASIZE);
	}	
	
	inData.close();
	return file_data;
}

vector<string> readFile(char* filename)
{
	vector<string> file_data;
	
	FILE* pFile = fopen(filename, "r");
	if(!pFile)
	{
		fprintf(stderr, "cannot open file %s\n", filename);
		return file_data;
	}
	char str[MAXDATASIZE];
	memset(str, 0, MAXDATASIZE);
	while(!feof(pFile))
	{
		memset(str, 0, MAXDATASIZE);
		if(fgets(str, MAXDATASIZE, pFile) != NULL)
		{
			trimString(str);
			int len = strlen(str);
			if(len == 0)
				continue;
			memset(&str[len - 2], 0, 2);	
			string s(str);
			file_data.push_back(s);	
		}
	}
	return file_data;
}

void trimString(char* str)
{
	char temp[MAXDATASIZE];
	memset(temp, 0, MAXDATASIZE);
	int len = strlen(str);
	int start = 0;
	int end = len - 1;
	for(; start < len; ++start)
	{
		if(str[start] != ' ')
			break;
	}
	for(; end >= 0; --end)
	{
		if(str[end] != ' ')
			break;
	}
	strncpy(temp, str + start, end - start + 1);
	memset(str, 0, len);
	strcpy(str, temp);
}

void parseLine(const char* str, char* prefix, char* data)
{	
	char* pch = strchr(str, '=');
	strncpy(prefix, str, pch - str);
	trimString(prefix);
	strcpy(data, pch + 1);
	trimString(data);
}

void parseBeacon(const char* str, char* hostname, char* port)
{
	memset(hostname, 0, strlen(str));
	memset(port ,0, 6);
	char* pch = strchr(str, ':');
	strncpy(hostname, str, pch - str);
	strcpy(port, pch + 1);
}

vector<string> tokenize(string text)
{
    size_t last_pos = 0;
    size_t pos = 0;
	text.erase(0, text.find_first_not_of(" "));
	text.erase(text.find_last_not_of(" ") + 1);
    vector<string> vectorWords;
    while(pos != string::npos)
    {
        pos=text.find_first_of(" ", last_pos);
        vectorWords.push_back(text.substr(last_pos, pos-last_pos));
        last_pos = pos+1;
    }
    #ifdef _DEBUG_SUC_
    	for(int i = 0; i < (int)vectorWords.size(); ++i)
    		printf("text: %s\n", vectorWords[i].c_str());
    #endif	
    return vectorWords;
}

uint32_t getFilesize(const char* filename)
{
	struct stat st;
	int status = stat(filename, &st);
	if(status < 0)
	{	
		fprintf(stderr, "cannot get file %s size\n", filename);
		return 0;
	}	
	return (uint32_t)st.st_size;
}	

void stringToHex(string s, unsigned char* hex)
{
	char* pEnd;
	for(size_t i = 0; i < s.length() / 2; i++)
	{
		long int rv = strtol(s.substr(i * 2, 2).c_str(), &pEnd, 16);
		hex[i] = (unsigned char)rv;
	}		
}

void parseKeywords(const string& word, vector<string>& keywords)
{
	size_t len = word.length();
	size_t first = 0;
	size_t second = 0;
	while(second != len)
	{
		if(word[second] == '=' || word[second] == '\"' || word[second] == ' ')
		{
			if(first != second)
			{
				string str(word, first, second - first);
				transform(str.begin(),str.end(),str.begin(), ::tolower);
				keywords.push_back(str);
				first = second;
			}
			first++;
			second++;
		}
		else
		{
			second++;	
		}	
	}
	if(first != second)
	{
		string str(word, first, second - first);
		transform(str.begin(),str.end(),str.begin(), ::tolower);
		keywords.push_back(str);
		first = second;
	}
}

UOID_Map:: UOID_Map()
{
	memset(m_UOID, 0, 20);
}

UOID_Map:: UOID_Map(unsigned char *uoid)
{
	memset(m_UOID, 0, 20);
	memcpy(this->m_UOID, uoid, 20);
}

UOID_Map:: UOID_Map(const UOID_Map& other)
{
	memset(this->m_UOID, 0, 20);
	memcpy(this->m_UOID, other.m_UOID, 20);
}

UOID_Map& UOID_Map:: operator= (const UOID_Map& other)
{
	if(this == &other)
		return *this;
	
	memset(this->m_UOID, 0, 20);	
	memcpy(this->m_UOID, other.m_UOID, 20);	
	return *this;	
}

bool UOID_Map:: operator< (const UOID_Map& other) const
{
	return (memcmp(this->m_UOID, other.m_UOID, 20) < 0);
}

void create_metaDataFilename(char* filename, char* homeDir, uint32_t tmpFileIndex)
{
	char metaDataFileName[20];
	memset(metaDataFileName , 0, 20);
	sprintf(metaDataFileName, "%d.meta", tmpFileIndex);
	char path[strlen(homeDir) + strlen("/files/") + strlen(metaDataFileName) + 1];
	memset(path, 0, strlen(homeDir) + strlen("/files/") + strlen(metaDataFileName) + 1);
	strcpy(path, homeDir);
	strcat(path, "/files/");
	strcat(path, metaDataFileName);
	strcpy(filename, path);
}

void create_dataFilename(char* filename, char* homeDir, uint32_t tmpFileIndex)
{
	char out_filename[20];
	memset(out_filename, 0, 20);
	sprintf(out_filename, "%d.data", tmpFileIndex);
	char path[strlen(homeDir) + strlen("/files/") + strlen(out_filename) + 1];
	memset(path, 0, strlen(homeDir) + strlen("/files/") + strlen(out_filename) + 1);
	strcpy(path, homeDir);
	strcat(path, "/files/");
	strcat(path, out_filename);
	strcpy(filename, path);
}

void create_passFilename(char* filename, char* homeDir, uint32_t tmpFileIndex)
{
	char out_filename[20];
	memset(out_filename, 0, 20);
	sprintf(out_filename, "%d.pass", tmpFileIndex);
	char path[strlen(homeDir) + strlen("/files/") + strlen(out_filename) + 1];
	memset(path, 0, strlen(homeDir) + strlen("/files/") + strlen(out_filename) + 1);
	strcpy(path, homeDir);
	strcat(path, "/files/");
	strcat(path, out_filename);
	strcpy(filename, path);
}

void create_tempMetaDataFilename(char* filename, char* homeDir, uint32_t tempMataDataIndex)
{
	char out_filename[30];
	memset(out_filename, 0, 30);	
	sprintf(out_filename, "%d.tempMeta", tempMataDataIndex);
	char path[strlen(homeDir) + strlen("/tempMeta/") + strlen(out_filename) + 1];
	memset(path, 0, strlen(homeDir) + strlen("/tempMeta/") + strlen(out_filename) + 1);
	strcpy(path, homeDir);
	strcat(path, "/tempMeta/");
	strcat(path, out_filename);
	strcpy(filename, path);
}

SearchRes_Elm:: SearchRes_Elm(): tempMataIndex(0) {}

SearchRes_Elm::	SearchRes_Elm(uint32_t temp_index, UOID_Map uoid_fileID)
{
	this->tempMataIndex = temp_index;
	this->fileID_uoid = uoid_fileID;
}

SearchRes_Elm::	SearchRes_Elm(const SearchRes_Elm& other)
{
	this->tempMataIndex = other.tempMataIndex;
	this->fileID_uoid = other.fileID_uoid;
}

SearchRes_Elm& SearchRes_Elm:: operator= (const SearchRes_Elm& other)
{
	if(this == &other)
		return *this;
	
	this->tempMataIndex = other.tempMataIndex;
	this->fileID_uoid = other.fileID_uoid;
	return *this;	
}

