/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CContainerWnd.h"
#include "CBroadcaster.h"
#include "CIconButton.h"
#include "CListener.h"
#include "CToolbarManager.h"

// Constants

// Messages

// Resources

// Classes

class CIconButton;
class CCommander;
typedef ptrvector<CButton> CButtonArray;
typedef ptrvector<CWnd> CWndArray;
typedef std::vector<CCommander*> CCommanderArray;
class CToolbarButton;
class CToolbarView;

class CToolbar : public CContainerWnd,
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
	virtual 		~CToolbar();

			void InitToolbar(bool is_3pane, CToolbarView* parent);

	static void	PrefsChanged();
	static bool	RulesChanged();

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual void ListenTo_Message(long msg, void* param);

	void	SetCommander(CCommander* cmdr);				// Set the commander to send commands to
	void	AddCommander(CCommander* cmdr);				// Add a commander to send commands to
	void	RemoveCommander(CCommander* cmdr);			// Remove a commander to send commands to

	void	UpdateToolbarState();						// Force update of state if items visible
	void	UpdatePopupState(CWnd* wnd)					// Update state of popup menu as it is shown
	{
		UpdatePopupControl(wnd);
	}

	bool	IsVisible() const
		{ return mShowIt; }
	unsigned long GetMinimumWidth() const;
	unsigned long GetActualWidth() const;

	void	AddItem(CWnd* wnd, const CToolbarItem::CToolbarItemInfo& details);
	void	AddButton(CIconButton* btn, const CToolbarItem::CToolbarItemInfo& details);

protected:
	struct SMessageCommandMap
	{
		MessageT msg;
		CommandT cmd;
	};

	typedef std::vector<CToolbar*> CToolbarList;

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

	struct SItemSpec
	{
		SItemSpec(CWnd* ctrl, const CToolbarItem::CToolbarItemInfo& details) :
			mWnd(ctrl), mDetails(details) {}

		CWnd*							mWnd;
		CToolbarItem::CToolbarItemInfo	mDetails;
	};
	typedef std::vector<SItemSpec> CItemArray;

	CItemArray mItemList;

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
	CWnd*			GetItem(CToolbarManager::EToolbarItem item) const;
	unsigned long	GetItemIndex(CWnd* item) const;

			void	RemoveButton(CWnd* pane);
			void	MoveButton(CWnd* pane);
			void	ResetButtons(bool use_default);

			void	ShowToolbar(bool show);

			void	SmallIcons(bool small_icons);
			void	ShowIcons(bool show);	
			void	ShowCaptions(bool show);	

			void	Reset();
			void	ResetLayout();
			void	PositionButton(CRect& btnrect);
			CSize	GetBtnSize(const CToolbarButton* tb = NULL) const;

	CWnd*			GetCommander() const;

	virtual void	DoUpdateToolbarState();							// Force update of state
			void	UpdateControlState(CWnd* ctrl, UINT cmd)
				{ UpdateControl(ctrl, cmd, true, false); }
			void	UpdateControlVisibility(CWnd* ctrl, UINT cmd)
				{ UpdateControl(ctrl, cmd, false, true); }
			void	UpdateControl(CWnd* ctrl, UINT cmd, bool enable, bool show);
			void	UpdatePopupControl(CWnd* ctrl);
			void	UpdateControlValue(CWnd* ctrl, const UINT* cmd);
			bool	UpdateRulesButtons();

protected:

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	unsigned long mDragIndex;

	//bool TrackMove(EventRecord* event, CToolbarTrackAttachment* tracker);
	void TrackStart(CWnd* pane);
	void TrackStop();
	void TrackMouseMove(CPoint point);
	void TrackHighlight();

};

#endif
