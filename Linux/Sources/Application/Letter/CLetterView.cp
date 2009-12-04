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
CLetterView::CLetterView(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: CBaseView(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CLetterView::~CLetterView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CLetterView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	mToolbar = new CLetterToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
	mToolbar->OnCreate();

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(mToolbar);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(mToolbar, static_cast<CLetterWindow*>(GetOwningWindow()), CToolbarView::eStdButtonsGroup);

	// Force toolbar to be active
	Broadcast_Message(eBroadcast_ViewActivate, this);
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
