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

/*
 * convert the input string to base64
 * resulting string in null terminated
 */
void kbase64_to64(unsigned char *out, unsigned char *in,int inlen);

/*
 * convert null terminated base64 input string to binary
 * **** WARNING **** conversion can be in place if desired
 * restorying the input string
 * returns the length of the binary output
 * if input string begins with typical imap "+ " (plus space) that is skipped
 */
int kbase64_from64(char *out, char *in);
