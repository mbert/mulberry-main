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

// Classes
#include "CMailboxInfoView.h"
#include "CMessageView.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

class CMailboxInfoWindow : public CMailboxWindow
{
public:
	friend class CMailboxInfoView;

	CMailboxInfoWindow(JXDirector* owner);
	virtual ~CMailboxInfoWindow();

	static  CMailboxWindow*	OpenWindow(CMbox* mbox, bool is_search = false);
	static CMailboxInfoWindow* ManualCreate();

	virtual void OnCreate();

	virtual JBoolean Close();							// Attempt close

	CMailboxInfoView*	GetMailboxInfoView()
		{ return mMailboxInfoView; }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
// begin JXLayout


// end JXLayout
	CToolbarView*		mToolbarView;
	CSplitterView*		mSplitterView;
	CMailboxInfoView*	mMailboxInfoView;
	CMessageView*		mMessageView;
};

#endif
