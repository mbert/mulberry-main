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


// Header for CBaseView class

#ifndef __CBASEVIEW__MULBERRY__
#define __CBASEVIEW__MULBERRY__

#include "CBroadcaster.h"
#include "CCommander.h"
#include "CListener.h"
#include "CWindowStatus.h"
#include "CWndAligner.h"

// Constants

// Classes
class CToolbarView;

class CBaseView : public CWnd, public CWindowStatus,
					public CBroadcaster, public CListener,
					public CWndAligner, public CCommander
{
protected:
	CWnd*				mOwnerWindow;
	bool				mIs3Pane;

public:
	enum
	{
		eBroadcast_ViewChanged = 'BVch',
		eBroadcast_ViewActivate = 'BVac',
		eBroadcast_ViewDeactivate = 'BVda',
		eBroadcast_ViewSelectionChanged = 'BVsc'
	};

					CBaseView();
	virtual 		~CBaseView();

	virtual bool	TestClose();
	virtual void	DoClose();

	CWnd*		GetOwningWindow() const
		{ return mOwnerWindow; }
	bool			Is3Pane() const
		{ return mIs3Pane; }

	virtual void	MakeToolbars(CToolbarView* parent) = 0;

	virtual bool	HasFocus() const = 0;
	virtual void	Focus() = 0;

	virtual void	RefreshToolbar() const;

			bool	IsVisible() const
		{ return IsWindowVisible(); }

public:
	virtual void	ResetState(bool force = false) = 0;			// Reset window state
	virtual void	SaveState() {}							// Save current state in prefs
	virtual void	SaveDefaultState() = 0;					// Save current state as default

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif
