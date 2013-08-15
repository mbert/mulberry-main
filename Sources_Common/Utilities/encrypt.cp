/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

// Encryption/decryption

#include "encrypt.h"

#include "base64.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

// nibble_swap	:	swap nibbles in c-string
//
// value		:	data to encrypt/decrypt - will be overwritten with result
void nibble_swap(unsigned char* value);
void nibble_swap(unsigned char* value)
{
	unsigned char* p = value;
	while(*p)
    {
        unsigned char c = *p;
		*p++ = ((c >> 4) & 0x0F) | ((c << 4) & 0xF0);
    }
}

void encrypt_simple(char* value)
{
	nibble_swap((unsigned char*) value);
}

void decrypt_simple(char* value)
{
	nibble_swap((unsigned char*) value);
}

// cipher	:	do a Vigenere cipher
//
// value	:	data to encrypt/decrypt - will be overwritten with result
// vlen		:	length of data
// key		:	key to use for encytpion/decryption
// klen		:	length of key
// encrypt	:	true - encrypting; false - decrypting
//
void cipher(unsigned char* value, size_t vlen, const unsigned char* key, size_t klen, bool encrypt);
void cipher(unsigned char* value, size_t vlen, const unsigned char* key, size_t klen, bool encrypt)
{
	const unsigned char* q = key;
	size_t kklen = klen;
	while(vlen--)
	{
        unsigned char c = *value;
		if (encrypt)
			*value++ = c + *q;
		else
			*value++ = c - *q;

		// Bump key
		q++;
		if (!--kklen)
		{
			q = key;
			kklen = klen;
		}
	}
}

char* encrypt_cipher(const char* value, const char* key)
{
	size_t vlen = strlen(value);
	size_t klen = strlen(key);

	// Partial result before base64 encoding
	size_t rlen = vlen + 5;
	unsigned char* result = new unsigned char[rlen];

	// Get one-time key and store at start of result
	srand(::time(0L));
	unsigned long t = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
	*(long*) result = t;
	result[4] = 0;

	// One-time key must not contain nulls
	for(int i = 0; i < 4; i++)
		if (!result[i]) result[i] = i + 1;

	// Encrypt key with one-time key
	unsigned char* temp_key = new unsigned char[klen + 1];
	strcpy((char*) temp_key, key);
	::cipher(temp_key, klen, result, 4, true);

	// Encrypt one-time key with original key and store at start
	::cipher(result, 4, (unsigned char*) key, klen, true);

	// Encrypt value with encrypted key
	strcpy((char*) result + 4, value);
	::cipher(result + 4, vlen, temp_key, klen, true);

	// base64 encode result
	char* resultb64 = ::base64_encode(result, vlen + 4);

	// Clean up
	delete temp_key;
	delete result;

	return resultb64;
}

char* decrypt_cipher(const char* value, const char* key)
{
	size_t klen = strlen(key);

	// base64 decode input
	size_t rlen = 0;
	unsigned char* unbase64 = ::base64_decode(value, rlen);

	// Do validity check
	if (rlen <= 4)
	{
		delete unbase64;
		return 0L;
	}

	char* result = new char[rlen - 3];

	// Decrypt one-time key
	memcpy(result, unbase64, 4);
	result[4] = 0;
	::cipher((unsigned char*) result, 4, (unsigned char*) key, klen, false);

	// Encrypt key with one-time
	unsigned char* temp_key = new unsigned char[klen + 1];
	strcpy((char*) temp_key, key);
	::cipher(temp_key, klen, (unsigned char*) result, 4, true);

	// Decrypt value with encrypted one-time key
	memcpy(result, unbase64 + 4, rlen - 4);
	::cipher((unsigned char*) result, rlen - 4, temp_key, klen, false);
	result[rlen - 4] = 0;

	// Clean up
	delete temp_key;
	delete unbase64;

	return result;
}
