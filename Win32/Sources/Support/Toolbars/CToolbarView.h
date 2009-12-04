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


// Header for CToolbarView class

#ifndef __CTOOLBARVIEW__MULBERRY__
#define __CTOOLBARVIEW__MULBERRY__

#include "CWndAligner.h"
#include "CListener.h"

// Constants

// Messages

// Resources

// Classes
class CCommander;
class CToolbar;

class CToolbarView : public CControlBar, public CWndAligner, public CListener
{
	friend class CToolbar;

public:
	typedef vector<CToolbarView*> CToolbarViewList;
	static CToolbarViewList sToolbarViews;

	enum
	{
		// Standard groups
		eNoGroup = 0,
		eStdButtonsGroup
	};

					CToolbarView();
	virtual 		~CToolbarView();

	static void UpdateAllToolbars();

	virtual void ListenTo_Message(long msg, void* param);

	void	SetSibling(CWnd* sibling)
		{ mSibling = sibling; }

	void	AddToolbar(CToolbar* toolbar, CCommander* cmdr, unsigned long group = eNoGroup);
	void	ActivateToolbar(CToolbar* tb, bool show);

	void	ShowToolbar(unsigned long group, bool show);
	bool	IsVisible(unsigned long group) const;

	void	ShowDivider(bool show);

	void	UpdateToolbars();					// Force update of visual of all toolbars
	void	UpdateToolbarState();				// Force update of state of all toolbars

	virtual CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz );
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	CWnd*					mSibling;
	CStatic					mDivider;
	struct SToolbarGroup
	{
		unsigned long		mGroupID;
		unsigned long		mActiveIndex;
		bool				mIsVisible;
		vector<CToolbar*>	mToolbars;
	};
	typedef ptrvector<SToolbarGroup> SToolbarGroups;
	SToolbarGroups			mGroups;
	unsigned long			mSideBySideWidth;
	int						mHeight;
	bool					mShowDivider;

	SToolbarGroup*	FindGroup(unsigned long group) const;
	SToolbarGroup*	FindGroup(CToolbar* tb, unsigned long& index) const;

			void	AdjustSize();

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif
