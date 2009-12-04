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

#ifndef __CERROREVENTDISPATCHER__MULBERRY__
#define __CERROREVENTDISPATCHER__MULBERRY__

#include <PP_Types.h>

// ALRT's
const	OSType	cEstrType = 'Estr';		// Resource type for OSErr descriptions

const	ResIDT	STR_Error = 10000;

const	ResIDT	STRx_Demo = 11000;
enum
{
	str_DemoWarn = 1,
	str_SendDemoWarn
};

// Classes

class CErrorHandler {

public:

	static short	PutStopAlert(short strId,
								short index,
								Boolean beep = true);			// Put up a stop alert

	static short	PutStopAlertTxt(short strId,
								short index,
								const char* txt,
								Boolean beep = true);			// Put up a stop alert with additional text

	static short	PutStopAlertStr(short strId,
								short index,
								const char* txt,
								Boolean beep = true);			// Put up a stop alert with additional text

	static short	PutStopAlertStr(short strId,
								short index,
								const unsigned char* txt,
								Boolean beep = true);			// Put up a stop alert with additional text

	static short	PutOSErrAlert(short strId,
								short index,
								OSErr err,
								Boolean beep = true);			// Put up a stop alert with OSErr

	static short	PutCautionAlert(short strId,
								short index,
								Boolean beep = true);			// Put up a caution alert

	static short	PutCautionAlertStr(short strId,
								short index,
								const char* txt,
								Boolean beep = true);			// Put up a caution alert with addition text
	static short	PutCautionAlertStr(short strId,
								short index,
								const char* txt1,
								const char* txt2,
								Boolean beep = true);			// Put up a caution alert with addition text

	static short	PutNoteAlert(short strId,
								short index,
								Boolean doResumeAlert = true,
								Boolean beep = true);			// Put up a note alert
	static short	PutNoteAlert(const char* txt,
								Boolean doResumeAlert = true,
								Boolean beep = true);			// Put up a note alert

private:
	CErrorHandler() {};		// Never created...static methods only
	~CErrorHandler() {};
	
};

#endif
