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


// CSearchListPanel.h

#ifndef __CSEARCHLISTPANEL__MULBERRY__
#define __CSEARCHLISTPANEL__MULBERRY__

#include "CMailboxListPanel.h"

// Classes
class CSearchListPanel : public CMailboxListPanel
{
	friend class CSearchWindow;

public:
				CSearchListPanel();
	virtual		~CSearchListPanel();

	virtual void CreateSelf(CWnd* parent_frame, CWnd* move_parent1, CWnd* move_parent2, int width, int height);			// Manually create document

	virtual void SetInProgress(bool in_progress);

protected:
	CButton		mOptionsBtn;

	// message handlers
	afx_msg void OnOptions();

	virtual void AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	afx_msg void OnClearMailboxList();
	afx_msg void OnOpenMailboxList();

	DECLARE_MESSAGE_MAP()
};

#endif
