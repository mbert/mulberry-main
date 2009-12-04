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


// CCaptionParser.h - parses tagged caption input by user

#ifndef __CCAPTIONPARSER__MULBERRY__
#define __CCAPTIONPARSER__MULBERRY__

#include "cdstring.h"

extern const char* cCaptions[];

class CAddress;
class CAddressList;
class CMessage;

class CCaptionParser
{
public:
	enum EMode
	{
		ePercent = 0,
		eMe,
		eMeFirst,
		eMeEmail,
		eMeFull,
		eDateNow,
		eSmart,
		eSmartFirst,
		eSmartEmail,
		eSmartFull,
		eFrom,
		eFromFirst,
		eFromEmail,
		eFromFull,
		eTo,
		eToFirst,
		eToEmail,
		eToFull,
		eCC,
		eCCFirst,
		eCCEmail,
		eCCFull,
		eSubject,
		eDateSentFull,
		eDateSentShort,
		ePage
	};

	static cdstring ParseCaption(const cdstring& caption, const CMessage* msg, bool multi = true, unsigned long page_num = 0);

private:
	static bool PutAddressListCaption(std::ostream& out, EMode mode, const CAddressList& addrs, bool multi);
	static void PutAddressCaption(std::ostream& out, EMode mode, const CAddress& addr);
};

#endif
