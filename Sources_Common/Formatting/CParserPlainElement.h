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


#ifndef __CPARSERPLAINELEMENT__MULBERRY__
#define __CPARSERPLAINELEMENT__MULBERRY__

#include "cdstring.h"
#include "ptrvector.h"

class CParserPlainElement
{
public:
	CParserPlainElement();
	CParserPlainElement(const char*);
	~CParserPlainElement();
	
	unsigned int	mLength;
	cdstring 		mValue;
};

typedef ptrvector<CParserPlainElement> CParserPlainElementList;

#endif
