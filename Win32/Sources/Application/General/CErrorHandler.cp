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
#include "CSDIFrame.h"
#include "CXStringResources.h"

#include <stdio.h>
#include <string.h>

// OS Error index table
static const int cOSErrTable[] =
{
	ERROR_FILE_NOT_FOUND,		IDE_ERROR_FILE_NOT_FOUND,
	ERROR_PATH_NOT_FOUND,		IDE_ERROR_PATH_NOT_FOUND,
	ERROR_TOO_MANY_OPEN_FILES,	IDE_ERROR_TOO_MANY_OPEN_FILES,
	ERROR_ACCESS_DENIED,		IDE_ERROR_ACCESS_DENIED,
	ERROR_INVALID_HANDLE,		IDE_ERROR_INVALID_HANDLE,
	ERROR_NOT_ENOUGH_MEMORY,	IDE_ERROR_NOT_ENOUGH_MEMORY,
	ERROR_OUTOFMEMORY,			IDE_ERROR_OUTOFMEMORY,
	ERROR_INVALID_DRIVE,		IDE_ERROR_INVALID_DRIVE,
	ERROR_CURRENT_DIRECTORY,	IDE_ERROR_CURRENT_DIRECTORY,
	ERROR_WRITE_PROTECT,		IDE_ERROR_WRITE_PROTECT,
	ERROR_SHARING_VIOLATION,	IDE_ERROR_SHARING_VIOLATION,
	ERROR_LOCK_VIOLATION,		IDE_ERROR_LOCK_VIOLATION,
	ERROR_HANDLE_DISK_FULL,		IDE_ERROR_HANDLE_DISK_FULL,
	ERROR_DUP_NAME,				IDE_ERROR_DUP_NAME,
	ERROR_OPEN_FAILED,			IDE_ERROR_OPEN_FAILED,
	ERROR_BUFFER_OVERFLOW,		IDE_ERROR_BUFFER_OVERFLOW,
	ERROR_DISK_FULL,			IDE_ERROR_DISK_FULL,
	ERROR_INVALID_NAME,			IDE_ERROR_INVALID_NAME,
	ERROR_ALREADY_EXISTS,		IDE_ERROR_ALREADY_EXISTS,
	ERROR_DIRECTORY,			IDE_ERROR_DIRECTORY,
	10038, 						IDE_WSAENOTSOCK,
	10045, 						IDE_WSAEOPNOTSUPP,
	10050, 						IDE_WSAENETDOWN,
	10051, 						IDE_WSAENETUNREACH,
	10052, 						IDE_WSAENETRESET,
	10053, 						IDE_WSAECONNABORTED,
	10054, 						IDE_WSAECONNRESET,
	10055, 						IDE_WSAENOBUFS,
	10056, 						IDE_WSAEISCONN,
	10057, 						IDE_WSAENOTCONN,
	10058, 						IDE_WSAESHUTDOWN,
	10060, 						IDE_WSAETIMEDOUT,
	10061, 						IDE_WSAECONNREFUSED,
	10064, 						IDE_WSAEHOSTDOWN,
	10065, 						IDE_WSAEHOSTUNREACH,
	10091, 						IDE_WSASYSNOTREADY,
	10093, 						IDE_WSANOTINITIALISED,
	11001, 						IDE_WSAHOST_NOT_FOUND,
	11002, 						IDE_WSATRY_AGAIN,
	11003, 						IDE_WSANO_RECOVERY,
	11004, 						IDE_WSANO_DATA,
	0, 0
};
	
// __________________________________________________________________________________________________
// C L A S S __ C E R R O R H A N D L E R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CErrorHandler::CErrorHandler()
{
}

// Default destructor
CErrorHandler::~CErrorHandler()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Put up a stop alert
short CErrorHandler::PutStopAlert(const char* string, bool beep)
{
	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(cdstring(string).win_str(), MB_TASKMODAL | MB_ICONSTOP | MB_OK);
}

// Put up a stop alert
short CErrorHandler::PutStopAlert(short strId, bool beep)
{
	cdstring temp;
	temp.FromResource(strId);
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
short CErrorHandler::PutStopAlertTxt(short strId, const char* txt, bool beep)
{
	// Set up the string
	cdstring temp;
	temp.FromResource(strId);
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
	cdstring err_msg(alert);
	err_msg.Substitute(txt);
	return PutStopAlert(err_msg, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertStr(short strId, const char* txt, bool beep)
{
	cdstring temp;
	temp.FromResource(strId);
	return PutStopAlertStr(temp, txt, beep);
}

// Put up a stop alert with additional text
short CErrorHandler::PutStopAlertRsrcStr(const char* rsrcid, const char* txt, bool beep)
{
	return PutStopAlertStr(rsrc::GetString(rsrcid), txt, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlert(short strId, OSErr err, bool beep)
{
	return PutOSErrAlertTxt(strId, err, NULL, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertRsrc(const char* rsrc_id, OSErr err, bool beep)
{
	return PutOSErrAlertRsrcTxt(rsrc_id, err, NULL, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertTxt(const char* alert, OSErr err, const char* txt, bool beep)
{
	// Look for matching OSErr
	int match_id = 0;
	const int* p = cOSErrTable;
	while(*p)
	{
		if (*p == err)
		{
			match_id = *++p;
			break;
		}
		p += 2;
	}

	CString s;
	CString buf;
	
	if (match_id)
	{
		s.LoadString(match_id);
		s += " ";
		buf = s;
	}
	else
	{
		s = rsrc::GetString("Alerts::General::OSERR");
		buf.Format(s, err);
	}
	s = alert;
	s = buf + s;
	if (txt)
		s += cdustring(txt);
	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(s, MB_TASKMODAL | MB_ICONSTOP | MB_OK);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertTxt(short strId, OSErr err, const char* txt, bool beep)
{
	cdstring alert;
	alert.FromResource(strId);
	return PutOSErrAlertTxt(alert, err, txt, beep);
}

// Put up a stop alert with OSErr
short CErrorHandler::PutOSErrAlertRsrcTxt(const char* rsrc_id, OSErr err, const char* txt, bool beep)
{
	cdstring alert = rsrc::GetString(rsrc_id);
	return PutOSErrAlertTxt(alert, err, txt, beep);
}

// Put up a stop alert with File error
short CErrorHandler::PutFileErrAlert(const char* txt, CFileException& ex, bool beep)
{
	// Get file specific message
	TCHAR errmsg[256];
	ex.GetErrorMessage(errmsg, 256);

	// Get error action
	cdstring s(txt);
	s += errmsg;

	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(s.win_str(), MB_TASKMODAL | MB_ICONSTOP | MB_OK);
}

// Put up a stop alert with File error
short CErrorHandler::PutFileErrAlert(short strId, CFileException& ex, bool beep)
{
	// Get error action
	cdstring s;
	s.FromResource(strId);
	return PutFileErrAlert(s, ex, beep);
}

// Put up a stop alert with File error
short CErrorHandler::PutFileErrAlertRsrc(const char* rsrcid, CFileException& ex, bool beep)
{
	return PutFileErrAlert(rsrc::GetString(rsrcid), ex, beep);
}

// Put up a caution alert
short CErrorHandler::PutCautionAlert(bool yesno, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring strdontshow;
	if (dontshow_rsrc != NULL)
		strdontshow = rsrc::GetString(dontshow_rsrc);

	// Put up the alert
	return CErrorDialog::CautionAlert(yesno, txt, strdontshow.c_str(), dontshow, beep) == CErrorDialog::eBtn1 ? Ok : Cancel;
}

// Put up a caution alert
short CErrorHandler::PutCautionAlert(bool yesno, short strId, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get string
	cdstring str;
	str.FromResource(strId);
	return PutCautionAlert(yesno, str, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert
short CErrorHandler::PutCautionAlertRsrc(bool yesno, const char* rsrcid, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get string
	return PutCautionAlert(yesno, rsrc::GetString(rsrcid), dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert
short CErrorHandler::PutCautionAlertStr(bool yesno, const char* alert, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	// Get the string
	cdstring err_msg(alert);
	err_msg.Substitute(txt);

	cdstring strdontshow;
	if (dontshow_rsrc != NULL)
		strdontshow = rsrc::GetString(dontshow_rsrc);

	// Put up the alert
	return CErrorDialog::CautionAlert(yesno, err_msg.c_str(), strdontshow.c_str(), dontshow, beep) == CErrorDialog::eBtn1 ? Ok : Cancel;
}

// Put up a caution alert
short CErrorHandler::PutCautionAlertStr(bool yesno, short strId, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring temp;
	temp.FromResource(strId);
	return PutCautionAlertStr(yesno, temp, txt, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert - NB can only be called while not suspended
short CErrorHandler::PutCautionAlertRsrcStr(bool yesno, const char* details_rsrc, const char* txt, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	return PutCautionAlertStr(yesno, rsrc::GetString(details_rsrc), txt, dontshow_rsrc, dontshow, beep);
}

// Put up a caution alert
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

// Put up a caution alert
short CErrorHandler::PutCautionAlertStr(bool yesno, short strId, const char* txt1, const char* txt2, const char* dontshow_rsrc, bool* dontshow, bool beep)
{
	cdstring temp;
	temp.FromResource(strId);
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
	cdustring s(txt);
	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(s.c_str(), MB_TASKMODAL | MB_ICONINFORMATION | MB_OK);
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(short strId, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId);
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
	cdstring err_msg(alert);
	err_msg += txt;
	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(err_msg.win_str(), MB_TASKMODAL | MB_ICONINFORMATION | MB_OK);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertTxt(short strId, const char* txt, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId);
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
	cdstring err_msg(alert);
	err_msg.Substitute(txt);
	if (beep)
		::MessageBeep(-1);
	return ::ModalMessageBox(err_msg.win_str(), MB_TASKMODAL | MB_ICONINFORMATION | MB_OK);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertStr(short strId, const char* txt, bool beep)
{
	// Get the string
	cdstring temp;
	temp.FromResource(strId);
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
	if (mnotify.DoPlaySound())
		::PlayNamedSound(mnotify.GetSoundID());
		
	// Now check foreground/background
	DWORD wnd_proc_id;
	::GetWindowThreadProcessId(CWnd::GetForegroundWindow()->GetSafeHwnd(), &wnd_proc_id);
	if (wnd_proc_id == ::GetCurrentProcessId())
	{
		if (mnotify.DoShowAlertForeground())
		{
			if (!no_display && !CMulberryApp::sApp->IsPaused())
			{
				cdustring s(alert);
				::ModalMessageBox(s.c_str(), MB_TASKMODAL | MB_ICONINFORMATION | MB_OK);
			}
			else if (!mnotify.DoPlaySound())
				::MessageBeep(-1);
		}
	}
	else
	{
		if (mnotify.DoFlashIcon() && !no_display && !CMulberryApp::sApp->IsPaused())
			CMulberryApp::sApp->ShowNotification();

		if (mnotify.DoShowAlertBackground())
		{
			if (!no_display && !CMulberryApp::sApp->IsPaused())
			{
				cdustring s(alert);
				::ModalMessageBox(s.c_str(), MB_SYSTEMMODAL | MB_ICONINFORMATION | MB_OK);
			}
			else if (!mnotify.DoPlaySound())
				::MessageBeep(-1);
		}
	}

	return 1;
}

// Put up a note alert
short CErrorHandler::PutNoteAlert(short strId, const CMailNotification& mnotify, bool no_display)
{
	cdstring temp;
	temp.FromResource(strId);
	return PutNoteAlert(temp, mnotify, no_display);
}

// Put up a note alert
short CErrorHandler::PutNoteAlertRsrc(const char* rsrcid, const CMailNotification& mnotify, bool no_display)
{
	return PutNoteAlert(rsrc::GetString(rsrcid), mnotify, no_display);
}
