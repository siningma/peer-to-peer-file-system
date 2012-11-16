#include "meta.h"

Meta:: Meta(const char* filename)
{
	this->filename = new char[strlen(filename) + 1];
	memset(this->filename, 0, strlen(filename) + 1);
	strcpy(this->filename, filename);
		
	fileSize = 0;
	memset(fileHash, 0, SHA_DIGEST_LENGTH);
	memset(password, 0, SHA_DIGEST_LENGTH);
	memset(nonce, 0, SHA_DIGEST_LENGTH);
}

Meta:: ~Meta()
{
	delete[] filename;
}

int Meta:: init(char* node_instance_id, vector<string>& keywords)
{
	fileSize = getFilesize(filename);
		
	//get this file SHA1
	int rv = do_sha1_file(filename, fileSize, fileHash);
	if(rv == -1)
		return -1;
	
	//get the nonce
	char obj[] = "password";
	GetUOID(node_instance_id, obj, (char*)password, UOID_LEN);
	//get file nonce from password
	SHA1(password, UOID_LEN, nonce);
	
	//create bitvector from keywords
	this->keywords = keywords;
	for(int i = 0; i < (int)this->keywords.size(); ++i)
	{
		//mark each keyword into bit vector
		bitvector.keyword_bitvector(this->keywords[i]);
	}
	return 0;
}

void Meta:: write_to_metadata(uint32_t tmpFileIndex, char* homeDir)
{
	char metaDataFileName[256];
	memset(metaDataFileName, 0, 256);
	create_metaDataFilename(metaDataFileName, homeDir, tmpFileIndex);
	
	FILE* metaDataFile = fopen(metaDataFileName, "w+");
	if(!metaDataFile) 
	{
		fprintf(stderr, "cannot create file %s\n", metaDataFileName);
		return;
	}
	
	fprintf(metaDataFile, "[metadata]\r\n");
	fprintf(metaDataFile, "FileName=%s\r\n", filename);
	fprintf(metaDataFile, "FileSize=%d\r\n", fileSize);
	fprintf(metaDataFile, "SHA1=");
	for(int i = 0; i < UOID_LEN; i++)
    	fprintf(metaDataFile, "%02x", fileHash[i]);	
    fprintf(metaDataFile, "\r\n");
    fprintf(metaDataFile, "Nonce=");	
    for(int i = 0; i < UOID_LEN; i++)
    	fprintf(metaDataFile, "%02x", nonce[i]);		
    fprintf(metaDataFile, "\r\n");
    fprintf(metaDataFile, "Keywords=");
    for(int i = 0; i < (int)keywords.size(); ++i)
    	fprintf(metaDataFile, "%s ", keywords[i].c_str());
    fprintf(metaDataFile, "\r\n");
    fprintf(metaDataFile, "Bit-vector=");
    unsigned char hex[BITVECTOR_LEN];
    memset(hex, 0, BITVECTOR_LEN);
    bitvector.getHex(hex);
    for(int i = 0; i < BITVECTOR_LEN; ++i)
    	fprintf(metaDataFile, "%02x", hex[i]);
    fprintf(metaDataFile, "\r\n");	
	fflush(metaDataFile);    
	fclose(metaDataFile);			
}

void Meta:: write_to_passFile(uint32_t tmpFileIndex, char* homeDir)
{
	char passFileName[256];
	memset(passFileName, 0, 256);
	create_passFilename(passFileName, homeDir, tmpFileIndex);
	
	FILE* passFile = fopen(passFileName, "w+");
	if(!passFile) 
	{
		fprintf(stderr, "cannot create file %s\n", passFileName);
		return;
	}
	
	for(int i = 0; i < UOID_LEN; i++)
    	fprintf(passFile, "%02x", password[i]);
	fprintf(passFile, "\r\n");
	fflush(passFile);
	fclose(passFile);
}

int do_sha1_file(char* filename, uint32_t fileSize, unsigned char* checksum)
{
	SHA_CTX c;
	memset(&c, 0, sizeof(SHA_CTX));
	unsigned long normal_time = fileSize / MAX_BUF_SIZE;
	unsigned long last_time = fileSize % MAX_BUF_SIZE;
	
	FILE* pf = fopen(filename, "r");
	if(!pf)
	{
		fprintf(stderr, "cannot open file %s\n", filename);
		return -1;
	}	
	fseek(pf, 0, SEEK_SET);
	SHA1_Init(&c);
	
	char buf[MAX_BUF_SIZE];
	memset(buf, 0, MAX_BUF_SIZE);	
	for(unsigned long i = 0; i < normal_time; i++)
	{
		memset(buf, 0, MAX_BUF_SIZE);
		fread(buf, 1, MAX_BUF_SIZE, pf);
		SHA1_Update(&c, buf, MAX_BUF_SIZE);
	}
	
	memset(buf, 0, MAX_BUF_SIZE);
	fread(buf, 1, (size_t)last_time, pf);
	SHA1_Update(&c, buf, last_time);
	SHA1_Final(checksum, &c);
	
	fclose(pf);
	return 0;
}

int do_md5_file(char* filename, uint32_t fileSize, unsigned char* out)
{
	return 0;
}
