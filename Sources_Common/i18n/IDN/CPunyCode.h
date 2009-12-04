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


// Header for CPunyCode class

#ifndef __CPunyCode__MULBERRY__
#define __CPunyCode__MULBERRY__

#include "cdstring.h"

namespace idn 
{

class CPunyCode
{
public:
	static void Encode(cdstring& str);
	static void Decode(cdstring& str);

private:
	static bool ReadChar(const char** input, int bias, int& output);
	static int Adapt(int delta, int numpoints, bool firsttime);

};

}
#endif
