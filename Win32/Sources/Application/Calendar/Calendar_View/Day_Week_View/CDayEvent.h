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

#ifndef H_CDayEvent
#define H_CDayEvent
#pragma once

#include "CCalendarEventBase.h"

// ===========================================================================
//	CDayEvent

class	CDayEvent : public CCalendarEventBase
{
public:
	static CDayEvent* Create(CWnd* parent, const CRect& frame);

	CDayEvent();
						
	virtual ~CDayEvent() {}

	uint32_t GetColumnOffset() const
	{
		return mColumnOffset;
	}
	void SetColumnOffset(uint32_t offset)
	{
		mColumnOffset = offset;
	}
	
	uint32_t GetColumnTotal() const
	{
		return mColumnTotal;
	}
	void SetColumnTotal(uint32_t total)
	{
		mColumnTotal = total;
	}

	bool GetSpanToEdge() const
	{
		return mSpanToEdge;
	}
	void SetSpanToEdge(bool span)
	{
		mSpanToEdge = span;
	}

	float GetRelativeOffset() const
	{
		return mRelativeOffset;
	}
	void SetRelativeOffset(float offset)
	{
		mRelativeOffset = offset;
	}
	
	float GetRelativeWidth() const
	{
		return mRelativeWidth;
	}
	void SetRelativeWidth(float width)
	{
		mRelativeWidth = width;
	}
	
protected:
	uint32_t	mColumnOffset;
	uint32_t	mColumnTotal;
	bool		mSpanToEdge;
	float		mRelativeOffset;
	float		mRelativeWidth;
};

typedef std::vector<CDayEvent*> CDayEventList;

#endif
