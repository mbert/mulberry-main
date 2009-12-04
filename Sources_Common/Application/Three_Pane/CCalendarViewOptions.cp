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


// Source for CCalendarViewOptions class

#include "CCalendarViewOptions.h"

#include "CPreferenceVersions.h"
#include "CUtils.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CCalendarViewOptions

// Static

// Static members

// Default constructor
CCalendarViewOptions::CCalendarViewOptions()
{
	mShowPreview = true;
}

// Default destructor
CCalendarViewOptions::~CCalendarViewOptions()
{
}

// Default constructor
void CCalendarViewOptions::_copy(const CCalendarViewOptions& copy)
{
	mShowPreview = copy.mShowPreview;
}

// Compare with same type
int CCalendarViewOptions::operator==(const CCalendarViewOptions& comp) const
{
	return (mShowPreview == comp.mShowPreview);
}

cdstring CCalendarViewOptions::GetInfo() const
{
	cdstring details;

	details += mShowPreview ? cValueBoolTrue : cValueBoolFalse;
	
	return details;
}

bool CCalendarViewOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mShowPreview);

	return true;
}
