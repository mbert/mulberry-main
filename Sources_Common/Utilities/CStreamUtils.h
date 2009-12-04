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


// Header for iostream utilities

#ifndef __CSTREAMUTILS__MULBERRY__
#define __CSTREAMUTILS__MULBERRY__

#include <iosfwd>
#include "cdstring.h"
#include "templs.h"

class LStream;

const int cDefaultCopyBufferSize = 4096;

void StreamCopy(std::istream& in, std::ostream& out,
				unsigned long start, unsigned long length,
				unsigned long buf_size = cDefaultCopyBufferSize);
void StreamCopyIO(std::iostream& inout, std::ostream& out,
				unsigned long start, unsigned long length,
				unsigned long buf_size = cDefaultCopyBufferSize);
void StreamCopy(std::istream& in, LStream& out,
				unsigned long start, unsigned long length,
				unsigned long buf_size = cDefaultCopyBufferSize);
unsigned long StreamLength(std::istream& in);

// Network/host byte order stream i/o
void WriteHost(std::ostream& out, const unsigned long& data);
void WriteHost(std::ostream& out, const ulvector& data);

void ReadHost(std::istream& in, unsigned long& data);
void ReadHost(std::istream& in, long& data);
void ReadHost(std::istream& in, ulvector* data);

// I18L i/o
void Write1522(std::ostream& out, const cdstring& text);
void Read1522(std::istream& in, cdstring& text);

#endif
