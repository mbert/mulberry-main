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


// COSStringMap : class to implement extra preference items in a compound preference

#ifndef __COSSTRINGMAP__MULBERRY__
#define __COSSTRINGMAP__MULBERRY__

#include "cdstring.h"

class COSStringMap : public cdstrmap
{
public:
	COSStringMap() {}
	~COSStringMap() {}

	// Get/set
	const cdstring& GetData() const;
	void SetData(const cdstring& txt);

	// Read/write prefs
	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);
};

#endif
