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
#include "CStaticText.h"
#include "CTextFieldX.h"

#include <LLittleArrows.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S
// Default constructor
CDateControl::CDateControl()
{
	mDayFirst = true;
}

// Constructor from stream
CDateControl::CDateControl(LStream *inStream)
		: LView(inStream)
{
	mDayFirst = true;
}

// Default destructor
CDateControl::~CDateControl()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDateControl::FinishCreateSelf(void)
{
	// Do inherited
	LView::FinishCreateSelf();

	// Get controls
	mText1 = (CTextFieldX*) FindPaneByID(paneid_DateControlText1);
	mSeparator1 = (CStaticText*) FindPaneByID(paneid_DateControlSeparator1);
	mText2 = (CTextFieldX*) FindPaneByID(paneid_DateControlText2);
	mSeparator2 = (CStaticText*) FindPaneByID(paneid_DateControlSeparator2);
	mText3 = (CTextFieldX*) FindPaneByID(paneid_DateControlText3);
	mArrows = (LLittleArrows*) FindPaneByID(paneid_DateControlArrows);

	InitDate();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CDateControlBtns);
}

// Handle buttons
void CDateControl::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_DateControlArrows:
		;
	}
}

// Use locale date settings
void CDateControl::InitDate()
{
	Intl0Hndl hdl = (Intl0Hndl) ::GetIntlResource(0);

	char sep[2] = "/";

	if (hdl)
	{
		if ((**hdl).dateSep)
			sep[0] = (**hdl).dateSep;

		mDayFirst = ((**hdl).dateOrder == dmy) || ((**hdl).dateOrder == dym) || ((**hdl).dateOrder == ydm);
	}

	mSeparator1->SetText(sep);
	mSeparator2->SetText(sep);

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

	SetDate(::time(nil));
}

void CDateControl::SetDate(time_t date)
{
	struct tm* tms = ::localtime(&date);

	mDay->SetNumber(tms->tm_mday);

	mMonth->SetNumber(tms->tm_mon + 1);

	mText3->SetNumber(tms->tm_year + 1900);
}

time_t CDateControl::GetDate() const
{
	cdstring date;

	date += cdstring(mDay->GetNumber());
	date += "-";

	switch(mMonth->GetNumber())
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

	date += cdstring(mText3->GetNumber());

	return CRFC822::ParseDate(date);
}