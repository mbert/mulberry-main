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

#include "LWindowHeader.h"
#include "CListener.h"

// Constants
const	PaneIDT		paneid_CToolbarView_Splitter = 'SPLT';

// Messages

// Resources

// Classes
class CToolbar;

class CToolbarView : public LWindowHeader, public CListener
{
	friend class CToolbar;

public:
	typedef std::vector<CToolbarView*> CToolbarViewList;
	static CToolbarViewList sToolbarViews;

	enum { class_ID = 'TBar' };

	enum
	{
		// Standard groups
		eNoGroup = 0,
		eStdButtonsGroup
	};

					CToolbarView(LStream *inStream);
	virtual 		~CToolbarView();

	static void UpdateAllToolbars();

	virtual void	AdaptToSuperFrameSize(
								SInt32		inSurrWidthDelta,
								SInt32		inSurrHeightDelta,
								Boolean		inRefresh);

	virtual void ListenTo_Message(long msg, void* param);

	void	SetSibling(LView* sibling)
		{ mSibling = sibling; }

	void	AddToolbar(CToolbar* toolbar, LCommander* cmdr, unsigned long group = eNoGroup);
	void	ActivateToolbar(CToolbar* tb, bool show);

	void	ShowToolbar(unsigned long group, bool show);
	bool	IsVisible(unsigned long group) const;

	void	UpdateToolbars();					// Force update of visual of all toolbars
	void	UpdateToolbarState();				// Force update of state of all toolbars

protected:
	LView*					mSibling;
	LView*					mSplitter;
	struct SToolbarGroup
	{
		unsigned long		mGroupID;
		unsigned long		mActiveIndex;
		bool				mIsVisible;
		std::vector<CToolbar*>	mToolbars;
	};
	typedef ptrvector<SToolbarGroup> SToolbarGroups;
	SToolbarGroups			mGroups;
	unsigned long			mSideBySideWidth;

	virtual void	FinishCreateSelf();

	SToolbarGroup*	FindGroup(unsigned long group) const;
	SToolbarGroup*	FindGroup(CToolbar* tb, unsigned long& index) const;

			void	AdjustSize();
};

#endif
