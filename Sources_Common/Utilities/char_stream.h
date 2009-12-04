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


// char_stream.h

// Handles parsing a stream of text typically found in preferences

#ifndef __CHAR_STREAM__MULBERRY__
#define __CHAR_STREAM__MULBERRY__

#include <stddef.h>

class cdstring;

class char_stream
{
public:
	char_stream(char* p)
		{ mTxt = p; mPutback = 0; }
	~char_stream() 
		{ mTxt = NULL; }
	
	char operator*()
		{ return (mTxt ? *mTxt : 0); }
	char_stream& operator+=(int i)			// Bump position
		{ mTxt += i; return *this; }

	operator char*()						// Access the whole string
		{ return mTxt; }

	char* get();							// Get atom or quoted string
	void get(cdstring& copy, bool convert = false);	// Copy atom or quoted string
	void get(bool& copy);					// Copy bool
	void get(long& copy);					// Copy long
	void get(unsigned long& copy);			// Copy unsigned long

	bool start_sexpression();				// Check for start of (...) items
	bool end_sexpression();					// End of (...) items
	bool test_start_sexpression();			// Test for start of (...) items
	bool test_end_sexpression();			// Test for end of (...) items

private:
	char* mTxt;
	char  mPutback;
	
	void putback();							// Handle the putback character
};

#endif
