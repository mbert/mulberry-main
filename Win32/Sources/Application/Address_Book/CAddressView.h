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


// Header for CAddressView class

#ifndef __CADDRESSVIEW__MULBERRY__
#define __CADDRESSVIEW__MULBERRY__

#include "CBaseView.h"

#include "CAddressPreview.h"
#include "CGroupPreview.h"

#include "cdmutexprotect.h"

// Classes
class CAddressBook;
class CAdbkAddress;
class CGroup;

class CAddressView : public CBaseView
{
public:
	typedef vector<CAddressView*>	CAddressViewList;
	static cdmutexprotect<CAddressViewList>	sAddressViews;

					CAddressView();
	virtual 		~CAddressView();

	static bool	ViewExists(const CAddressView* wnd);			// Check for view

	virtual void	DoClose();

	void			SetAddress(CAddressBook* adbk, CAdbkAddress* addr, bool refresh = false);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const;

	void		SetGroup(CAddressBook* adbk, CGroup* grp, bool refresh = false);
	void		ClearGroup();
	CGroup*		GetGroup() const;

	virtual void	MakeToolbars(CToolbarView* parent) {}

	virtual bool	HasFocus() const;
	virtual void	Focus();

protected:
	CAddressPreviewBase*	mCurrent;
	CAddressPreview			mAddressPreview;
	CGroupPreview			mGroupPreview;
	
			void	InitPreviews();

public:
	virtual void	ResetState(bool force = false) {}			// Reset window state
	virtual void	SaveDefaultState() {}						// Save current state as default

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
