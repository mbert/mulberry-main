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


// Header for CPrefsAccountRemoteCalendar class

#ifndef __CPREFSACCOUNTREMOTECALENDAR__MULBERRY__
#define __CPREFSACCOUNTREMOTECALENDAR__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountRemoteCalendar = 5047;
const	PaneIDT		paneid_AccountRemoteCalendarLoginAtStart = 'LOGO';
const	PaneIDT		paneid_AccountRemoteCalendarDisconnected = 'DISC';
const	PaneIDT		paneid_AccountRemoteCalendarBaseRURL = 'RURL';

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsAccountRemoteCalendar : public CPrefsTabSubPanel
{
private:
	LCheckBox*			mLogonAtStartup;
	LCheckBox*			mDisconnected;
	CTextFieldX*		mBaseRURL;

public:
	enum { class_ID = 'Acal' };

					CPrefsAccountRemoteCalendar();
					CPrefsAccountRemoteCalendar(LStream *inStream);
	virtual 		~CPrefsAccountRemoteCalendar();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
