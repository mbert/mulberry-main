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


// Header for CPopupButton class

#ifndef __CPOPUPBUTTON__MULBERRY__
#define __CPOPUPBUTTON__MULBERRY__

#include "CIconButton.h"

// Classes
class	CPopupButton : public CIconButton
{

	DECLARE_DYNCREATE(CPopupButton)

public:
					CPopupButton(bool handle = false, bool handle_dlgid = true);
	virtual 		~CPopupButton();

	virtual void	SetMenu(UINT menu_id);
	virtual void	SetText(CRichEditCtrl* text)
						{ mText = text; }
	virtual void	SetButtonText(bool buttonText)
						{ mButtonText = buttonText; }
	virtual void	SetValue(UINT value);
	virtual void	RefreshValue();
	virtual UINT	GetValue(void) const
						{ return mValue; }
	virtual cdstring	GetValueText(void) const;

	afx_msg	void	OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

	virtual void	DrawContent(LPDRAWITEMSTRUCT lpDIS);

	virtual const CMenu* GetPopupMenu(void) const;
	virtual CMenu*	GetPopupMenu(void);
	UINT	GetMenuID() const
		{ return mMenuID; }

protected:
	CMenu*			mMenu;
	UINT			mMenuID;
	UINT			mValue;
	CRichEditCtrl*	mText;
	bool			mButtonText;
	bool			mMainMenu;
	bool			mAlwaysEnable;
	bool			mHandleCommand;
	bool			mHandleSendDlgID;

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
