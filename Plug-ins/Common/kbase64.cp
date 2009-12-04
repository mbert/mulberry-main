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

/*
 * kbase64.h - routines to convert to and from base64 crom andrew cyrus project
 */

#include <stdlib.h>
#include "kbase64.h"

/* base64 tables
 */
static char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char index_64[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};
#define CHAR64(c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])

/*
 * convert the input string to base64
 * resulting string in null terminated
 */
void kbase64_to64(unsigned char *out, unsigned char *in,int inlen)
{
    unsigned char oval;
    
    while (inlen >= 3) {
		*out++ = basis_64[in[0] >> 2];
		*out++ = basis_64[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*out++ = basis_64[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*out++ = basis_64[in[2] & 0x3f];
		in += 3;
		inlen -= 3;
    }
    if (inlen > 0) {
		*out++ = basis_64[in[0] >> 2];
		oval = (in[0] << 4) & 0x30;
		if (inlen > 1) oval |= in[1] >> 4;
		*out++ = basis_64[oval];
		*out++ = (inlen < 2) ? '=' : basis_64[(in[1] << 2) & 0x3c];
		*out++ = '=';
    }
    *out = '\0';
}

/*
 * convert null terminated base64 input string to binary
 * **** WARNING **** conversion can be in place if desired
 * restorying the input string
 * returns the length of the binary output
 * if input string begins with typical imap "+ " (plus space) that is skipped
 */
int kbase64_from64(char *out, char *in)
{
    int len = 0;
    int c1, c2, c3, c4;

    if (in[0] == '+' && in[1] == ' ') in += 2;
    do {
    	if (*in == '\r') return (len);
    	if (in[0]==0)	return len;
		c1 = in[0];
		if (CHAR64(c1) == -1) return (-1);
		c2 = in[1];
		if (CHAR64(c2) == -1) return (-1);
		c3 = in[2];
		if (c3 != '=' && CHAR64(c3) == -1) return (-1); 
		c4 = in[3];
		if ((c4 != 0) && (c4 != '\r') && (c4 != '=') && (CHAR64(c4) == -1)) return (-1);
		in += 4;
		*out++ = (CHAR64(c1) << 2) | (CHAR64(c2) >> 4);
		++len;
		if (c3 != '=') {
		    *out++ = ((CHAR64(c2) << 4) & 0xf0) | (CHAR64(c3) >> 2);
		    ++len;
		    if (c4 != '=') {
				*out++ = ((CHAR64(c3) << 6) & 0xc0) | CHAR64(c4);
				++len;
		    }
		}
    } while ((c4 != '=')&&(c4 !='\r')&&(c4!=0));

    return (len);
}
