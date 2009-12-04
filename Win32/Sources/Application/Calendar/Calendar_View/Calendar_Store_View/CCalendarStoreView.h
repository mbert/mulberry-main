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


// Header for CCalendarStoreView class

#ifndef __CCALENDARSTOREVIEW__MULBERRY__
#define __CCALENDARSTOREVIEW__MULBERRY__

#include "CTableView.h"

#include "CCalendarStoreTable.h"
#include "CCalendarStoreTitleTable.h"
#include "CGrayBackground.h"

#include "cdmutexprotect.h"

// Classes
class CCalendarView;
class CCalendarStoreWindow;

// ===========================================================================
//	CCalendarStoreView

class	CCalendarStoreView : public CTableView
{
public:
	typedef vector<CCalendarStoreView*>	CCalendarStoreViewList;
	static cdmutexprotect<CCalendarStoreViewList> sCalendarStoreViews;	// List of windows (protected for multi-thread access)

						CCalendarStoreView();
	virtual				~CCalendarStoreView();

	static bool	ViewExists(const CCalendarStoreView* view);				// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	void SetCalendarView(CCalendarView* view);
	CCalendarView* GetCalendarView()
		{ return mCalendarView; }

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual void	MakeToolbars(CToolbarView* parent);

	virtual bool	TestClose();
	virtual void	DoClose();

	CCalendarStoreWindow* GetCalendarStoreWindow() const;

	CCalendarView*	GetPreview() const
		{ return mCalendarView; }
	void	SetPreview(CCalendarView* view)
		{ mCalendarView = view; }

	virtual const CUserAction& GetPreviewAction() const;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const;		// Return user action data

	CCalendarStoreTable* GetTable() const;
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable();								// Clear all contents out of the view prior to closing

	virtual void	InitColumns();								// Init columns and text
			
	afx_msg	void	OnFileSave();

	virtual void	ResetState(bool force = false);			// Reset window state
	virtual void	SaveDefaultState();						// Save current state as default

protected:
	CCalendarView*				mCalendarView;								// View to preview calendars (3-pane)
	CGrayBackground				mFocusRing;									// Focus ring
	CCalendarStoreTitleTable	mCalTitles;
	CCalendarStoreTable			mCalTable;

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
