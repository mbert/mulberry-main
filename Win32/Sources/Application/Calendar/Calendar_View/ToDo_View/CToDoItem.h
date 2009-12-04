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

#ifndef H_CToDoItem
#define H_CToDoItem
#pragma once

#include "CBroadcaster.h"
#include "CCommander.h"

#include <vector>

#include "CICalendarVToDo.h"

class CCalendarTableBase;

// ===========================================================================
//	CToDoItem

class	CToDoItem : public CWnd,
					public CBroadcaster,
					public CCommander
{
public:
	enum
	{
		eBroadcast_EditToDo = 'EdiT',		// param = NULL
		eBroadcast_SelectToDo = 'SelT'			// param = this
	};

	enum EType
	{
		eToDo = 0,
		eOverdue,
		eDueNow,
		eDueLater,
		eDone,
		eCancelled,
		eEmpty
	};

	static CToDoItem* Create(CWnd* parent, const CRect& frame, CCommander* super_commander);

						CToDoItem(CCommander* inSuper);
	virtual				~CToDoItem();

	void SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table);
	void SetDetails(EType type);

	void SelectToDo(bool select, bool silent = false);

	const iCal::CICalendarComponentExpandedShared& GetVToDo() const
		{ return mVToDo; }

protected:
	enum
	{
		eCheckbox_ID = 'CHCK'
	};

	CButton				mCompleted;

	EType									mType;
	iCal::CICalendarComponentExpandedShared	mVToDo;

	CCalendarTableBase*		mTable;
	bool					mIsSelected;
	bool					mIsCompleted;
	uint32_t				mColour;
	cdstring				mSummary;
	cdstring				mStatus;
	cdstring				mTooltipText;
#ifdef _UNICODE
	cdustring				mTooltipTextUTF16;
#endif
	
	virtual void		SetupTagText();

protected:
	// Common updaters
	afx_msg void OnUpdateRealToDo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCompletedToDo(CCmdUI* pCmdUI);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);			// Handle character
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void OnLButtonDblClk(UINT nFlags, CPoint point);			// Double-clicked item
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnEditToDo();
	afx_msg void OnDuplicateToDo();
	afx_msg void OnDeleteToDo();
	afx_msg void OnChangeCompleted();
	afx_msg void OnCompleted();

	DECLARE_MESSAGE_MAP()
};

typedef std::vector<CToDoItem*> CToDoItemList;

#endif
