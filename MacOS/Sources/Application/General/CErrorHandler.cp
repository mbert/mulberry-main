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
#include "CMailNotification.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNotificationAlert.h"
#include "CXStringResources.h"

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C E R R O R H A N D L E R
// __________________________________________________________________________________________________

// Put up a stop alert
short CErrorHandler::PutStopAlert(const char* txt, bool beep)
{
	// Put up the alert
	if (!CMulberryApp::sSuspended)
		 return CErrorDialog::StopAlert(txt, beep);
	else
	{
		LStr255	err_msg = txt;
		CNotificationAlert::SNotificationAlert notify(err_msg, true, false, true, beep, cdstring::null_str);;
		new CNotificationStopAlert(notify);
		return -1;
	}
}

// Put up a stop alert
short CErrorHandler::PutStopAlert(short strId, short index, bool beep)
{
	cdstring temp;
	temp.FromResource(strId, index);
	return PutStopAlert(temp, beep);
}

// Put up a stop alert
short CErrorHandler::PutStopAlertRsrc(const char* rsrc_id, bool beep)
{
	return PutStopAlert(rsrc::GetString(rsrc_id), beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertTxt(const char* alert, const char* txt, bool beep)
{
	// Set up the string
	cdstring temp(alert);
	temp += txt;

	// Do default action
	return PutStopAlert(temp, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertTxt(short strId, short index, const char* txt, bool beep)
{
	// Set up the string
	cdstring temp;
	temp.FromResource(strId, index);
	return PutStopAlertTxt(temp, txt, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertRsrcTxt(const char* rsrc_id, const char* txt, bool beep)
{
	return PutStopAlertTxt(rsrc::GetString(rsrc_id), txt, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(const char* alert, const char* txt, bool beep)
{
	// Set up the string
	cdstring err_msg(alert);
	err_msg.Substitute(txt);

	// Do default action
	return PutStopAlert(err_msg, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(short strId, short index, const char* txt, bool beep)
{
	cdstring temp;
	temp.FromResource(strId, index);
	return PutStopAlertStr(temp, txt, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertRsrcStr(const char* rsrcid, const char* txt, bool beep)
{
	return PutStopAlertStr(rsrc::GetString(rsrcid), txt, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(short strId, short index, const unsigned char* txt, bool beep)
{
	// Convert to C-string and do alert
	cdstring temp(txt);
	short answer = PutStopAlertStr(strId, index, temp, beep);

	return answer;
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlert(short strId, short index, OSErr err, bool beep)
{
	return PutOSErrAlertTxt(strId, index, err, NULL, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertRsrc(const char* rsrc_id, OSErr err, bool beep)
{
	return PutOSErrAlertRsrcTxt(rsrc_id, err, NULL, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertTxt(const char* alert, OSErr err, const char* txt, bool beep)
{
	LStr255	err_1, err_2;
	short	answer = 1;
	StringHandle errtxt;

	errtxt = (StringHandle) ::GetResource(cEstrType, err);

	if (errtxt)
	{
		err_1 = *errtxt;
		err_1 += "\p ";
	}
	else
	{
		cdstring temp = rsrc::GetString("Alerts::General::OSERR");
		temp.Substitute((long) err);

		// Convert err to string
		err_1 = temp;
	}

	// Set up the description string
	err_2 = alert;
	err_1 += err_2;

	if (txt != NULL)
		err_1 += txt;

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		cdstring temp = err_1;
		answer = CErrorDialog::StopAlert(temp.c_str(), beep);
	}
	else
	{
		CNotificationAlert::SNotificationAlert notify(err_1, true, false, true, beep, cdstring::null_str);;
		new CNotificationStopAlert(notify);
		answer = -1;
	}

	return answer;
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertTxt(short strId, short index, OSErr err, const char* txt, bool beep)
{
	cdstring alert;
	alert.FromResource(strId, index);
	return PutOSErrAlertTxt(alert, err, txt, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertRsrcTxt(const char* rsrc_id, OSErr err, const char* txt, bool beep)
{
	cdstring alert = rsrc::GetString(rsrc_id);
	return PutOSErrAlertTxt(alert, err, txt, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlert(bool yesno, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring strdontshow;
	if (dontshow_rsrc != NULL)
		strdontshow = rsrc::GetString(dontshow_rsrc);

	// Put up the alert
	return CErrorDialog::CautionAlert(yesno, txt, strdontshow.c_str(), dontshow, beep) == CErrorDialog::eBtn1 ? Ok : Cancel;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlert(bool yesno, short strId, short index, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get string
	cdstring str;
	str.FromResource(strId, index);
	return PutCautionAlert(yesno, str, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertRsrc(bool yesno, const char* rsrcid, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get string
	return PutCautionAlert(yesno, rsrc::GetString(rsrcid), dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(bool yesno, const char* alert, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Set up the string
	cdstring err_msg(alert);
	err_msg.Substitute(txt);

	cdstring strdontshow;
	if (dontshow_rsrc != NULL)
		strdontshow = rsrc::GetString(dontshow_rsrc);

	// Put up the alert
	return CErrorDialog::CautionAlert(yesno, err_msg.c_str(), strdontshow.c_str(), dontshow, beep) == CErrorDialog::eBtn1 ? Ok : Cancel;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(bool yesno, short strId, short index, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring temp;
	temp.FromResource(strId, index);
	return PutCautionAlertStr(yesno, temp, txt, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertRsrcStr(bool yesno, const char* details_rsrc, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	return PutCautionAlertStr(yesno, rsrc::GetString(details_rsrc), txt, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(bool yesno, const char* alert, const char* txt1, const char* txt2, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get the string
	size_t err_msg_reserve = ::strlen(alert) + ::strlen(txt1) + ::strlen(txt2) + 1;
	cdstring err_msg;
	err_msg.reserve(err_msg_reserve);
	::snprintf(err_msg.c_str_mod(), err_msg_reserve, alert, txt1, txt2);

	cdstring strdontshow;
	if (dontshow_rsrc != NULL)
		strdontshow = rsrc::GetString(dontshow_rsrc);

	// Put up the alert
	return CErrorDialog::CautionAlert(yesno, err_msg.c_str(), strdontshow.c_str(), dontshow, beep) == CErrorDialog::eBtn1 ? Ok : Cancel;
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertStr(bool yesno, short strId, short index, const char* txt1, const char* txt2, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring temp;
	temp.FromResource(strId, index);
	return PutCautionAlertStr(yesno, temp, txt1, txt2, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertRsrcStr(bool yesno, const char* details_rsrc, const char* txt1, const char* txt2, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	return PutCautionAlertStr(yesno, rsrc::GetString(details_rsrc), txt1, txt2, dontshow_rsrc, dontshow, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(const char* txt, bool beep)
{
	// Put up the alert
	if (!CMulberryApp::sSuspended)
		return CErrorDialog::NoteAlert(txt, NULL, NULL, beep);
	else
	{
		LStr255	err_msg;

		// Get the string
		err_msg = txt;
		CNotificationAlert::SNotificationAlert notify(err_msg, true, false, true, beep, cdstring::null_str);;
		new CNotificationNoteAlert(notify);
		return -1;
	}
}

short CErrorHandler::PutNoteAlert(short strId, short index, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId, index);
	return PutNoteAlert(temp, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertRsrc(const char* rsrcid, bool beep)
{
	return PutNoteAlert(rsrc::GetString(rsrcid), beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertTxt(const char* alert, const char* txt, bool beep)
{
	// Set up the string
	cdstring err_msg(alert);
	err_msg += txt;

	return PutNoteAlert(err_msg, beep);
}

short CErrorHandler::PutNoteAlertTxt(short strId, short index, const char* txt, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId, index);
	return PutNoteAlertTxt(temp, txt, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertRsrcTxt(const char* rsrcid, const char* txt, bool beep)
{
	return PutNoteAlertTxt(rsrc::GetString(rsrcid), txt, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertStr(const char* alert, const char* txt, bool beep)
{
	// Set up the string
	cdstring err_msg(alert);
	err_msg.Substitute(txt);

	return PutNoteAlert(err_msg, beep);
}

short CErrorHandler::PutNoteAlertStr(short strId, short index, const char* txt, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId, index);
	return PutNoteAlertStr(temp, txt, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertRsrcStr(const char* rsrcid, const char* txt, bool beep)
{
	return PutNoteAlertStr(rsrc::GetString(rsrcid), txt, beep);
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(const char* alert, const CMailNotification& mnotify, bool no_display)
{
	short	answer = 1;

	// Get the string
	LStr255	err_msg(alert);

	// Put up the alert
	if (!CMulberryApp::sSuspended)
	{
		if (mnotify.DoPlaySound())
			::PlayNamedSound(mnotify.GetSoundID());
		if (mnotify.DoShowAlertForeground())
		{
			if (!no_display && !CMulberryApp::sApp->IsPaused())
			{
				cdstring temp = err_msg;
				answer = CErrorDialog::NoteAlert(temp.c_str(), NULL, NULL, !mnotify.DoPlaySound());
			}
			else if (!mnotify.DoPlaySound())
				::SysBeep(1);
		}
	}
	else
	{
		if (!no_display && !CMulberryApp::sApp->IsPaused())
		{
			CNotificationAlert::SNotificationAlert notify(err_msg,
															mnotify.DoShowAlertForeground(),
															mnotify.DoShowAlertBackground(),
															mnotify.DoFlashIcon(),
															mnotify.DoPlaySound(),
															mnotify.GetSoundID());
			new CNotificationNoteAlert(notify);
			answer = -1;
		}
		else if (mnotify.DoPlaySound())
			::PlayNamedSound(mnotify.GetSoundID());
	}

	return answer;
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(short strId, short index, const CMailNotification& mnotify, bool no_display)
{
	cdstring temp;
	temp.FromResource(strId, index);
	return PutNoteAlert(temp, mnotify, no_display);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertRsrc(const char* rsrcid, const CMailNotification& mnotify, bool no_display)
{
	return PutNoteAlert(rsrc::GetString(rsrcid), mnotify, no_display);
}
