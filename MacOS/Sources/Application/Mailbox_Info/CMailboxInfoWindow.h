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


// Header for CMailboxInfoWindow class

#ifndef __CMAILBOXINFOWINDOW__MULBERRY__
#define __CMAILBOXINFOWINDOW__MULBERRY__

#include "CMailboxWindow.h"

#include "CMailboxInfoView.h"

// Constants
const	PaneIDT		paneid_MailboxWindow = 1000;
const	PaneIDT		paneid_MailboxToolbarView = 'TBar';

// Classes
class CToolbarView;

class	CMailboxInfoWindow : public CMailboxWindow
{
	friend class CMailboxInfoView;

public:
	enum { class_ID = 'MbWi' };

					CMailboxInfoWindow();
					CMailboxInfoWindow(LStream *inStream);
	virtual 		~CMailboxInfoWindow();

	static  CMailboxWindow*	OpenWindow(CMbox* mbox, bool is_search = false);

	CMailboxInfoView*	GetMailboxInfoView()
		{ return static_cast<CMailboxInfoView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
	CToolbarView*	mToolbarView;

	virtual void	FinishCreateSelf(void);

public:
	virtual void	AttemptClose(void);
};

#endif
