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


// Header for CRulesActionLocal class

#ifndef __CRULESACTIONLOCAL__MULBERRY__
#define __CRULESACTIONLOCAL__MULBERRY__

#include "CRulesAction.h"

#include "CCmdEdit.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CMailboxPopup.h"
#include "CPopupButton.h"
#include "CSoundPopup.h"

// Constants

// Classes
class CRulesActionLocal : public CRulesAction
{
private:
	CPopupButton		mPopup1;
	CPopupButton		mPopup2;
	CCmdEdit			mText;
	CMailboxPopup		mMailboxPopup;
	CButton				mSaveSingle;
	CButton				mOptionsBtn;
	CSoundPopup			mSoundPopup;

	CActionItem::CActionReply	mActionReply;
	CActionItem::CActionForward	mActionForward;
	CActionItem::CActionBounce	mActionBounce;
	CActionItem::CActionReject	mActionReject;
	mutable COSStringMap		mActionSound;

public:
					CRulesActionLocal();
	virtual 		~CRulesActionLocal();

	virtual BOOL	Create(const CRect& rect, CRulesDialog* dlog, CWnd* pParentWnd, CView* parentView);

	virtual CActionItem*	GetActionItem() const;
	virtual void			SetActionItem(const CActionItem* spec); 

protected:

	void	InitLabelNames();

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
	afx_msg void	OnSetAction(UINT nID);
	afx_msg void	OnSetFlag(UINT nID);
	afx_msg void	OnMailboxPopup(UINT nID);
	afx_msg void	OnSetOptions();
	afx_msg void	OnSoundPopup(UINT nID);

	DECLARE_MESSAGE_MAP()
};

#endif
