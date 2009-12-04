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

#include "CBroadcaster.h"
#include "CListener.h"
#include "CToolbarManager.h"

// Constants

// Messages

// Resources

// Classes
class LBevelButton;
class CToolbarButton;

class CToolbar : public LView,
					public LListener,
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

					CToolbar();
					CToolbar(LStream *inStream);
	virtual 		~CToolbar();

	static void	PrefsChanged();
	static bool	RulesChanged();

	virtual void ListenTo_Message(long msg, void* param);

	void	SetCommander(LCommander* cmdr);			// Set the commander to send commands to
	void	AddCommander(LCommander* cmdr);			// Add a commander to send commands to
	void	RemoveCommander(LCommander* cmdr);		// Remove a commander to send commands to

	void	UpdateToolbarState();					// Force update of state if items visible

	bool	IsVisible() const
		{ return mShowIt; }
	unsigned long GetMinimumWidth() const;
	unsigned long GetActualWidth() const;

	void	AddItem(LPane* item, const CToolbarItem::CToolbarItemInfo& details);
	void	AddButton(LBevelButton* btn, const CToolbarItem::CToolbarItemInfo& details);

	bool	HandleContextMenuEvent(const EventRecord& cmmEvent);

	virtual void		ResizeFrameBy(
								SInt16		inWidthDelta,
								SInt16		inHeightDelta,
								Boolean		inRefresh);

protected:
	struct SMessageCommandMap
	{
		MessageT msg;
		CommandT cmd;
	};

	typedef std::vector<CToolbar*> CToolbarList;
	typedef std::vector<LCommander*> CCommanderArray;

	static CToolbarList	sToolbars;

	CToolbarManager::EToolbarType	mType;
	unsigned long 		mLeftJustOffset;

	bool				mIs3Pane;
	bool				mShowIt;
	bool				mSmallIcons;
	bool				mShowIcons;
	bool				mShowCaptions;
	CCommanderArray		mCmdrs;
	mutable LCommander*	mLastCommander;

	struct SItemSpec
	{
		SItemSpec(LPane* ctrl, const CToolbarItem::CToolbarItemInfo& details) :
			mWnd(ctrl), mDetails(details) {}

		LPane*				mWnd;
		CToolbarItem::CToolbarItemInfo	mDetails;
	};
	typedef std::vector<SItemSpec> CItemArray;

	CItemArray mItemList;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

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
	LPane*			GetItem(CToolbarManager::EToolbarItem item) const;
	unsigned long	GetItemIndex(LPane* item) const;

			void	RemoveButton(LPane* pane);
			void	MoveButton(LPane* pane);
			void	ResetButtons(bool use_default);

			void	ShowToolbar(bool show);

			void	SmallIcons(bool small_icons);
			void	ShowIcons(bool show);	
			void	ShowCaptions(bool show);	

			void	Reset();
			void	ResetLayout();
			void	PositionButton(Rect& btnrect);
			Point	GetBtnSize(const CToolbarButton* tb = NULL) const;

	LCommander*		GetCommander() const;

	virtual void	DoUpdateToolbarState();						// Force update of state
			void	UpdateControlState(LControl* ctrl, CommandT cmd, bool caption = false)
				{ UpdateControl(ctrl, cmd, true, false, caption); }
			void	UpdateControlVisibility(LControl* ctrl, CommandT cmd, bool caption = false)
				{ UpdateControl(ctrl, cmd, false, true, caption); }
			void	UpdateControl(LControl* ctrl, CommandT cmd, bool enable, bool show, bool caption = false);
			void	UpdateControlValue(LControl* ctrl, const CommandT* cmd, bool caption = false);
			bool	UpdateRulesButtons();

private:
	friend class CToolbarTrackAttachment;
	class CToolbarTrackAttachment : public LAttachment
	{
	public:
		CToolbarTrackAttachment(CToolbar* tb) :
			LAttachment(msg_Event, true), mTb(tb) {}

		void SetToolbar(CToolbar* tb)
		{
			mTb = tb;
		}

	protected:
		CToolbar* mTb;

		virtual void	ExecuteSelf(MessageT inMessage, void* ioParam)
		{
			if (mTb != NULL)
				SetExecuteHost(mTb->TrackMove(static_cast<EventRecord*>(ioParam), this));
		}
	};
	static CToolbarTrackAttachment* sTracker;
	unsigned long mDragIndex;
	bool	mFirstMouseDown;

	bool TrackMove(EventRecord* event, CToolbarTrackAttachment* tracker);
	void TrackStart(LPane* pane);
	void TrackStop();
	void TrackMouseMove();
	void TrackHighlight();
};

#endif
