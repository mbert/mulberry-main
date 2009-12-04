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


// Header for CServerView class

#ifndef __CSERVERVIEW__MULBERRY__
#define __CSERVERVIEW__MULBERRY__

#include "CTableView.h"

#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CServerTable.h"
#include "CServerTitleTable.h"

#include "cdmutexprotect.h"

// Classes
class CMailboxInfoView;
class CServerWindow;

class CServerView : public CTableView
{
	friend class CServerWindow;

public:
	typedef vector<CServerView*>	CServerViewList;
	static cdmutexprotect<CServerViewList> sServerViews;	// List of windows (protected for multi-thread access)

	enum { class_ID = 'SeVi' };

					CServerView();
	virtual 		~CServerView();

	static bool	ViewExists(const CServerView* view);				// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	virtual void	MakeToolbars(CToolbarView* parent);

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual void	SetManager();								// Set as manager
	virtual bool	IsManager()
		{ return GetTable()->IsManager(); }
	virtual void	SetServer(CMboxProtocol* server);				// Set server
	virtual CMboxProtocol*	GetServer() const					// Get server
		{ return GetTable()->GetServer(); }

	void	DoPreview(CMbox* mbox, bool use_dynamic = true, bool is_search = false)		// Preview a mailbox
		{ GetTable()->DoPreview(mbox, use_dynamic, is_search); }
	bool	IsPreviewed(const CMbox* mbox) const
		{ return GetTable()->IsPreviewed(mbox); }

	CMailboxInfoView*	GetPreview() const
		{ return mMailboxView; }
	void	SetPreview(CMailboxInfoView* view)
		{ mMailboxView = view; }

	CServerWindow* GetServerWindow() const;

	virtual const CUserAction& GetPreviewAction() const;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const;		// Return user action data

	CIconButton* GetHierarchyBtn()
		{ return &mFlatHierarchyBtn; }
	void	DoFlatHierarchy(void);							// Change to flat view of hierarchy

	CServerTable* GetTable() const
		{ return static_cast<CServerTable*>(mTable); }
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable()								// Clear all contents out of the view prior to closing
		{ GetTable()->ClearTable(); }

	void	LogonServer()										// Update the list
		{ GetTable()->LogonServer(); }
	void	OpenNewMailbox(CMbox* mbox, bool subscribe)			// Open mailbox
		{ GetTable()->OpenTheMailbox(mbox, subscribe); }

			void	ChangedMbox(const CMbox* aMbox);			// Mailbox status changed
	virtual void	InitColumns();								// Init columns and text

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();						// Save current state as default

protected:
	CMailboxInfoView*	mMailboxView;						// View to preview mailboxes (3-pane)
	CGrayBackground		mFocusRing;							// Focus ring
	CServerTitleTable	mServerTitles;						// Cached titles
	CServerTable		mServerTable;						// Cached table
	CIconButton			mFlatHierarchyBtn;					// Flat/hierarchy button

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);

	DECLARE_MESSAGE_MAP()
};

#endif
