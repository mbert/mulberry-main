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


// Header for CToolbar class

#ifndef __CTOOLBAR__MULBERRY__
#define __CTOOLBAR__MULBERRY__

#include <JXWidgetSet.h>
#include "CBroadcaster.h"
#include "CListener.h"
#include "CToolbarManager.h"

#include "HPopupMenu.h"

// Classes
class CCommander;
class JXFlatRect;
class JXImageButton;
class CToolbarButton;

class CToolbar : public JXWidgetSet,
					public CBroadcaster,
					public CListener
{
	friend class CToolbarView;

public:
	enum
	{
		eBroadcast_ToolbarActivate = 'TBac',
		eBroadcast_ToolbarDeactivate = 'TBda'
	};

					CToolbar(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CToolbar();

	virtual void	OnCreate();

	static void	PrefsChanged();
	static bool	RulesChanged();

	virtual void ListenTo_Message(long msg, void* param);

	void	SetCommander(CCommander* cmdr);			// Set the commander to send commands to
	void	AddCommander(CCommander* cmdr);			// Add a commander to send commands to
	void	RemoveCommander(CCommander* cmdr);		// Remove a commander to send commands to

	void	UpdateToolbarState();					// Force update of state if items visible

	bool	IsVisible() const
		{ return mShowIt; }
	unsigned long GetMinimumWidth() const;
	unsigned long GetActualWidth() const;

	void	AddItem(JXWidget* item, const CToolbarItem::CToolbarItemInfo& details);
	void	AddButton(CToolbarButton* btn, const CToolbarItem::CToolbarItemInfo& details);

protected:
	typedef std::vector<CToolbar*> CToolbarList;
	typedef std::vector<CCommander*> CCommanderArray;

	static CToolbarList	sToolbars;

	CToolbarManager::EToolbarType	mType;
	unsigned long 		mLeftJustOffset;

	bool				mIs3Pane;
	bool				mShowIt;
	bool				mSmallIcons;
	bool				mShowIcons;
	bool				mShowCaptions;
	CCommanderArray		mCmdrs;
	mutable CCommander*	mLastCommander;

	HPopupMenu*			mContextPopup;
	HPopupMenu*			mContextAddPopup;
	HPopupMenu*			mContextRulesPopup;
	
	JXWidget*			mContextItem;
	JXWidget*			mHighlighter;
	JSize				mContextAddItem;
	JSize				mApplyRulesIndex;
	JSize				mDragIndex;

	struct SItemSpec
	{
		SItemSpec(JXWidget* ctrl, const CToolbarItem::CToolbarItemInfo& details) :
			mWnd(ctrl), mDetails(details) {}

		JXWidget* mWnd;
		CToolbarItem::CToolbarItemInfo	mDetails;
	};
	typedef std::vector<SItemSpec> CItemArray;

	CItemArray mItemList;
	
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);

	virtual void	HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers);

	virtual void	HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
			void 	ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
			void	ContextResult(HPopupMenu* menu, JIndex index);

	CToolbarManager::EToolbarType GetType() const
	{
		return mType;
	}
	void SetType(CToolbarManager::EToolbarType type)
	{
		mType = type;
	}

			void	BuildToolbar();

	CToolbarButton*	GetButton(CToolbarManager::EToolbarItem item) const;
	CToolbarButton*	GetButton(CToolbarManager::EToolbarItem item, const cdstring& extra) const;
	JXWidget*		GetItem(CToolbarManager::EToolbarItem item) const;
	unsigned long	GetItemIndex(JXWidget* item) const;

			void	RemoveButton(JXWidget* pane);
			void	MoveButton(JXWidget* pane);
			void	ResetButtons(bool use_default);

			void	ShowToolbar(bool show);

			void	SmallIcons(bool small_icons);
			void	ShowIcons(bool show);	
			void	ShowCaptions(bool show);	

			void	Reset();
			void	ResetLayout();
			void	PositionButton(JRect& btnrect);
			JPoint	GetBtnSize(const CToolbarButton* tb = NULL) const;

	CCommander*		GetCommander() const;

	virtual void	DoUpdateToolbarState();						// Force update of state
			void	UpdateControlState(JXWidget* ctrl, unsigned long cmd, bool caption = false)
				{ UpdateControl(ctrl, cmd, true, false, caption); }
			void	UpdateControlVisibility(JXWidget* ctrl, unsigned long cmd, bool caption = false)
				{ UpdateControl(ctrl, cmd, false, true, caption); }
			void	UpdateControl(JXWidget* ctrl, unsigned long cmd, bool enable, bool show, bool caption = false);
			void	UpdatePopupControl(JXWidget* ctrl, unsigned long cmd);
			bool	UpdateRulesButtons();

	bool TrackMove(const JPoint& pt, const JXKeyModifiers& modifiers);
	void TrackStart(JXWidget* pane);
	void TrackStop();
	void TrackMouseMove(const JPoint& pt);
	void TrackHighlight();
};

#endif
