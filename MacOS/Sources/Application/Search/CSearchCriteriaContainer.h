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


// Header for CSearchCriteriaContainer class

#ifndef __CSEARCHCRITERIACONTAINER__MULBERRY__
#define __CSEARCHCRITERIACONTAINER__MULBERRY__

#include "CBroadcaster.h"
#include "CListener.h"

#include "CFilterItem.h"

// Constants
const	PaneIDT		paneid_SearchCriteriaContainer = 1014;
const	PaneIDT		paneid_SearchCriteriaContainerSeparator = 'SEPR';
const	PaneIDT		paneid_SearchCriteriaContainerMore = 'MCHS';
const	PaneIDT		paneid_SearchCriteriaContainerFewer = 'FCHS';
const	PaneIDT		paneid_SearchCriteriaContainerClear = 'CLRS';

// Messages
const	MessageT	msg_SearchCriteriaContainerMore = 'MCHS';
const	MessageT	msg_SearchCriteriaContainerFewer = 'FCHS';
const	MessageT	msg_SearchCriteriaContainerClear = 'CLRS';

// Resources
const	ResIDT		RidL_CSearchCriteriaContainerBtns = 1014;

// Classes
class CSearchItem;
class LPushButton;

class CSearchCriteriaContainer : public LView, public LListener, public CBroadcaster, public CListener
{
	friend class CSearchBase;
	friend class CSearchCriteria;
	friend class CSearchCriteriaLocal;
	friend class CSearchCriteriaSIEVE;

public:
	enum { class_ID = 'SrcP' };

	// Messages for broadcast
	enum
	{
		eBroadcast_SearchCriteriaContainerResized = 'rsiz'
	};

					CSearchCriteriaContainer();
					CSearchCriteriaContainer(LStream *inStream);
	virtual 		~CSearchCriteriaContainer();

	unsigned long	GetCount() const
		{ return mCriteria.GetCount(); }

	void	SetTopLevel()
		{ mTopLevel = true; }
	void	SetRules(bool rules)
		{ mRules = rules; }

protected:
	LWindow*		mContainer;
	LPane*			mSeparator;
	LPushButton*	mMoreBtn;
	LPushButton*	mFewerBtn;
	LPushButton*	mClearBtn;
	bool			mTopLevel;
	bool			mRules;

	CFilterItem::EType	mFilterType;		// Used to toggle local/SIEVE switches

	virtual void	FinishCreateSelf(void);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);				// Respond to clicks in the icon buttons

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	void	DoActivate();

	void	OnMore();
	void	OnFewer();
	void	OnClear();

	void	RecalcLayout();

	void	InitGroup(CFilterItem::EType type, const CSearchItem* spec);
	void	InitCriteria(const CSearchItem* spec = NULL);
	void	AddCriteria(const CSearchItem* spec = NULL, bool use_or = true);
	void	RemoveCriteria();
	void	RemoveAllCriteria();

	CSearchItem*	ConstructSearch() const;

private:
	TArray<LPane*>	mCriteria;
};

#endif
