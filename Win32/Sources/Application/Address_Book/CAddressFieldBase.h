/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Header for CAddressFieldBase class

#ifndef __CAddressFieldBase__MULBERRY__
#define __CAddressFieldBase__MULBERRY__

#include "CWndAligner.h"

#include "CGrayBackground.h"
#include "CToolbarButton.h"
#include "CPopupButton.h"

// Constants

// Classes
class CAddressFieldBase : public CStatic, public CWndAligner
{
public:
					CAddressFieldBase();
	virtual 		~CAddressFieldBase();

	virtual BOOL	Create(const CRect& rect, CWnd* pParentWnd);

	void			SetSingleInstance();
	void			SetMultipleInstance();
	void			SetBottom();
	void			HideType();
	void			HideTitle();

    virtual void    SetDetails(const cdstring& title, int type, const cdstring& data) = 0;
    virtual bool    GetDetails(int& newtype, cdstring& newdata) = 0;
    void            SetMenu(bool typePopup);

    virtual bool	SetInitialFocus() = 0;

protected:
	CGrayBackground		mHeader;
	CToolbarButton		mAdd;
	CToolbarButton		mRemove;
	CStatic             mTitle;
	CPopupButton		mType;
	CWnd*				mDataMove;

    bool                mUsesType;
    int                 mOriginalType;
    cdstring            mOriginalData;

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnAdd();
	afx_msg void	OnRemove();
	afx_msg void	OnSetType(UINT nID);

	DECLARE_MESSAGE_MAP()
};

#endif
