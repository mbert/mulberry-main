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


// Source for CDateTimeControl class

#include "CDateTimeControl.h"

#include "JXMultiImageButton.h"

#include <jASCIIConstants.h>
#include <jGlobals.h>

#include <stdio.h>
#include <stdlib.h>

CDateTimeControl::CDateTimeControl(const JCharacter* text, JXContainer* enclosure,
								JXTextMenu* menu,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
				: CTextInputField(text, enclosure, menu, hSizing, vSizing, x, y, w - 20, h)
{
}

CDateTimeControl::CDateTimeControl(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
				: CTextInputField(enclosure, hSizing, vSizing, x, y, w - 20, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDateTimeControl::OnCreate(EDateTimeMode dtmode)
{
	JFontStyle style;
	SetFont(JGetDefaultFontName(), kJDefaultFontSize, style);
	SetBreakCROnly(true);

	WantInput(kTrue, kTrue, kTrue);		// tab/shift-tab moves to next/previous item

	SetMode(dtmode);
	
	JRect frame = GetFrame();

	// Create spin up/down buttons
    mSpinUp =
        new JXMultiImageButton(const_cast<JXContainer*>(GetEnclosure()),
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, frame.right, frame.top, 20, frame.height() / 2);
	mSpinUp->SetImage(IDI_SPIN_UP);

    mSpinDown =
        new JXMultiImageButton(const_cast<JXContainer*>(GetEnclosure()),
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, frame.right, frame.top + frame.height() / 2, 20, frame.height() / 2);
	mSpinDown->SetImage(IDI_SPIN_DOWN);

	ListenTo(mSpinUp);
	ListenTo(mSpinDown);
}

// Handle OK button
void CDateTimeControl::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mSpinUp)
			Nudge(true);
		else if (sender == mSpinDown)
			Nudge(false);
	}
}

void CDateTimeControl::SetMode(EDateTimeMode dtmode)
{
	switch(dtmode)
	{
	case eDate:
	{
		// Determine whether to use DDMMYYYY or MMDDYYYY

		// Get string time in locale format
		char s[64];
		time_t now = ::time(NULL);
		struct tm* now_tm = ::localtime(&now);
		now_tm->tm_mon = 0;
		now_tm->tm_mday = 31;
		::strftime(s, 64, "%x", now_tm);

		// If first character is a digit assuming day is first in this lcoale
		dtmode = (*s == '3') ? eDateDDMMYYYY : eDateMMDDYYYY;
		break;
	}
	case eTimeSecs:
	{
		// Determine whether to use 12 or 24 hour time
		time_t date = ::time(NULL);
		struct tm* now_tm = ::localtime(&date);
		now_tm->tm_hour = 23;
		cdstring result;
		result.reserve(256);
		::strftime(result.c_str_mod(), 256,"%X", now_tm);

		dtmode = (result[0UL] == '2') ? e24TimeSecs : e12TimeSecs;
		break;
	}
	case eTimeNoSecs:
	{
		// Determine whether to use 12 or 24 hour time
		time_t date = ::time(NULL);
		struct tm* now_tm = ::localtime(&date);
		now_tm->tm_hour = 23;
		cdstring result;
		result.reserve(256);
		::strftime(result.c_str_mod(), 256,"%X", now_tm);

		dtmode = (result[0UL] == '2') ? e24TimeNoSecs : e12TimeNoSecs;
		break;
	}
	default:;
	}
	
	mMode = dtmode;
	InitDate();
}

// Use locale date settings
void CDateTimeControl::InitDate()
{
	// Get string time in locale format
	char s[64];
	time_t now = ::time(NULL);
	struct tm* now_tm = ::localtime(&now);

	switch(mMode)
	{
	case eDateDDMMYYYY:
	case eDateMMDDYYYY:
		SetDate(now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);
		break;
	case e12TimeSecs:
	case e12TimeNoSecs:
	case e24TimeSecs:
	case e24TimeNoSecs:
		SetTime(now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
		break;
	default:;
	}	
}

void CDateTimeControl::SetDate(unsigned long year, unsigned long month, unsigned long mday)
{
	cdstring buffer;
	buffer.reserve(32);
	switch(mMode)
	{
	case eDateDDMMYYYY:
		::snprintf(buffer.c_str_mod(), 32, "%02ld/%02ld/%ld", mday, month, year);
		break;
	case eDateMMDDYYYY:
		::snprintf(buffer.c_str_mod(), 32, "%02ld/%02ld/%ld", month, mday, year);
		break;
	default:;
	}
	if (!buffer.empty())
		SetText(buffer);
}

void CDateTimeControl::GetDate(unsigned long& year, unsigned long& month, unsigned long& mday) const
{
	cdstring buffer = GetText();
	
	cdstring t1(buffer, 0, 2);
	cdstring t2(buffer, 3, 2);
	cdstring t3(buffer, 6);

	unsigned long i1 = ::atoi(t1);
	unsigned long i2 = ::atoi(t2);
	unsigned long i3 = ::atoi(t3);

	switch(mMode)
	{
	case eDateDDMMYYYY:
		year = i3;
		month = i2;
		mday = i1;
		break;
	case eDateMMDDYYYY:
		year = i3;
		month = i1;
		mday = i2;
		break;
	default:;
	}
	
	// Special for typing of zero
	if (year == 0)
		year = 1;
	if (month == 0)
		month = 1;
	if (mday == 0)
		mday = 1;
}

void CDateTimeControl::SetTime(unsigned long hours, unsigned long mins, unsigned long secs)
{
	cdstring buffer;
	buffer.reserve(32);
	bool am = hours < 12;
	switch(mMode)
	{
	case e12TimeSecs:
		if (hours == 0)
			hours = 12;
		else if (hours > 12)
			hours -= 12;
		::snprintf(buffer.c_str_mod(), 32, am ? "%02ld:%02ld:%02ld AM" : "%02ld:%02ld:%02ld PM", hours, mins, secs);
		break;
	case e12TimeNoSecs:
		if (hours == 0)
			hours = 12;
		else if (hours > 12)
			hours -= 12;
		::snprintf(buffer.c_str_mod(), 32, am ? "%02ld:%02ld AM" : "%02ld:%02ld PM", hours % 12, mins);
		break;
	case e24TimeSecs:
		::snprintf(buffer.c_str_mod(), 32, "%02ld:%02ld:%02ld", hours, mins, secs);
		break;
	case e24TimeNoSecs:
		::snprintf(buffer.c_str_mod(), 32, "%02ld:%02ld", hours, mins);
		break;
	default:;
	}
	if (!buffer.empty())
		SetText(buffer);
}

void CDateTimeControl::GetTime(unsigned long& hours, unsigned long& mins, unsigned long& secs) const
{
	cdstring buffer = GetText();
	

	switch(mMode)
	{
	case e12TimeSecs:
	{
		cdstring t1(buffer, 0, 2);
		cdstring t2(buffer, 3, 2);
		cdstring t3(buffer, 6, 2);
		cdstring t4(buffer, 9);
		
		unsigned long i1 = ::atoi(t1);
		unsigned long i2 = ::atoi(t2);
		unsigned long i3 = ::atoi(t3);
		bool am = (t4.compare("AM") == 0);

		if (i1 == 12)
			i1 = 0;
		hours = i1 + (am ? 0 : 12);
		mins = i2;
		secs = i3;
		break;
	}
	case e12TimeNoSecs:
	{
		cdstring t1(buffer, 0, 2);
		cdstring t2(buffer, 3, 2);
		cdstring t4(buffer, 6);
		
		unsigned long i1 = ::atoi(t1);
		unsigned long i2 = ::atoi(t2);
		bool am = (t4.compare("AM") == 0);

		if (i1 == 12)
			i1 = 0;
		hours = i1 + (am ? 0 : 12);
		if (hours == 24)
			hours = 0;
		mins = i2;
		secs = 0;
		break;
	}
	case e24TimeSecs:
	{
		cdstring t1(buffer, 0, 2);
		cdstring t2(buffer, 3, 2);
		cdstring t3(buffer, 6, 2);
		
		unsigned long i1 = ::atoi(t1);
		unsigned long i2 = ::atoi(t2);
		unsigned long i3 = ::atoi(t3);

		hours = i1;
		mins = i2;
		secs = i3;
		break;
	}
	case e24TimeNoSecs:
	{
		cdstring t1(buffer, 0, 2);
		cdstring t2(buffer, 3, 2);
		
		unsigned long i1 = ::atoi(t1);
		unsigned long i2 = ::atoi(t2);

		hours = i1;
		mins = i2;
		secs = 0;
		break;
	}
	default:;
	}
}

bool CDateTimeControl::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	return CTextInputField::ObeyCommand(cmd, menu);
}

void CDateTimeControl::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	CTextInputField::UpdateCommand(cmd, cmdui);
}

bool CDateTimeControl::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	const JBoolean controlOn = modifiers.control();
	const JBoolean metaOn    = modifiers.meta();
	const JBoolean shiftOn   = modifiers.shift();
	
	// Some mods must be off
	if (controlOn || metaOn)
		return true;
		
	switch(key)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		HandleNumber(key - '0');
		break;
	case 'A':
	case 'P':
	case 'a':
	case 'p':
		HandleAMPM(key);
		break;
	case kJTabKey:
		ShiftSelection(!shiftOn);
		return true;
	case kJUpArrow:
		Nudge(true);
		return true;
	case kJDownArrow:
		Nudge(false);
		return true;
	case kJLeftArrow:
		ShiftSelection(false);
		return true;
	case kJRightArrow:
		ShiftSelection(true);
		return true;
	default:
		return true;
	}
	
	return true;
}

void CDateTimeControl::HandleNumber(const int key)
{
	// Make sure current field is selected
	JIndex sel_start;
	JIndex sel_end;
	EFieldType field = GetCurrentField(sel_start, sel_end);

	switch(mMode)
	{
	case eDateDDMMYYYY:
	case eDateMMDDYYYY:
	{
		unsigned long year;
		unsigned long month;
		unsigned long mday;
		GetDate(year, month, mday);
		
		if (field == eYear)
		{
			year = year * 10 + key;
			if (year > 9999)
				year = 0;
		}
		else if (field == eMonth)
		{
			month = month * 10 + key;
			if (month > 12)
				month = 0;
		}
		else if (field == eDay)
		{
			mday = mday * 10 + key;
			mday = NormaliseDay(month, mday);
		}
		
		SetDate(year, month, mday);
		break;
	}
	case e12TimeSecs:
	case e12TimeNoSecs:
	case e24TimeSecs:
	case e24TimeNoSecs:
	{
		unsigned long hour;
		unsigned long min;
		unsigned long sec;
		GetTime(hour, min, sec);
		bool am = (mMode == e12TimeSecs) || (mMode == e12TimeNoSecs);
		bool is_am = hour < 12;
		
		if (field == eHour)
		{
			if (am)
			{
				if (!is_am)
					hour -= 12;
				hour = hour * 10 + key;
				if (hour > 12)
					hour = 0;
				if (!is_am)
					hour += 12;
			}
			else
			{
				hour = hour * 10 + key;
				if (hour > 23)
					hour = 0;
			}
		}
		else if (field == eMinute)
		{
			min = min * 10 + key;
			if (min > 59)
				min = 0;
		}
		else if (field == eSecond)
		{
			sec = sec * 10 + key;
			if (sec > 59)
				sec = 0;
		}
		
		SetTime(hour, min, sec);
		break;
	}
	default:;
	}	

	// Set selection to changed item
	SetSel(sel_start, sel_end);
}

void CDateTimeControl::HandleAMPM(const int key)
{
	// Make sure current field is selected
	JIndex sel_start;
	JIndex sel_end;
	EFieldType field = GetCurrentField(sel_start, sel_end);
	if (field != eAMPM)
		return;

	switch(mMode)
	{
	case e12TimeSecs:
	case e12TimeNoSecs:
	{
		unsigned long hour;
		unsigned long min;
		unsigned long sec;
		GetTime(hour, min, sec);
		bool am = (mMode == e12TimeSecs) || (mMode == e12TimeNoSecs);
		
		if (field == eAMPM)
		{
			if (hour < 12)
			{
				if ((key == 'P') || (key == 'p'))
					hour += 12;
			}
			else
			{
				if ((key == 'A') || (key == 'a'))
					hour -= 12;
			}
		}
		
		SetTime(hour, min, sec);
		break;
	}
	default:;
	}	

	// Set selection to changed item
	SetSel(sel_start, sel_end);
}

void CDateTimeControl::Show()
{
	CTextInputField::Show();
	mSpinUp->Show();
	mSpinDown->Show();
}

void CDateTimeControl::Hide()
{
	CTextInputField::Hide();
	mSpinUp->Hide();
	mSpinDown->Hide();
}

void CDateTimeControl::Activate()
{
	CTextInputField::Activate();
	mSpinUp->SetActive(true);
	mSpinDown->SetActive(true);
}

void CDateTimeControl::Deactivate()
{
	CTextInputField::Deactivate();
	mSpinUp->SetActive(false);
	mSpinDown->SetActive(false);
}

void CDateTimeControl::ShiftSelection(bool forward)
{
	// Get the current field
	JIndex sel_start;
	JIndex sel_end;
	EFieldType field = GetCurrentField(sel_start, sel_end);

	bool shift_focus = false;

	switch(field)
	{
	case eDay:
		if (forward)
			field = (mMode == eDateDDMMYYYY) ? eMonth : eYear;
		else if (mMode == eDateDDMMYYYY)
			shift_focus = true;
		else
			field = eMonth;
		break;
	case eMonth:
		if (forward)
			field = (mMode == eDateDDMMYYYY) ? eYear : eDay;
		else if (mMode == eDateDDMMYYYY)
			field = eDay;
		else
			shift_focus = true;
		break;
	case eYear:
		if (forward)
			shift_focus = true;
		else
			field = (mMode == eDateDDMMYYYY) ? eMonth : eDay;
		break;
	case eHour:
		if (forward)
			field = eMinute;
		else
			shift_focus = true;
		break;
	case eMinute:
		if (forward)
		{
			if ((mMode == e12TimeSecs) || (mMode == e24TimeSecs))
				field = eSecond;
			else if (mMode == e12TimeNoSecs)
				field = eAMPM;
			else
				shift_focus = true;
		}
		else
			field = eHour;
		break;
	case eSecond:
		if (forward)
		{
			if (mMode == e12TimeSecs)
				field = eAMPM;
			else
				shift_focus = true;
		}
		else
			field = eMinute;
		break;
	case eAMPM:
		if (forward)
			shift_focus = true;
		else
		{
			if (mMode == e12TimeSecs)
				field = eSecond;
			else
				field = eMinute;
		}
		break;
	}

	if (shift_focus)
	{
		// Set focus to previous/next widget
		//GetWindow()->SwitchFocus(!forward);
	}
	else
	{
		SelectField(field);
	}
}

void CDateTimeControl::Nudge(bool up)
{
	// Get the current field
	JIndex sel_start;
	JIndex sel_end;
	EFieldType field = GetCurrentField(sel_start, sel_end);

	switch(mMode)
	{
	case eDateDDMMYYYY:
	case eDateMMDDYYYY:
	{
		unsigned long year;
		unsigned long month;
		unsigned long mday;
		GetDate(year, month, mday);
		
		if (field == eYear)
			up ? year++ : year--;
		else if (field == eMonth)
		{
			up ? month++ : month--;
			if (month > 12)
				month = 1;
			else if (month == 0)
				month = 12;

			mday = NormaliseDay(month, mday);
		}
		else if (field == eDay)
		{
			up ? mday++ : mday--;
			mday = NormaliseDay(month, mday);
		}
		
		SetDate(year, month, mday);
		break;
	}
	case e12TimeSecs:
	case e12TimeNoSecs:
	case e24TimeSecs:
	case e24TimeNoSecs:
	{
		unsigned long hour;
		unsigned long min;
		unsigned long sec;
		GetTime(hour, min, sec);
		bool am = (mMode == e12TimeSecs) || (mMode == e12TimeNoSecs);
		
		if (field == eHour)
		{
			hour++;
			up ? hour++ : hour--;
			if (hour > (am ? 12 : 24))
				hour = 1;
			else if (hour == 0)
				hour = (am ? 12 : 24);
			hour--;
		}
		else if (field == eMinute)
		{
			min++;
			up ? min++ : min--;
			if (min > 60)
				min = 1;
			else if (min == 0)
				min = 60;
			min--;
		}
		else if (field == eSecond)
		{
			sec++;
			up ? sec++ : sec--;
			if (sec > 60)
				sec = 1;
			else if (sec == 0)
				sec = 60;
			sec--;
		}
		else if (field == eAMPM)
		{
			if (hour < 12)
				hour += 12;
			else
				hour -= 12;
		}
		
		SetTime(hour, min, sec);
		break;
	}
	default:;
	}	

	// Set selection to changed item
	SetSel(sel_start, sel_end);
}

unsigned long CDateTimeControl::NormaliseDay(unsigned long month, unsigned long mday) const
{
	const unsigned long cMdays[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if ((month < 12) && (mday <= cMdays[month - 1]))
	{
		if (mday == 0)
			return cMdays[month - 1];
		else
			return mday;
	}
	else
		return 1;
}

CDateTimeControl::EFieldType CDateTimeControl::GetCurrentField(JIndex& sel_start, JIndex& sel_end) const
{
	GetSel(sel_start, sel_end);

	switch(mMode)
	{
	case eDateDDMMYYYY:
	case eDateMMDDYYYY:
		// First field
		if (sel_start < 3)
		{
			sel_start = 0;
			sel_end = 2;

			return (mMode == eDateDDMMYYYY) ? eDay : eMonth;
		}
		// Second field
		else if (sel_start < 6)
		{
			sel_start = 3;
			sel_end = 5;

			return (mMode == eDateDDMMYYYY) ? eMonth : eDay;
		}
		// Third field
		else
		{
			sel_start = 6;
			sel_end = GetTextLength();

			return eYear;
		}
		break;
	case e12TimeSecs:
		// First field
		if (sel_start < 3)
		{
			sel_start = 0;
			sel_end = 2;

			return eHour;
		}
		// Second field
		else if (sel_start < 6)
		{
			sel_start = 3;
			sel_end = 5;

			return eMinute;
		}
		// Third field
		else if (sel_start < 9)
		{
			sel_start = 6;
			sel_end = 8;

			return eSecond;
		}
		// Fourth field
		else
		{
			sel_start = 9;
			sel_end = 11;

			return eAMPM;
		}
		break;
	case e12TimeNoSecs:
		// First field
		if (sel_start < 3)
		{
			sel_start = 0;
			sel_end = 2;

			return eHour;
		}
		// Second field
		else if (sel_start < 6)
		{
			sel_start = 3;
			sel_end = 5;

			return eMinute;
		}
		// Third field
		else
		{
			sel_start = 6;
			sel_end = 8;

			return eAMPM;
		}
		break;
	case e24TimeSecs:
		// First field
		if (sel_start < 3)
		{
			sel_start = 0;
			sel_end = 2;

			return eHour;
		}
		// Second field
		else if (sel_start < 6)
		{
			sel_start = 3;
			sel_end = 5;

			return eMinute;
		}
		// Third field
		else
		{
			sel_start = 6;
			sel_end = 8;

			return eSecond;
		}
		break;
	case e24TimeNoSecs:
		// First field
		if (sel_start < 3)
		{
			sel_start = 0;
			sel_end = 2;

			return eHour;
		}
		// Second field
		else
		{
			sel_start = 3;
			sel_end = 5;

			return eMinute;
		}
		break;
	default:;
	}
	
	return eDay;
}

void CDateTimeControl::SelectField(EFieldType type)
{
	JIndex sel_start;
	JIndex sel_end;

	switch(mMode)
	{
	case eDateDDMMYYYY:
	case eDateMMDDYYYY:
		// First field
		if (type == eDay)
		{
			sel_start = (mMode == eDateDDMMYYYY) ? 0 : 3;
			sel_end = (mMode == eDateDDMMYYYY) ? 2 : 5;
		}
		// Second field
		else if (type == eMonth)
		{
			sel_start = (mMode == eDateDDMMYYYY) ? 3 : 0;
			sel_end = (mMode == eDateDDMMYYYY) ? 5 : 2;
		}
		// Third field
		else if (type == eYear)
		{
			sel_start = 6;
			sel_end = GetTextLength();
		}
		break;
	case e12TimeSecs:
	case e12TimeNoSecs:
	case e24TimeSecs:
	case e24TimeNoSecs:
		// First field
		if (type == eHour)
		{
			sel_start = 0;
			sel_end = 2;
		}
		// Second field
		else if (type == eMinute)
		{
			sel_start = 3;
			sel_end = 5;
		}
		// Third field
		else if (type == eSecond)
		{
			sel_start = 6;
			sel_end = 8;
		}
		// Third/Fourth field
		else if (type == eAMPM)
		{
			sel_start = (mMode == e12TimeSecs) ? 9 : 6;
			sel_end = (mMode == e12TimeSecs) ? 11 : 8;
		}
		break;
	default:;
	}
	
	SetSel(sel_start, sel_end);
}
