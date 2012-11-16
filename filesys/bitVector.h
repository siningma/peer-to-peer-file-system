#ifndef BITVECTOR_H
#define BITVECTOR_H

#include "../system/tool.h"

class BitVector 
{
	public:
	unsigned char bit_vector[BITVECTOR_LEN];
	
	BitVector();
	BitVector(unsigned char* bitvector);
	BitVector(const BitVector& other);
	BitVector& operator= (const BitVector& other);
	bool operator<(const BitVector& other) const;
	
	void setBitVector(unsigned char* bitvector);
	void keyword_bitvector(string s);
	void wordSHA1(const char* word);
	void wordMD5(const char* word);
	void mark(unsigned short entry, unsigned short offset);
	bool match(const BitVector& bv);
	void getHex(unsigned char* hex);	
	void print_bitvector();
};

#endif
