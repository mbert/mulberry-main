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


// Header for CURLHelpersMap class

#ifndef __CURLHELPERSMAP__MULBERRY__
#define __CURLHELPERSMAP__MULBERRY__

#include "cdstring.h"

//Extentions are saved with the leading dot
class CURLHelpersMap
{
 public:
	static CURLHelpersMap sURLHelpersMap;
	
	CURLHelpersMap() { }

	static void LoadDefaults(const cdstring& path);
	static bool LaunchURL(const char* url);
	static void SetupHelp();

	void ReadFromStream(std::istream& ins);

protected:
	cdstrmap 	mHelpers;
};

#endif
