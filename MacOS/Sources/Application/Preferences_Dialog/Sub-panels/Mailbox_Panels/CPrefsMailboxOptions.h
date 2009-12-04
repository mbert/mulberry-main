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


// Header for CPrefsMailboxOptions class

#ifndef __CPREFSMAILBOXOPTIONS__MULBERRY__
#define __CPREFSMAILBOXOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsMailboxOptions = 5063;
const	PaneIDT		paneid_MDOPromptStartup = 'PROM';
const	PaneIDT		paneid_MDOAutoDial = 'DIAL';
const	PaneIDT		paneid_MDOUIDValidityMismatch = 'UIDV';
const	PaneIDT		paneid_MDOOSDefaultLocation = 'DFOS';
const	PaneIDT		paneid_MDOOSSleepReconnect = 'QUIK';

// Mesages

// Resources

// Classes
class LCheckBox;

class	CPrefsMailboxOptions : public CPrefsTabSubPanel
{
private:
	LCheckBox*		mPromptStartup;
	LCheckBox*		mAutoDial;
	LCheckBox*		mUIDValidityMismatch;
	LCheckBox*		mOSDefaultLocation;
	LCheckBox*		mSleepReconnect;

public:
	enum { class_ID = 'Auid' };

					CPrefsMailboxOptions();
					CPrefsMailboxOptions(LStream *inStream);
	virtual 		~CPrefsMailboxOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
