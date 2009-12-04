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


// CAdbkSearchWindow.h

#ifndef __CADBKSEARCHWINDOW__MULBERRY__
#define __CADBKSEARCHWINDOW__MULBERRY__

#include "CTableWindow.h"
#include "CWndAligner.h"

#include "CAdbkServerPopup.h"
#include "CCmdEdit.h"
#include "CCommanderProtect.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CAdbkSearchTable.h"
#include "CAdbkSearchTitleTable.h"
#include "CPopupButton.h"
#include "CWindowStatesFwd.h"

// Classes

class CAdbkSearchWindow : public CTableWindow
{
	DECLARE_DYNCREATE(CAdbkSearchWindow)

	friend class CAdbkSearchTable;

public:
	static CAdbkSearchWindow*		sAdbkSearch;
	static CMultiDocTemplate*		sAdbkSearchDocTemplate;

				CAdbkSearchWindow();
	virtual		~CAdbkSearchWindow();

	static CAdbkSearchWindow* ManualCreate(void);			// Manually create document

	virtual void	OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void	OnDraw(CDC* pDC) {}
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual CAdbkSearchTable* GetTable(void)
						{ return &mTable; }
	virtual void	ResetTable(void);					// Reset the table

	virtual void	InitColumns(void);						// Init columns and text

	virtual void	ResetState(bool force = false);		// Reset state from prefs
	virtual void	SaveDefaultState(void);					// Save state in prefs

private:
	CAdbkSearchTitleTable	mTitles;						// Cached titles
	CAdbkSearchTable		mTable;							// Cached table
	CGrayBackground			mHeader;						// Header pane
	CIconButton				mSearchBtn;						// Search button
	CIconButton				mClearBtn;						// Clear button
	CIconButton				mNewMessageBtn;					// New Message button
	CStatic					mSourceTitle;					// Source title
	CPopupButton			mSourcePopup;					// Source popup
	CStatic					mFieldTitle1;					// Source title
	CStatic					mServersTitle;					// Servers title
	CAdbkServerPopup		mServerPopup;					// Popup for LDAP servers
	CStatic					mFieldTitle2;					// Source title
	CPopupButton			mFieldPopup;					// Field popup
	CPopupButton			mMethodPopup;					// Method popup
	CCmdEdit				mSearchText;					// Search text
	CCommanderProtect		mCmdProtect;					// Protect commands


	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSearchBtn(void);
	afx_msg void OnClearBtn(void);
	afx_msg void OnNewMessageBtn(void);

	afx_msg void OnChangeSource(UINT nID);				// Source popup change
	afx_msg void OnChangeServers(UINT nID);				// Servers popup change
	afx_msg void OnChangeField(UINT nID);				// Field popup change
	afx_msg void OnChangeMethod(UINT nID);				// Method popup change

	DECLARE_MESSAGE_MAP()
};

#endif
