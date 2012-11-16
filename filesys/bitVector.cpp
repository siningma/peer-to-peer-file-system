#include "bitVector.h"

BitVector:: BitVector()
{
	memset(bit_vector, 0, BITVECTOR_LEN);
}

BitVector::	BitVector(unsigned char* bitvector)
{
	memset(bit_vector, 0, BITVECTOR_LEN);
	memcpy(bit_vector, bitvector, BITVECTOR_LEN);
}

BitVector::	BitVector(const BitVector& other)
{
	memset(this->bit_vector, 0, BITVECTOR_LEN);
	memcpy(this->bit_vector, other.bit_vector, BITVECTOR_LEN);
}

BitVector& BitVector:: operator= (const BitVector& other)
{
	if(this == &other)
		return *this;
		
	memset(this->bit_vector, 0, BITVECTOR_LEN);
	memcpy(this->bit_vector, other.bit_vector, BITVECTOR_LEN);	
	return *this;
}

bool BitVector:: operator< (const BitVector& other) const
{
	return (memcmp(this->bit_vector, other.bit_vector, BITVECTOR_LEN) < 0);
}

void BitVector:: setBitVector(unsigned char* bitvector)
{
	memcpy(bit_vector, bitvector, BITVECTOR_LEN);
}
	
void BitVector:: keyword_bitvector(string s)
{
	wordSHA1(s.c_str());
	wordMD5(s.c_str());
	
	#ifdef _DEBUG_
		printf("keyword: %s\n", s.c_str());
		print_bitvector();
	#endif
}

void BitVector:: wordSHA1(const char* word)
{
	unsigned char sha1_buf[SHA_DIGEST_LENGTH];
	memset(sha1_buf, 0, SHA_DIGEST_LENGTH);
	SHA1((unsigned char*)word, strlen(word), sha1_buf);
	unsigned short trail = 0;
	trail |= sha1_buf[SHA_DIGEST_LENGTH - 1];
	trail |= ((sha1_buf[SHA_DIGEST_LENGTH - 2] & 0x01) << 8);
	trail += 512;
	unsigned short entry = trail >> 3;
	unsigned short offset = trail & 0x07;
	mark(entry, offset);
}

void BitVector:: wordMD5(const char* word)
{
	unsigned char md5_buf[MD5_DIGEST_LENGTH];
	memset(md5_buf, 0, MD5_DIGEST_LENGTH);
	MD5((unsigned char*)word, strlen(word), md5_buf);
	unsigned short trail = 0;
	trail |= md5_buf[MD5_DIGEST_LENGTH - 1];
	trail |= ((md5_buf[MD5_DIGEST_LENGTH - 2] & 0x01) << 8);
	unsigned short entry = trail >> 3;
	unsigned short offset = trail & 0x07;
	mark(entry, offset);
}
	
void BitVector:: mark(unsigned short entry, unsigned short offset)
{
	bit_vector[BITVECTOR_LEN - 1 - entry] |= (1 << offset);
}

bool BitVector:: match(const BitVector& bv)
{
	for(int i = 0; i < BITVECTOR_LEN; ++i)
	{
		if(this->bit_vector[i] & bv.bit_vector[i])
			return true;
	}			
	return false;
}
	
void BitVector:: getHex(unsigned char* hex)
{
	memcpy(hex, bit_vector, BITVECTOR_LEN);
}

void BitVector:: print_bitvector()
{
	printf("Bit vector:\n");
	for(int i = 0; i < BITVECTOR_LEN; ++i)
	{
		printf("%02x", bit_vector[i]);
	}
	printf("\n");
}
