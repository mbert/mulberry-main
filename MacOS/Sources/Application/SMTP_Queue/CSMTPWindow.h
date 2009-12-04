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

#include "CSMTPView.h"

// Panes
const	PaneIDT		paneid_SMTPWindow = 1500;
const	PaneIDT		paneid_SMTPToolbarView = 'TBar';

// Messages

// Classes
class CToolbarView;

class	CSMTPWindow : public CMailboxWindow
{
public:
	enum { class_ID = 'SmWi' };

	static CSMTPWindow*		sSMTPWindow;
	
					CSMTPWindow();
					CSMTPWindow(LStream *inStream);
	virtual 		~CSMTPWindow();

	static	void	OpenSMTPWindow();
	static	void	CloseSMTPWindow();

			void	UpdateTitle();
			void	ResetConnection();

	CSMTPView*	GetSMTPView()
		{ return static_cast<CSMTPView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
	CToolbarView*	mToolbarView;

	virtual void	FinishCreateSelf(void);
};

#endif
