#ifndef META_H
#define META_H

#include "../system/tool.h"
#include "bitVector.h"

using namespace std;

class Meta 
{
	public:
	char* filename;
	uint32_t fileSize;
	unsigned char password[UOID_LEN];	
	unsigned char fileHash[SHA_DIGEST_LENGTH];
	unsigned char nonce[SHA_DIGEST_LENGTH];
	vector<string> keywords;
	BitVector bitvector;
	
	Meta(const char* filename);
	~Meta();
	int init(char* node_instance_id, vector<string>& keywords);
	void write_to_metadata(uint32_t tmpFileIndex, char* homeDir);
	void write_to_passFile(uint32_t tmpFileIndex, char* homeDir);
};

extern int do_sha1_file(char* filename, uint32_t fileSize, unsigned char* checksum);
extern int do_md5_file(char* filename, uint32_t fileSize, unsigned char* out);

#endif
