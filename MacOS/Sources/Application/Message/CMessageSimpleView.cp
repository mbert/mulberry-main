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


// Source for CMessageSimpleView class

#include "CMessageSimpleView.h"

#include "CMessageToolbar.h"
#include "CToolbarView.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSimpleView::CMessageSimpleView()
{
}

// Constructor from stream
CMessageSimpleView::CMessageSimpleView(LStream *inStream)
		: CBaseView(inStream)
{
}

// Default destructor
CMessageSimpleView::~CMessageSimpleView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CMessageSimpleView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	mToolbar = static_cast<CMessageToolbar*>(UReanimator::CreateView(paneid_MessageToolbar1, parent, NULL));

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(mToolbar);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(mToolbar, GetOwningWindow(), CToolbarView::eStdButtonsGroup);
}

bool CMessageSimpleView::HasFocus() const
{
	return false;
}

void CMessageSimpleView::Focus()
{
}

// Reset window state
void CMessageSimpleView::ResetState(bool force)
{
}

// Save current state as default
void CMessageSimpleView::SaveDefaultState()
{
}