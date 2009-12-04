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


// Header for CAdbkManagerView class

#ifndef __CADBKMANAGERVIEW__MULBERRY__
#define __CADBKMANAGERVIEW__MULBERRY__

#include "CTableView.h"

#include "CGrayBackground.h"
#include "CAdbkManagerTable.h"
#include "CAdbkManagerTitleTable.h"

#include "cdmutexprotect.h"

// Constants

// Classes
class CAdbkManagerWindow;
class CAddressBookView;

class CAdbkManagerView : public CTableView
{
	friend class CAdbkManagerWindow;

public:
	typedef vector<CAdbkManagerView*>	CAdbkManagerViewList;
	static cdmutexprotect<CAdbkManagerViewList>	sAdbkManagerViews;

					CAdbkManagerView();
	virtual 		~CAdbkManagerView();

	virtual const CUserAction& GetPreviewAction() const;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const;		// Return user action data

	CAddressBookView*	GetPreview() const
		{ return mAddressBookView; }
	void	SetPreview(CAddressBookView* view)
		{ mAddressBookView = view; }

	void	DoPreview(CAddressBook* adbk)						// Preview a address book
		{ GetTable()->DoPreview(adbk); }
		
	virtual void	MakeToolbars(CToolbarView* parent);

	virtual bool	TestClose();
	virtual void	DoClose();

	CAdbkManagerWindow* GetAdbkManagerWindow() const;

	CAdbkManagerTable* GetTable() const
		{ return static_cast<CAdbkManagerTable*>(mTable); }
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable()								// Clear the entire table
		{ GetTable()->ClearTable(); }

	virtual void	InitColumns();							// Init columns and text
	
	virtual void	ResetState(bool force = false);			// Reset window state
	virtual void	SaveState();							// Save current state as default
	virtual void	SaveDefaultState();						// Save current state as default

protected:
	CAddressBookView*		mAddressBookView;
	CGrayBackground			mFocusRing;							// Focus ring
	CAdbkManagerTitleTable	mAdbkTitles;						// Cached titles
	CAdbkManagerTable		mAdbkTable;							// Cached table

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
