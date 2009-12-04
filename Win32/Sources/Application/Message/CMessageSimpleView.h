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


// Header for CMessageSimpleView class

#ifndef __CMESSAGESIMPLEVIEW__MULBERRY__
#define __CMESSAGESIMPLEVIEW__MULBERRY__

#include "CBaseView.h"

#include "CCommanderProtect.h"

// Constants

// Messages

// Resources

// Classes
class CMessageToolbar;

class CMessageSimpleView : public CBaseView
{
public:
					CMessageSimpleView();
	virtual 		~CMessageSimpleView();

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual void	MakeToolbars(CToolbarView* parent);

	CMessageToolbar*	GetToolbar()
		{ return mToolbar; }

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	ResetState(bool force = false);		// Reset window state
	virtual void	SaveDefaultState();					// Save current state as default

protected:
	CMessageToolbar*		mToolbar;
	CCommanderProtect		mCmdProtect;					// Protect commands

	//{{AFX_MSG(CLetterHeaderView)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
