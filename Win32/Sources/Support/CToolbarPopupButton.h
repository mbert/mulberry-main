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


// Header for CToolbarPopupButton class

#ifndef __CTOOLBARPOPUPBUTTON__MULBERRY__
#define __CTOOLBARPOPUPBUTTON__MULBERRY__

#include "CToolbarButton.h"

// Classes
class	CToolbarPopupButton : public CToolbarButton
{

	DECLARE_DYNCREATE(CToolbarPopupButton)

public:
					CToolbarPopupButton();
	virtual 		~CToolbarPopupButton();

	virtual bool 	HasPopup() const;

	virtual void	SetMenu(UINT menu_id);
	virtual void	SetText(CRichEditCtrl* text)
						{ mText = text; }
	virtual void	SetValue(UINT value);
	virtual void	RefreshValue();
	virtual UINT	GetValue(void) const
						{ return mValue; }

	afx_msg	void	OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

	virtual const CMenu* GetPopupMenu(void) const;
	virtual CMenu*	GetPopupMenu(void);
	UINT	GetMenuID() const
		{ return mMenuID; }

protected:
	CMenu*			mMenu;
	UINT			mMenuID;
	UINT			mValue;
	CRichEditCtrl*	mText;
	bool			mMainMenu;
	bool			mAlwaysEnable;
	bool			mPopupSetValue;

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

			void	LButtonDownBtn(UINT nFlags, CPoint point);
			void	LButtonDownPopup(UINT nFlags, CPoint point);

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
