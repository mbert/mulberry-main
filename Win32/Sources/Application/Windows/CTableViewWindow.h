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


// Header for CTableViewWindow class

#ifndef __CTABLEVIEWWINDOW__MULBERRY__
#define __CTABLEVIEWWINDOW__MULBERRY__

#include "CTableView.h"
#include "CUserAction.h"

// Constants

// Classes
class CTitleTableView;
class CTable;
class CToolbarView;

class CTableViewWindow : public CView
{
public:
					CTableViewWindow();
	virtual 		~CTableViewWindow();

	virtual void	OnInitialUpdate(void);				// Focus
	virtual void	OnDraw(CDC* pDC) {}
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	void		SetTableView(CTableView* tblview)
		{ mTableView = tblview; }
	CTableView*	GetTableView() const
		{ return mTableView; }
	CTable*		GetBaseTable() const
		{ return mTableView->GetBaseTable(); }
	CTitleTableView*	GetBaseTitles() const
		{ return mTableView->GetBaseTitles(); }
	virtual CToolbarView* GetToolbarView() = 0;

	const CUserAction& GetUserPreviewAction() const
		{ return mUserPreviewAction; }
	const CUserAction& GetUserFullViewAction() const
		{ return mUserFullViewAction; }

			void	ResetStandardSize() {}						// Does nothing - provides compatbility with other frameworks
	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

protected:
	CTableView*			mTableView;
	CUserAction			mUserPreviewAction;
	CUserAction			mUserFullViewAction;

	// message handlers
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSaveDefaultState(void);				// Save state in prefs
	afx_msg void OnResetDefaultState(void);

	DECLARE_MESSAGE_MAP()

private:
			void	InitTableViewWindow();
};

#endif
