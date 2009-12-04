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


// Source for CPrefsRemoteAccount class

#include "CPrefsRemoteAccount.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsRemoteOptions.h"
#include "CPrefsRemoteSets.h"
#include "CTabController.h"

// Get details of sub-panes
void CPrefsRemoteAccount::InitTabs()
{
	// Create tab sub-panels
	CTabPanel* card = new CPrefsAccountAuth(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Authenticate");

	card = new CPrefsRemoteOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Options");

	card = new CPrefsRemoteSets(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Sets");
}
