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


// Header for CSearchListPanel class

#ifndef __CSEARCHLISTPANEL__MULBERRY__
#define __CSEARCHLISTPANEL__MULBERRY__

#include "CMailboxListPanel.h"

// Constants
const	PaneIDT		paneid_SearchOptions = 'OPTS';

// Messages
const	MessageT	msg_SearchOptions = 'OPTS';

// Classes
class LPushButton;

class CSearchListPanel : public CMailboxListPanel
{
	friend class CSearchWindow;

public:
					CSearchListPanel();
	virtual 		~CSearchListPanel();

	virtual void SetInProgress(bool in_progress);

protected:
	LPushButton*	mOptionsBtn;

	virtual void	FinishCreateSelf(LWindow* parent, LPane* container);

	virtual void	AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	virtual void	OnClearMailboxList();
	virtual void	OnOpenMailboxList();
			void	OnOptions();
};

#endif
