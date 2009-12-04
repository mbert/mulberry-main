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


// Header for CPrefsMailboxDisconnected class

#ifndef __CPREFSMAILBOXDISCONNECTED__MULBERRY__
#define __CPREFSMAILBOXDISCONNECTED__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsMailboxDisconnected = 5062;
const	PaneIDT		paneid_MDTabs = 'TABS';

// Mesages
const	MessageT	msg_MDTabs = 'TABS';

// Resources
const	ResIDT		RidL_CPrefsMailboxDisconnectedBtns = 5062;

// Classes
class CPrefsSubTab;

class CPrefsMailboxDisconnected : public CPrefsTabSubPanel, public LListener
{
private:
	CPrefsSubTab*		mTabs;

public:
	enum { class_ID = 'Mdis' };

					CPrefsMailboxDisconnected();
					CPrefsMailboxDisconnected(LStream *inStream);
	virtual 		~CPrefsMailboxDisconnected();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
