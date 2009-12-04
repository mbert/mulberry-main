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


// Source for CPrefsMailboxDisconnected class

#include "CPrefsMailboxDisconnected.h"

#include "CTabController.h"

#include "CPrefsMailboxOptions.h"
#include "CPrefsAccountLocal.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set up params for DDX
void CPrefsMailboxDisconnected::SetData(void* data)
{
	mTabs->SetData(data);
}

// Get params from DDX
bool CPrefsMailboxDisconnected::UpdateData(void* data)
{
	// Get values
	return mTabs->UpdateData(data);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay message handlers

void CPrefsMailboxDisconnected::OnCreate()
{
// begin JXLayout1

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 360,225);
    assert( mTabs != NULL );

// end JXLayout1

	// Create tab panels
	CTabPanel* card = new CPrefsMailboxOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Options");

	card = new CPrefsAccountLocal(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	static_cast<CPrefsAccountLocal*>(card)->SetDisconnected();
	mTabs->AppendCard(card, "Local");
}
