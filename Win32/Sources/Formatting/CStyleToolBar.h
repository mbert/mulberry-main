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


#ifndef __CSTYLETOOLBAR__MULBERRY__
#define __CSTYLETOOLBAR__MULBERRY__

#include "CContainerView.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CDisplayFormatter.h"
#include "CFontMenu.h"
#include "CSizeMenu.h"
#include "CColorMenu.h"
#include "CMIMESupport.h"




class CContainerView;
class CGrayBackground;
class CIconButton;
class CDisplayFormatter;
class CFontPopup;
class CSizePopup;
class CColorPopup;



class CStyleToolbar : public CGrayBackground
{
	friend class CLetterWindow;
	
public:
	CStyleToolbar();
	virtual ~CStyleToolbar();
	DECLARE_DYNCREATE(CStyleToolbar)
	
	
protected:
	static void 	UpdateUIPos(void);
	
protected:
	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void 	OnSize(UINT nType, int cx, int cy);
	
private:
	CGrayBackground		mToolbar;
	
	CIconButton			mBold;
	CIconButton			mItalic;
	CIconButton			mUnderline;
	
	CFontPopup			mFontMenu;
	CSizePopup			mSize;
	CColorPopup			mColor;
	
	CIconButton			mLeftAlignment;
	CIconButton			mCenterAlignment;
	CIconButton			mRightAlignment;
	
	CDisplayFormatter	*mFormatter;
	EContentSubType		mToolbarType;
	
	DECLARE_MESSAGE_MAP()
	
public:
	void 			SetFormatter(CDisplayFormatter* mFormatter);
	void 			OnSelChange();
	bool 			GetBoldButton();
	bool			GetItalicButton();
	bool			GetUnderlineButton();
	bool			GetLeftButton();
	bool 			GetCenterButton();
	bool			GetRightButton();

	void 			OnStyle(UINT nID);
	void			OnAlignment(UINT nID);
	void 			OnFont(UINT nID);
	void			OnFontSize(UINT nID);
	void 			OnColor(UINT nID);

	void 			OnUpdateStyle(CCmdUI* pCmdUI);
	void 			OnUpdateAlignment(CCmdUI*pCmdUI);
	void 			OnUpdateFont(CCmdUI* pCmdUI);
	void 			OnUpdateSize(CCmdUI* pCmdUI);
	void 			OnUpdateColor(CCmdUI* pCmdUI);

	void			OnEnrichedToolbar();
	void 			OnPlainToolbar();
	void 			OnHTMLToolbar();
	
	
	bool			Is(int nID);
	
	afx_msg void 	OnPlain();
	afx_msg void 	OnBold();
	afx_msg void 	OnItalic();
	afx_msg void 	OnUnderline();
	afx_msg void	OnLeftAlign();
	afx_msg void	OnCenterAlign();
	afx_msg void	OnRightAlign();
};

#endif
