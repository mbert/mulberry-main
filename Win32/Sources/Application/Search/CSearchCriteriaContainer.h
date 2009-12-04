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

#include "CBroadcaster.h"
#include "CListener.h"
#include "CWndAligner.h"

#include "CGrayBackground.h"
#include "CFilterItem.h"

// Classes
class CSearchCriteria;
class CSearchItem;

class CCriteriaBase;
typedef vector<CCriteriaBase*> CCriteriaBaseList;

class CSearchCriteriaContainer : public CWnd, public CBroadcaster, public CListener, public CWndAligner
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

				CSearchCriteriaContainer();
	virtual		~CSearchCriteriaContainer();

	virtual BOOL	Create(const CRect& rect, CWnd* pParentWnd);

	unsigned long	GetCount() const
		{ return mCriteriaItems.size(); }

	void	SetTopLevel()
		{ mTopLevel = true; }
	void	SetRules(bool rules)
		{ mRules = rules; }

	bool	SetInitialFocus();

protected:
	CStatic				mBorder;
	CButton				mMoreBtn;
	CButton				mFewerBtn;
	CButton				mClearBtn;
	bool				mTopLevel;
	bool				mRules;

	CFilterItem::EType		mFilterType;		// Used to toggle local/SIEVE switches

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	void	RecalcLayout();

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMore();
	afx_msg void OnFewer();
	afx_msg void OnClear();
	void	InitGroup(CFilterItem::EType type, const CSearchItem* spec);
	void	InitCriteria(const CSearchItem* spec = NULL);
	void	AddCriteria(const CSearchItem* spec = NULL, bool use_or = true);
	void	RemoveCriteria();
	void	RemoveAllCriteria();
	void	SelectNextCriteria(CSearchCriteria* previous);

	CSearchItem*	ConstructSearch() const;

private:
	CCriteriaBaseList		mCriteriaItems;

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
