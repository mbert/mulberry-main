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


// Header for CMailboxListPanel class

#ifndef __CMAILBOXLISTPANEL__MULBERRY__
#define __CMAILBOXLISTPANEL__MULBERRY__

// Constants
const	PaneIDT		paneid_MailboxListTwist = 'TWST';
const	PaneIDT		paneid_MailboxListShowCaption = 'SHOW';
const	PaneIDT		paneid_MailboxListHideCaption = 'HIDE';
const	PaneIDT		paneid_MailboxListMailboxArea = 'MBXV';
const	PaneIDT		paneid_MailboxListCabinets = 'MSET';
const	PaneIDT		paneid_MailboxListMailboxAdd = 'ADDR';
const	PaneIDT		paneid_MailboxListMailboxClear = 'CLRR';
const	PaneIDT		paneid_MailboxListMailboxList = 'LIST';
const	PaneIDT		paneid_MailboxListTotal = 'TOTL';
const	PaneIDT		paneid_MailboxListFound = 'FUND';
const	PaneIDT		paneid_MailboxListMessages = 'MSGS';
const	PaneIDT		paneid_MailboxListProgress = 'PBAR';

// Messages
const	MessageT	msg_MailboxListTwist = 'TWST';
const	MessageT	msg_MailboxListCabinets = 'MSET';
const	MessageT	msg_MailboxListMailboxAdd = 'ADDR';
const	MessageT	msg_MailboxListMailboxClear = 'CLRR';
const	MessageT	msg_MailboxListMailboxList = 'LIST';

// Resources

// Classes

class CBarPane;
class CMbox;
class CMboxList;
class CMboxRefList;
class CIconTextTable;
class CStaticText;
class LDisclosureTriangle;
class LPushButton;
class LPopupButton;

class CMailboxListPanel
{
	friend class CSearchWindow;
	friend class CDisconnectDialog;

public:
					CMailboxListPanel();
	virtual 		~CMailboxListPanel();

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

	LWindow*			mParent;
	LPane*				mContainer;
	LDisclosureTriangle* mTwister;
	CStaticText*		mShowCaption;
	CStaticText*		mHideCaption;
	LView*				mMailboxArea;
	LPopupButton*		mCabinets;
	LPushButton*		mAddListBtn;
	LPushButton*		mClearListBtn;
	CIconTextTable*		mMailboxList;
	CStaticText*		mTotal;
	CStaticText*		mFound;
	CStaticText*		mMessages;
	CBarPane*			mProgress;
	bool			mTwisted;
	bool			mInProgress;

	virtual void	FinishCreateSelf(LWindow* parent, LPane* container);

	virtual void	OnTwist();

	void	InitCabinets();
	virtual void	OnCabinet(long item);

	virtual void	ResetMailboxList(const CMboxRefList& targets, const ulvector& target_hits);
	virtual void	OnAddMailboxList();
	virtual void	AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	virtual void	OnClearMailboxList();
	virtual void	OnOpenMailboxList();
};

#endif
