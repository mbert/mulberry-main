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

// Classes
class CSearchListPanel : public CMailboxListPanel
{
	friend class CSearchWindow;

public:
	CSearchListPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual	~CSearchListPanel();

protected:
	virtual void	OnCreate(JXWindowDirector* parent, JXWidgetSet* container, JXWidgetSet* stop_resize);

	virtual void	AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	virtual void	OnClearMailboxList();
	virtual void	OnOpenMailboxList();
			void	OnOptions();
};

#endif
