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


//	CDataAttachment.cp

#include "CDataAttachment.h"

#include "CStringUtils.h"

// Default constructor
CDataAttachment::CDataAttachment()
{
}

// Create text body
CDataAttachment::CDataAttachment(char* text)
	: CAttachment(text)
{
	mContent.SetContentDisposition(eContentDispositionInline);

	mContent.SetContentSize(text ? ::strlen(text) : 0);
}

// Copy constructor
CDataAttachment::CDataAttachment(const CDataAttachment& copy)
	: CAttachment(copy)
{
	// Copy data as well
	if (copy.mData)
		mData = ::strdup(copy.mData);
}

// Destructor
CDataAttachment::~CDataAttachment()
{
}

// Check for content that Mulberry can display: only text/xxx at the moment
bool CDataAttachment::CanDisplay() const
{
	return (mContent.GetContentType() == eContentText);
}

// Check for content that Mulberry can edit: any text/xxx except text/calendar
bool CDataAttachment::CanEdit() const
{
	return (mContent.GetContentType() == eContentText) &&
			(mContent.GetContentSubtype() != eContentSubCalendar);
}

