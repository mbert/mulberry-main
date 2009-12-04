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


// Header for CRulesTarget class

#ifndef __CRULESTARGET__MULBERRY__
#define __CRULESTARGET__MULBERRY__

#include "CCriteriaBase.h"

#include "CFilterTarget.h"

#include "CCmdEdit.h"
#include "CGrayBackground.h"
#include "CMailboxPopup.h"
#include "CPopupButton.h"

// Constants

// Classes
class CTargetsDialog;

class CRulesTarget : public CCriteriaBase
{
private:
	CPopupButton		mPopup1;
	CPopupButton		mPopup2;
	CPopupButton		mPopup3;
	CCmdEdit			mText;
	CMailboxPopup		mMailboxPopup;

	CTargetsDialog*		mDlog;

public:
					CRulesTarget();
	virtual 		~CRulesTarget();

	virtual BOOL	Create(const CRect& rect, CTargetsDialog* dlog, CWnd* pParentWnd, CView* parentView);

	CFilterTarget*	GetFilterTarget() const;
	void			SetFilterTarget(const CFilterTarget* spec); 

protected:

	virtual CCriteriaBaseList& GetList();
	virtual void	SwitchWith(CCriteriaBase* other);

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
	afx_msg void	OnSetTarget(UINT nID);
	afx_msg void	OnSetCabinet(UINT nID);
	afx_msg void	OnSetAccount(UINT nID);
	afx_msg void	OnMailboxPopup(UINT nID);

	DECLARE_MESSAGE_MAP()

private:
			void	InitCabinetMenu();			// Set up cabinet menu
			void	InitAccountMenu();			// Set up account menu
};

#endif
