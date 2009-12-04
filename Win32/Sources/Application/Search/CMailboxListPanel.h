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


// CMailboxListPanel.h

#ifndef __CMAILBOXLISTPANEL__MULBERRY__
#define __CMAILBOXLISTPANEL__MULBERRY__

#include "CGrayBackground.h"
#include "CIconTextTable.h"
#include "CPopupButton.h"
#include "CProgress.h"
#include "CTwister.h"

// Classes
class CMbox;
class CMboxList;
class CMboxRefList;

class CMailboxListPanel : public CGrayBackground
{
	friend class CSearchWindow;
	friend class CDisconnectDialog;

public:
				CMailboxListPanel();
	virtual		~CMailboxListPanel();

	virtual void CreateSelf(CWnd* parent_frame, CWnd* move_parent1, CWnd* move_parent2, int width, int height);			// Manually create document

	void SetProgress(unsigned long progress);
	void SetFound(unsigned long found);
	void SetMessages(unsigned long msgs);
	void SetHitState(unsigned long item, bool hit, bool clear = false);

	virtual void SetInProgress(bool in_progress);

protected:
	enum EMboxListState
	{
		eNotHit = 0,
		eHit,
		eMiss
	};

	CWnd*					mParentFrame;
	CWnd*					mMoveParent1;
	CWnd*					mMoveParent2;
	CTwister				mTwister;
	CStatic					mShowCaption;
	CStatic					mHideCaption;
	CStatic					mDivider;
	CGrayBackground			mMailboxArea;
	CStatic					mMailboxBorder;
	CStatic					mCabinetsTitle;
	CPopupButton			mCabinets;
	CButton					mAddListBtn;
	CButton					mClearListBtn;
	CIconTextTable			mMailboxList;
	CGrayBackground			mProgressArea;
	CStatic					mTotalTitle;
	CStatic					mTotal;
	CStatic					mFoundTitle;
	CStatic					mFound;
	CStatic					mMessagesTitle;
	CStatic					mMessages;
	CBarPane				mProgress;
	bool					mTwisted;
	bool					mInProgress;

	// message handlers
	afx_msg void OnTwist();

	void	InitCabinets();
	afx_msg void OnCabinet(UINT nID);

	void	ResetMailboxList(const CMboxRefList& targets, const ulvector& target_hits);
	afx_msg void OnAddMailboxList();
	virtual void AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	afx_msg void OnClearMailboxList();
	afx_msg void OnOpenMailboxList();

	DECLARE_MESSAGE_MAP()
};

#endif
