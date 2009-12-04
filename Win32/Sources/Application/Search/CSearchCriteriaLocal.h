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


// Header for CSearchCriteriaLocal class

#ifndef __CSEARCHCRITERIALOCAL__MULBERRY__
#define __CSEARCHCRITERIALOCAL__MULBERRY__

#include "CSearchCriteria.h"

#include "CCmdEdit.h"
#include "CDateControl.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CPopupButton.h"
#include "CSearchItem.h"

// Constants

// Classes
class CCmdEdit;
class CDateControl;
class CPopupButton;
class CSearchItem;

class CSearchCriteriaLocal : public CSearchCriteria
{
private:
	CPopupButton		mPopup1;
	CPopupButton		mPopup2;
	CPopupButton		mPopup3;
	CPopupButton		mPopup4;
	CPopupButton		mPopup5;
	CPopupButton		mPopup6;
	CPopupButton		mPopup7;
	CPopupButton		mPopup8;
	CCmdEdit			mText1;
	CCmdEdit			mText2;
	CCmdEdit			mText3;
	CDateControl		mDate;

public:
					CSearchCriteriaLocal();
	virtual 		~CSearchCriteriaLocal();

	virtual BOOL	Create(const CRect& rect, CWnd* pParentWnd);

	virtual void	SetRules(bool rules);

	virtual long	ShowOrAnd(bool show);
	virtual bool	IsOr() const;
	virtual void	SetOr(bool use_or);

	virtual CSearchItem*	GetSearchItem() const;
	virtual void			SetSearchItem(const CSearchItem* spec, bool negate = false); 

	virtual bool			SetInitialFocus();
	
protected:

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSetCriteria(UINT nID);
	afx_msg void	OnSetMethod(UINT nID);
	afx_msg void	OnSetSize(UINT nID);
	afx_msg void	OnSetDateRel(UINT nID);
	afx_msg void	OnSetDateWithin(UINT nID);
	afx_msg void	OnSetMode(UINT nID);
	afx_msg void	OnSetSearchSet(UINT nID);
	afx_msg void	OnSetSearchSetMethod(UINT nID);
	afx_msg void	OnTextReturn();
	
	void	InitLabelNames();
	void	InitSearchSets();

	CSearchItem* ParseAddress(CSearchItem::ESearchType type) const;
	CSearchItem* ParseDate(bool sent) const;
	CSearchItem* ParseText(CSearchItem::ESearchType type) const;
	CSearchItem* ParseSize() const;
	CSearchItem* ParseFlag(CSearchItem::ESearchType type1, CSearchItem::ESearchType type2) const;
	CSearchItem* ParseLabel(CSearchItem::ESearchType type, unsigned long index) const;
	
	long GetDatePopup(const CSearchItem* spec, long original, cdstring& text1, cdstring& text3, long& popup5, long& popup6) const;

	DECLARE_MESSAGE_MAP()
};

#endif
