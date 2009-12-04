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


// Source for CDateControl class

#include "CDateControl.h"

#include "CRFC822.h"

#include <JXInputField.h>
#include "CInputField.h"
#include <JXStaticText.h>

#include <cassert>
#include <stdlib.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S
// Default constructor
CDateControl::CDateControl(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mDayFirst = true;

	// Create child widgets here
	OnCreate();
}

// Default destructor
CDateControl::~CDateControl()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDateControl::OnCreate()
{
// begin JXLayout1

    mText1 =
        new CInputField<JXInputField>(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 25,20);
    assert( mText1 != NULL );

    mSeparator1 =
        new JXStaticText("/", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 28,0, 14,20);
    assert( mSeparator1 != NULL );

    mText2 =
        new CInputField<JXInputField>(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 45,0, 25,20);
    assert( mText2 != NULL );

    mSeparator2 =
        new JXStaticText("/", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 73,0, 14,20);
    assert( mSeparator2 != NULL );

    mText3 =
        new CInputField<JXInputField>(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 90,0, 40,20);
    assert( mText3 != NULL );

// end JXLayout1

	InitDate();
}

// Use locale date settings
void CDateControl::InitDate()
{
	// Get string time in locale format
	char s[64];
	time_t now = ::time(NULL);
	struct tm* now_tm = ::localtime(&now);
	now_tm->tm_mon = 0;
	now_tm->tm_mday = 31;
	::strftime(s, 64, "%x", now_tm);

	// If first character is a digit assuming day is first in this lcoale
	mDayFirst = (*s == '3');

	// For now always use / as date separator
	mSeparator1->SetText("/");
	mSeparator2->SetText("/");

	// Set the appropriate pointers for day and month widgets
	if (mDayFirst)
	{
		mDay = mText1;
		mMonth = mText2;
	}
	else
	{
		mDay = mText2;
		mMonth = mText1;
	}
	
	SetDate(::time(NULL));
}

void CDateControl::SetDate(time_t date)
{
	struct tm* tms = ::localtime(&date);
	
	cdstring str;
	str = static_cast<long>(tms->tm_mday);
	mDay->SetText(str);
	
	str = static_cast<long>(tms->tm_mon + 1);
	mMonth->SetText(str);
	
	str = static_cast<long>(tms->tm_year + 1900);
	mText3->SetText(str);
}

time_t CDateControl::GetDate() const
{
	cdstring date;

	date += cdstring(::atol(mDay->GetText().GetCString()));
	date += "-";
	
	switch(::atol(mMonth->GetText().GetCString()))
	{
	case 1:
		date += "Jan";
		break;
	case 2:
		date += "Feb";
		break;
	case 3:
		date += "Mar";
		break;
	case 4:
		date += "Apr";
		break;
	case 5:
		date += "May";
		break;
	case 6:
		date += "Jun";
		break;
	case 7:
		date += "Jul";
		break;
	case 8:
		date += "Aug";
		break;
	case 9:
		date += "Sep";
		break;
	case 10:
		date += "Oct";
		break;
	case 11:
		date += "Nov";
		break;
	case 12:
		date += "Dec";
		break;
	}
	date += "-";

	date += cdstring(::atol(mText3->GetText().GetCString()));

	return CRFC822::ParseDate(date);
}
