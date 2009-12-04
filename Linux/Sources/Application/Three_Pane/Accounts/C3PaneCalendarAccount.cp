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


// Source for C3PaneCalendarAccount class

#include "C3PaneCalendarAccount.h"

#include "CCalendarStoreView.h"

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneCalendarAccount::C3PaneCalendarAccount(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: C3PaneViewPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
C3PaneCalendarAccount::~C3PaneCalendarAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneCalendarAccount::OnCreate()
{
	// Do inherited
	C3PaneViewPanel::OnCreate();

	// Get sub-panes
	mCalendarStoreView = new CCalendarStoreView(this, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
	mCalendarStoreView->OnCreate();
	mCalendarStoreView->FitToEnclosure(kTrue, kTrue);

	mCalendarStoreView->ResetState();
	mCalendarStoreView->ResetTable();
}

CBaseView* C3PaneCalendarAccount::GetBaseView() const
{
	return mCalendarStoreView;
}
