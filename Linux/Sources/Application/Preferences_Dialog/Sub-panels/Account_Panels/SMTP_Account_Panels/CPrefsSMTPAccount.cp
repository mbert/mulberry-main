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


// Source for CPrefsSMTPAccount class

#include "CPrefsSMTPAccount.h"

#include "CPreferences.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsSMTPOptions.h"
#include "CPrefsAccountLocal.h"
#include "CSMTPAccount.h"
#include "CTabController.h"

// Get details of sub-panes
void CPrefsSMTPAccount::InitTabs()
{
	// Create tab sub-panels
	CTabPanel* card = new CPrefsAccountAuth(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Authenticate");

	card = new CPrefsSMTPOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Options");

	card = new CPrefsAccountLocal(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 235);
	mTabs->AppendCard(card, "Local");
}

// Set prefs
void CPrefsSMTPAccount::SetData(void* data)
{
	// Check whether first account in prefs
	CSMTPAccount* acct = (CSMTPAccount*) data;
	if (mCopyPrefs->mSMTPAccounts.GetValue().front() == acct)
		mTabs->ActivateCard(3);
	else
	{
		// Can change local only in first SMTP account
		mTabs->SelectItem(1);
		mTabs->DeactivateCard(3);
	}
	CPrefsAccountPanel::SetData(data);
}

// Force update of data
void CPrefsSMTPAccount::UpdateData(void* data)
{
	CPrefsAccountPanel::UpdateData(data);

	// Check whether first account in prefs and propogate local path to all
	CSMTPAccount* acct = (CSMTPAccount*) data;
	if (mCopyPrefs->mSMTPAccounts.GetValue().front() == acct)
	{
		for(CSMTPAccountList::iterator iter = mCopyPrefs->mSMTPAccounts.Value().begin() + 1;
				iter != mCopyPrefs->mSMTPAccounts.Value().end(); iter++)
			(*iter)->GetCWD() = acct->GetCWD();
	}
}
