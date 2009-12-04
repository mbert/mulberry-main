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


// CSearchCriteriaContainer.h

#ifndef __CSEARCHCRITERIACONTAINER__MULBERRY__
#define __CSEARCHCRITERIACONTAINER__MULBERRY__

#include <JXWidgetSet.h>
#include "CBroadcaster.h"
#include "CListener.h"

#include "CFilterItem.h"

// Classes
class CSearchCriteria;
class CSearchItem;
class JXEngravedRect;
class JXTextButton;
class JXWidget;

class CCriteriaBase;
typedef std::vector<CCriteriaBase*> CCriteriaBaseList;

class CSearchCriteriaContainer : public JXWidgetSet, public CBroadcaster, public CListener
{
	friend class CSearchBase;
	friend class CSearchCriteria;
	friend class CSearchCriteriaLocal;
	friend class CSearchCriteriaSIEVE;

public:
	// Messages for broadcast
	enum
	{
		eBroadcast_SearchCriteriaContainerResized = 'rsiz'
	};

				CSearchCriteriaContainer(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual		~CSearchCriteriaContainer();

	virtual void	OnCreate();

	unsigned long	GetCount() const
		{ return mCriteriaItems.size(); }

	void	SetTopLevel()
		{ mTopLevel = true; }
	void	SetRules(bool rules)
		{ mRules = rules; }

protected:
// begin JXLayout1

    JXEngravedRect* mBorder;
    JXTextButton*   mMoreBtn;
    JXTextButton*   mFewerBtn;
    JXTextButton*   mClearBtn;

// end JXLayout1
	bool					mTopLevel;
	bool					mRules;
	CFilterItem::EType		mFilterType;		// Used to toggle local/SIEVE switches

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	void	RecalcLayout();

	// message handlers
	void OnMore();
	void OnFewer();
	void OnClear();

	// message handlers
	void	InitGroup(CFilterItem::EType type, const CSearchItem* spec);
	void	InitCriteria(const CSearchItem* spec = NULL);
	void	AddCriteria(const CSearchItem* spec = NULL, bool use_or = true);
	void	RemoveCriteria(unsigned long num = 1);
	void	RemoveAllCriteria();
	void	SelectNextCriteria(CSearchCriteria* previous);

	CSearchItem*	ConstructSearch() const;

private:
	CCriteriaBaseList		mCriteriaItems;

};

#endif
