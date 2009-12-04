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
	CHTTPUtils.cpp

	Author:			
	Description:	<describe the CHTTPUtils class here>
*/

#include "CHTTPUtils.h"

#include <cctype>

using namespace http; 

uint32_t CHTTPUtils::ParseStatusLine(const cdstring& line)
{
	uint32_t result = 0;

	const char* p = line.c_str();
	
	// Must have 'HTTP/' version at start
	if ((*p++ != 'H') ||
		(*p++ != 'T') ||
		(*p++ != 'T') ||
		(*p++ != 'P') ||
		(*p++ != '/'))
		return result;
	
	// Must have version '1.1 '
	if ((*p++ != '1') ||
		(*p++ != '.') ||
		(*p++ != '1') ||
		(*p++ != ' '))
		return result;
	
	// Must have three digits
	if (!isdigit(p[0]) || !isdigit(p[1]) || !isdigit(p[2]) || (p[3] != ' '))
		return result;
	
	// Read in the status code
	result = (*p++ - '0') * 100;
	result += (*p++ - '0') * 10;
	result += (*p++ - '0');
	
	return result;
}
