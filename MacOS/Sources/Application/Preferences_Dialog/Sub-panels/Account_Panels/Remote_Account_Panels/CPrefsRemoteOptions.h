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


// Header for CPrefsRemoteOptions class

#ifndef __CPREFSREMOTEOPTIONS__MULBERRY__
#define __CPREFSREMOTEOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsRemoteOptions = 5022;
const	PaneIDT		paneid_PrefsRemoteOptionsUse = 'USES';
const	PaneIDT		paneid_PrefsRemoteOptionsBaseRURLText = 'RTXT';
const	PaneIDT		paneid_PrefsRemoteOptionsBaseRURL = 'RURL';

// Mesages

// Resources

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsRemoteOptions : public CPrefsTabSubPanel
{
private:
	LCheckBox*		mUseRemote;
	CTextFieldX*	mBaseRURL;

public:
	enum { class_ID = 'Arop' };

					CPrefsRemoteOptions();
					CPrefsRemoteOptions(LStream *inStream);
	virtual 		~CPrefsRemoteOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
