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


// Header for CErrorHandler class

#ifndef __CERRORHANDLER__MULBERRY__
#define __CERRORHANDLER__MULBERRY__

#include "CMulberryCommon.h"

// ALRT's
const	OSType	cEstrType = 'Estr';		// Resource type for OSErr descriptions

// Classes

class CMailNotification;

class CErrorHandler {

public:

	enum
	{
		Ok = kOK_Btn,
		Cancel = kCancel_Btn
	};

	static short	PutStopAlert(const char* txt,
								bool beep = true);			// Put up a stop alert
	static short	PutStopAlert(short strId,
								short index,
								bool beep = true);			// Put up a stop alert
	static short	PutStopAlertRsrc(const char* rsrc_id,
								bool beep = true);			// Put up a stop alert

	static short	PutStopAlertTxt(const char* alert,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text
	static short	PutStopAlertTxt(short strId,
								short index,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text
	static short	PutStopAlertRsrcTxt(const char* rsrc_id,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text

	static short	PutStopAlertStr(const char* alert,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text
	static short	PutStopAlertStr(short strId,
								short index,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text
	static short	PutStopAlertRsrcStr(const char* rsrcid,
								const char* txt,
								bool beep = true);			// Put up a stop alert with additional text

	static short	PutStopAlertStr(short strId,
								short index,
								const unsigned char* txt,
								bool beep = true);			// Put up a stop alert with additional text

	static short	PutOSErrAlert(short strId,
								short index,
								OSErr err,
								bool beep = true);			// Put up a stop alert with OSErr
	static short	PutOSErrAlertRsrc(const char* rsrc_id,
								OSErr err,
								bool beep = true);			// Put up a stop alert with OSErr

	static short	PutOSErrAlertTxt(const char* alert,
								OSErr err,
								const char* txt,
								bool beep = true);			// Put up a stop alert with OSErr
	static short	PutOSErrAlertTxt(short strId,
								short index,
								OSErr err,
								const char* txt,
								bool beep = true);			// Put up a stop alert with OSErr with additional text
	static short	PutOSErrAlertRsrcTxt(const char* rsrc_id,
								OSErr err,
								const char* txt,
								bool beep = true);			// Put up a stop alert with OSErr with additional text

	static short	PutCautionAlert(bool yesno,
								const char* txt,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert
	static short	PutCautionAlert(bool yesno,
								short strId,
								short index,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert
	static short	PutCautionAlertRsrc(bool yesno,
								const char* rsrcid,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert

	static short	PutCautionAlertStr(bool yesno,
								const char* alert,
								const char* txt,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text
	static short	PutCautionAlertStr(bool yesno,
								short strId,
								short index,
								const char* txt,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text
	static short	PutCautionAlertRsrcStr(bool yesno,
								const char* details_rsrc,
								const char* txt,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text

	static short	PutCautionAlertStr(bool yesno,
								const char* alert,
								const char* txt1,
								const char* txt2,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text
	static short	PutCautionAlertStr(bool yesno,
								short strId,
								short index,
								const char* txt1,
								const char* txt2,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text
	static short	PutCautionAlertRsrcStr(bool yesno,
								const char* details_rsrc,
								const char* txt1,
								const char* txt2,
								const char* dontshow_rsrc = NULL,
								bool* dontshow = NULL,
								bool beep = true);			// Put up a caution alert with addition text

	static short	PutNoteAlert(const char* txt,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlert(short strId,
								short index,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlertRsrc(const char* rsrcid,
								bool beep = true);			// Put up a note alert

	static short	PutNoteAlertTxt(const char* alert,
								const char* txt,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlertTxt(short strId,
								short index,
								const char* txt,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlertRsrcTxt(const char* rsrcid,
								const char* txt,
								bool beep = true);			// Put up a note alert

	static short	PutNoteAlertStr(const char* alert,
								const char* txt,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlertStr(short strId,
								short index,
								const char* txt,
								bool beep = true);			// Put up a note alert
	static short	PutNoteAlertRsrcStr(const char* rsrcid,
								const char* txt,
								bool beep = true);			// Put up a note alert

	static short	PutNoteAlert(const char* alert,
								const CMailNotification& mnotify,
								bool no_display = false);	// Put up a note alert
	static short	PutNoteAlert(short strId,
								short index,
								const CMailNotification& mnotify,
								bool no_display = false);	// Put up a note alert
	static short	PutNoteAlertRsrc(const char* rsrcid,
								const CMailNotification& mnotify,
								bool no_display = false);	// Put up a note alert

private:
	CErrorHandler() {};		// Never created...static methods only
	~CErrorHandler() {};
	
};

#endif
