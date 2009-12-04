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


// Header for CMailcapMap class

#ifndef __CMAILCAPSMAP__MULBERRY__
#define __CMAILCAPSMAP__MULBERRY__

#include "cdstring.h"

#include <map>
#include <istream>

// Extensions are saved with the leading dot
class CMailcapMap
{
 public:
	static CMailcapMap sMailcapMap;
	
	CMailcapMap() {}

	static void LoadDefaults(const cdstring& path);

	//Both return "" if no match
	cdstring GetAppName(const cdstring& type);
	const cdstring& GetCommand(const cdstring& type);
		
	void AddEntry(const cdstring& mimetype, const cdstring& app);
	void ReadFromStream(std::istream& ins);

protected:
	cdstrmap mTypeToApp;
};

#endif
