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


// Source for CErrorHandler class

#include "CErrorHandler.h"

#include "CErrorDialog.h"
#include "CMulberryApp.h"
#include "CNotificationAlert.h"

#include <UDesktop.h>

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C E R R O R H A N D L E R
// __________________________________________________________________________________________________

// Put up a stop alert
short CErrorHandler::PutStopAlert(short strId, short index, Boolean beep)
{
	Str255	err_msg;
	short	answer = 1;

	// Set up the string
	::GetIndString(err_msg,strId,index);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		::p2cstr(err_msg);
		answer = CErrorDialog::StopAlert((char*) err_msg, beep);
	}
	else
	{
		CNotificationStopAlert* notify = new CNotificationStopAlert(err_msg, beep);
		answer = -1;
	}
	
	return answer;
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertTxt(short strId, short index, const char* txt, Boolean beep)
{
	Str255	err_msg;
	short	answer = 1;

	// Set up the string
	::GetIndString(err_msg,strId,index);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		::p2cstr(err_msg);
		::strcat((char*) err_msg, txt);
		answer = CErrorDialog::StopAlert((char*) err_msg, beep);
	}
	else
	{
		::c2pstr((char*) txt);
		CNotificationStopAlert* notify = new CNotificationStopAlert(err_msg, (unsigned char*) txt, beep);
		::p2cstr((unsigned char*) txt);
		answer = -1;
	}

	return answer;
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(short strId, short index, const char* txt, Boolean beep)
{
	Str255	err_str;
	Str255	err_msg;
	short	answer = 1;

	// Set up the string
	::GetIndString(err_str,strId,index);
	::p2cstr(err_str);
	::sprintf((char*) err_msg, (char*) err_str, txt);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		answer = CErrorDialog::StopAlert((char*) err_msg, beep);
	}
	else
	{
		::c2pstr((char*) err_msg);
		CNotificationStopAlert* notify = new CNotificationStopAlert(err_msg, beep);
		answer = -1;
	}

	return answer;
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(short strId, short index, const unsigned char* txt, Boolean beep)
{
	// Convert to C-string and do alert
	::p2cstr((unsigned char*) txt);
	short answer = PutStopAlertStr(strId, index, (char*) txt, beep);
	::c2pstr((char*) txt);

	return answer;
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlert(short strId, short index, OSErr err, Boolean beep)
{
	Str255	err_1, err_2;
	short	answer = 1;
	StringHandle errtxt;
	
	errtxt = (StringHandle) ::GetResource(cEstrType, err);

	if (errtxt)
	{
		::PLstrcpy(err_1, *errtxt);
		::PLstrcat(err_1, "\p ");
	}
	else
	{
		errtxt = ::GetString(STR_Error);
		::PLstrcpy(err_1, *errtxt);

		// Convert err to string
		Str255 err_3;
		::sprintf((char*) err_3,"%d\r",err);
		::c2pstr((char*) err_3);
		::PLstrcat(err_1, err_3);
	}

	// Set up the description string
	::GetIndString(err_2, strId, index);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		::PLstrcat(err_1, err_2);
		::p2cstr(err_1);
		answer = CErrorDialog::StopAlert((char*) err_1, beep);
	}
	else
	{
		CNotificationStopAlert* notify = new CNotificationStopAlert(err_1, err_2, beep);
		answer = -1;
	}
	
	return answer;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlert(short strId, short index, Boolean beep)
{
	Str255	err_msg;
	short	answer = 1;

	// Get the string
	::GetIndString(err_msg,strId,index);

	// Put up the alert
	::p2cstr(err_msg);
	answer = CErrorDialog::CautionAlert((char*) err_msg, beep);
	
	return answer;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(short strId, short index, const char* txt, Boolean beep)
{
	Str255	err_str;
	Str255	err_msg;
	short	answer = 1;

	// Get the string
	::GetIndString(err_str,strId,index);
	::p2cstr(err_str);
	::sprintf((char*) err_msg, (char*) err_str, txt);

	// Put up the alert
	answer = CErrorDialog::CautionAlert((char*) err_msg, beep);
	
	return answer;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(short strId, short index, const char* txt1, const char* txt2, Boolean beep)
{
	Str255	err_str;
	Str255	err_msg;
	short	answer = 1;

	// Get the string
	::GetIndString(err_str,strId,index);
	::p2cstr(err_str);
	::sprintf((char*) err_msg, (char*) err_str, txt1, txt2);

	// Put up the alert
	answer = CErrorDialog::CautionAlert((char*) err_msg, beep);
	
	return answer;
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(short strId, short index, Boolean doResumeAlert, Boolean beep)
{
	Str255	err_msg;
	short	answer = 1;

	// Get the string
	::GetIndString(err_msg, strId, index);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		if (doResumeAlert)
		{
			::p2cstr(err_msg);
			answer = CErrorDialog::NoteAlert((char*) err_msg, beep);
		}
		else if (beep)
			::SysBeep(1);
	}
	else
	{
		CNotificationNoteAlert* notify = new CNotificationNoteAlert(err_msg, doResumeAlert, beep);
		answer = -1;
	}
	
	return answer;
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(const char* txt, Boolean doResumeAlert, Boolean beep)
{
	short	answer = 1;

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		if (doResumeAlert)
			answer = CErrorDialog::NoteAlert(txt, beep);
		else if (beep)
			::SysBeep(1);
	}
	else
	{
		Str255	err_msg;

		// Get the string
		::strcpy((char*) err_msg, txt);
		::c2pstr((char*) err_msg);
		CNotificationNoteAlert* notify = new CNotificationNoteAlert(err_msg, doResumeAlert, beep);
		answer = -1;
	}
	
	return answer;
}
