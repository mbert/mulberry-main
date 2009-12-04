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


// ctrbuf.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 21-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a streambuf that counts bytes sent to it.
//
// History:
// 21-Nov-1997: Created initial header and implementation.
//

#ifndef __CTRBUF__MULBERRY__
#define __CTRBUF__MULBERRY__

#include <iostream>

class ctrbuf : public std::streambuf
{
public:
    ctrbuf();
    virtual ~ctrbuf();

	size_t get_size(void)
		{ return ctr; }
	void reset(void)
		{ ctr = 0; }
protected:
     virtual std::streamsize xsputn (const char_type* s, std::streamsize n);
     virtual int_type overflow (int_type c=traits_type::eof ());
private:
    size_t ctr;
};

inline ctrbuf::ctrbuf() : std::streambuf()
{
	ctr = 0;
}

inline ctrbuf::~ctrbuf()
{
}

inline ctrbuf::int_type ctrbuf::overflow (int_type c)
{
    if (traits_type::eq_int_type (c, traits_type::eof ()))
        return traits_type::not_eof (c);

	ctr++;
    return (int_type) c;
}

inline std::streamsize ctrbuf::xsputn (const char_type* s, std::streamsize n)
{
	ctr += n;
	return n;
}

#endif
