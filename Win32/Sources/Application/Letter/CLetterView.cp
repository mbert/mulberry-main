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


// Source for CLetterView class

#include "CLetterView.h"

#include "CLetterToolbar.h"
#include "CLetterWindow.h"
#include "CToolbarView.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CLetterView::CLetterView()
{
}

// Default destructor
CLetterView::~CLetterView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

BEGIN_MESSAGE_MAP(CLetterView, CBaseView)
	//{{AFX_MSG_MAP(CCmdBackground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLetterView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	mToolbar = new CLetterToolbar;
	mToolbar->InitToolbar(false, parent);

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(mToolbar);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(mToolbar, static_cast<CLetterWindow*>(GetParent()), CToolbarView::eStdButtonsGroup);
}

bool CLetterView::HasFocus() const
{
	return false;
}

void CLetterView::Focus()
{
}

// Reset window state
void CLetterView::ResetState(bool force)
{
}

// Save current state as default
void CLetterView::SaveDefaultState()
{
}

BOOL CLetterView::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Pass message up to owning window
	if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CBaseView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
