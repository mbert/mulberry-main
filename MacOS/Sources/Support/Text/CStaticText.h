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


// Header for CStaticText class

#ifndef __CStaticText__MULBERRY__
#define __CStaticText__MULBERRY__

#include <LStaticText.h>

// Constants

// Classes
class cdstring;
class cdustring;

class CStaticText : public LStaticText
{
public:
	enum { class_ID = 'Utxt' };

					CStaticText(LStream *inStream);
	virtual 		~CStaticText();

			void	SetText(const cdstring& txt);				// UTF8 in
			void	SetText(const char* txt, size_t size = -1);	// UTF8 in
			void	SetText(const cdustring& txt);				// UTF16 in
			void	SetNumber(long num);						// Number in

			void	GetText(cdstring& txt) const;				// UTF8 out
		cdstring	GetText() const;							// UTF8 out

			void	SetWrap(bool wrap);

protected:
	virtual void		FinishCreateSelf();

};

#endif
