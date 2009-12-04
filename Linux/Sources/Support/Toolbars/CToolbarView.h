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

#include <JXEngravedRect.h>
#include "CListener.h"

// Classes
class CToolbar;
class CCommander;
class JXWidgetSet;

class CToolbarView : public JXEngravedRect, public CListener
{
	friend class CToolbar;

public:
	typedef std::vector<CToolbarView*> CToolbarViewList;
	static CToolbarViewList sToolbarViews;

	enum
	{
		// Standard groups
		eNoGroup = 0,
		eStdButtonsGroup
	};

					CToolbarView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CToolbarView();

	virtual void	OnCreate();

	static void UpdateAllToolbars();

	virtual void ListenTo_Message(long msg, void* param);

	void	SetSibling(JXWidgetSet* sibling)
		{ mSibling = sibling; }

	void	AddToolbar(CToolbar* toolbar, CCommander* cmdr, unsigned long group = eNoGroup);
	void	ActivateToolbar(CToolbar* tb, bool show);

	void	ShowToolbar(unsigned long group, bool show);
	bool	IsVisible(unsigned long group) const;

	void	UpdateToolbars();					// Force update of visual of all toolbars
	void	UpdateToolbarState();				// Force update of state of all toolbars

protected:
	JXWidgetSet*			mSibling;
	typedef std::vector<CToolbar*> SToolbars;
	SToolbars				mToolbars;
	bool					mIsVisible;
	unsigned long			mActiveIndex;

			void	AdjustToolbarSize();
};

#endif
