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


// Header for C3PaneAccounts class

#ifndef __C3PANEACCOUNTS__MULBERRY__
#define __C3PANEACCOUNTS__MULBERRY__

#include "C3PaneParentPanel.h"
#include "CToolbarButton.h"

// Classes
class CAdbkManagerView;
class CCalendarStoreView;
class CServerView;
class C3PaneMailAccount;
class C3PaneContactsAccount;
class C3PaneCalendarAccount;

class C3PaneAccounts : public C3PaneParentPanel
{
public:
					C3PaneAccounts();
	virtual 		~C3PaneAccounts();

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual void	ResetState();						// Reset state from prefs
	virtual void	SaveDefaultState();					// Save state in prefs
			
	virtual void	SetViewType(N3Pane::EViewType view);

	CServerView*		GetServerView() const;
	CAdbkManagerView*	GetContactsView() const;
	CCalendarStoreView*	GetCalendarStoreView() const;

protected:
	CTabCtrl				mTabs;
	CToolbarButton			mZoom;
	C3PaneMailAccount*		mMailAccounts;
	C3PaneContactsAccount*	mContactsAccounts;
	C3PaneCalendarAccount*	mCalendarAccounts;

private:
			void	MakeMail();
			void	MakeContacts();
			void	MakeCalendar();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnZoomPane(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
