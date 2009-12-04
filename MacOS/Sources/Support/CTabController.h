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


// CTabController

#ifndef __CTABCONTROLLER__MULBERRY__
#define __CTABCONTROLLER__MULBERRY__

#include <LTabsControl.h>

// Panes
const	PaneIDT		paneid_TabController = 'TABS';

// Messages
const	MessageT	msg_TabController = 'TABS';

class LMultiPanelView;
class CTabPanel;

class CTabController : public LTabsControl
{
public:
	enum { class_ID = 'Ctab' };

					CTabController(LStream *inStream);
	virtual 		~CTabController();

	void	AddPanel(unsigned long id);
	CTabPanel*	GetPanel(unsigned long index);
	CTabPanel*	GetCurrentPanel();
	void	ChangePanel(unsigned long id, unsigned long index);

	void	SetData(void* data);						// Set data
	bool	UpdateData(void* data);						// Force update of data
	void	SetDisplayPanel(unsigned long index);		// Force update of display panel

	void	RemoveTabButtonAt(ArrayIndexT inAtIndex);

protected:
	LMultiPanelView*		mPanels;

	virtual void	FinishCreateSelf(void);					// Do odds & ends

};

#endif
