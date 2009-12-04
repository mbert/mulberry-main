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


// Source for C3PaneParentPanel class

#include "C3PaneParentPanel.h"

#include "CBaseView.h"
#include "C3PaneWindow.h"

BEGIN_MESSAGE_MAP(C3PaneParentPanel, C3PanePanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static

// Static members
C3PaneParentPanel* C3PaneParentPanel::sCurrentFocus = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneParentPanel::C3PaneParentPanel()
{
	mCurrent = NULL;
}

// Default destructor
C3PaneParentPanel::~C3PaneParentPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
int C3PaneParentPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PanePanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create multi-pane container and set alignment
	mView.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mView, CWndAlignment::eAlign_WidthHeight));

	return 0;
}

void C3PaneParentPanel::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CBaseView::eBroadcast_ViewActivate:
		// Mark as focussed
		sCurrentFocus = this;
		break;
	}
}		

void C3PaneParentPanel::MakeToolbars(CBaseView* view)
{
	// Create toolbars for a view we own
	view->MakeToolbars(m3PaneWindow->GetToolbarView());
}

bool C3PaneParentPanel::HasFocus() const
{
	return mCurrent ? mCurrent->HasFocus() : false;
}

void C3PaneParentPanel::Focus()
{
	if (mCurrent)
		mCurrent->Focus();
}
