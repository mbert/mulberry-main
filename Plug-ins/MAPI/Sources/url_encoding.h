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

// URL Encoding/decoding

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern const char  cURLEscape;
extern const char* cURLReserved;
extern const char cURLUnreserved[];
extern const char cURLCharacter[];
extern const char cURLXCharacter[];


extern const char* cHexChar;
extern const unsigned char cFromHex[];

char* EncodeURL(const char* text);
char* EncodeURLX(const char* text, char ignore);
char* DecodeURL(const char* text);

#ifdef __cplusplus
}
#endif
