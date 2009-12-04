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


// Header for CPrefsSMTPOptions class

#ifndef __CPREFSSMTPOPTIONS__MULBERRY__
#define __CPREFSSMTPOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsSMTPOptions = 5037;
const	PaneIDT		paneid_PrefsSMTPOptionsUseQueue = 'USEL';
const	PaneIDT		paneid_PrefsSMTPOptionsHoldMessages = 'HOLD';

// Mesages
const	MessageT	msg_PrefsSMTPOptionsUseQueue = 'USEL';

// Resources
const	ResIDT		RidL_CPrefsSMTPOptions = 5037;

// Classes
class LCheckBox;

class	CPrefsSMTPOptions : public CPrefsTabSubPanel, public LListener
{
private:
	LCheckBox*		mUseQueue;
	LCheckBox*		mHoldMessages;

public:
	enum { class_ID = 'Asmt' };

					CPrefsSMTPOptions();
					CPrefsSMTPOptions(LStream *inStream);
	virtual 		~CPrefsSMTPOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
