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


// Source for CPrefsMailAccount class

#include "CPrefsMailAccount.h"

#include "CMailAccount.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsAccountIMAP.h"
#include "CPrefsAccountExtras.h"
#include "CTabController.h"

// Get details of sub-panes
void CPrefsMailAccount::InitTabs()
{
	// Create tab sub-panels
	CTabPanel* card = new CPrefsAccountAuth(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Authentication");

	card = new CPrefsAccountIMAP(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Options");

	card = new CPrefsAccountExtras(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Extras");
}
