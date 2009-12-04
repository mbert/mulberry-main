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


// Source for CStaticText class

#include "CStaticText.h"

#include "cdstring.h"
#include "cdustring.h"

#include "MyCFString.h"

#include <LControlImp.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CStaticText::CStaticText(LStream *inStream)
		: LStaticText(inStream)
{
}

// Default destructor
CStaticText::~CStaticText()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CStaticText::FinishCreateSelf()
{
	LStaticText::FinishCreateSelf();
	
	// Default to single line mode
	SetWrap(true);
}

void CStaticText::SetWrap(bool wrap)
{
	Boolean value = wrap;
	mControlImp->SetDataTag(0, kControlStaticTextIsMultilineTag,
							sizeof(Boolean), &value);
}

// Set all text from utf8
void CStaticText::SetText(const cdstring& all)
{
	cdustring utf16all(all);
	SetText(utf16all);
}

// Set all text from utf8
void CStaticText::SetText(const char* txt, size_t size)
{
	if ((txt != NULL) && (size == -1))
		size = ::strlen(txt);

	cdustring utf16all(txt, size);
	SetText(utf16all);
}

// Set all text from utf16
void CStaticText::SetText(const cdustring& all)
{
	PPx::CFString temp(static_cast<const UniChar*>(all.c_str()), all.length());
	SetCFDescriptor(temp);
	Refresh();
}

// Set number
void CStaticText::SetNumber(long num)
{
	cdstring txt(num);
	SetText(txt);
}

// Get all text as utf8
void CStaticText::GetText(cdstring& all) const
{
	MyCFString txt(CopyCFDescriptor());
	all = txt.GetString();
}

// Get all text as utf8
cdstring CStaticText::GetText() const
{
	cdstring result;
	GetText(result);
	return result;
}
