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


// Header for CSMTPWindow class

#ifndef __CSMTPWINDOW__MULBERRY__
#define __CSMTPWINDOW__MULBERRY__

#include "CMailboxWindow.h"

// Classes
#include "CSMTPView.h"
#include "CMessageView.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

class	CSMTPWindow : public CMailboxWindow
{
public:
	static CSMTPWindow*		sSMTPWindow;
	
	CSMTPWindow(JXDirector* owner);
	virtual 		~CSMTPWindow();

	static	void	OpenSMTPWindow();
	static	void	CloseSMTPWindow();

	virtual void OnCreate();

	void	UpdateTitle();

	void	ResetConnection();

	CSMTPView*	GetSMTPView()
		{ return static_cast<CSMTPView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
// begin JXLayout


// end JXLayout

	CToolbarView*	mToolbarView;
	CSplitterView*	mSplitterView;
	CSMTPView*		mSMTPView;
	CMessageView*	mMessageView;
};

#endif
