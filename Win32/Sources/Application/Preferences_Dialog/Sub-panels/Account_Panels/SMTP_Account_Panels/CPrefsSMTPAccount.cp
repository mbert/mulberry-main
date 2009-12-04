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


// CPrefsSMTPAccount.cpp : implementation file
//

#include "CPrefsSMTPAccount.h"

#include "CPreferences.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsSMTPOptions.h"
#include "CPrefsLocalOptions.h"
#include "CSMTPAccount.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSMTPAccount dialog


CPrefsSMTPAccount::CPrefsSMTPAccount()
	: CPrefsAccountPanel(CPrefsSMTPAccount::IDD, IDC_PREFS_ACCOUNT_SMTP_TABS)
{
	//{{AFX_DATA_INIT(CPrefsSMTPAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsSMTPAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsSMTPAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_SMTP_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSMTPAccount message handlers

void CPrefsSMTPAccount::InitTabs()
{
	// Create tab panels
	mTabs.AddPanel(new CPrefsAccountAuth);
	mTabs.AddPanel(new CPrefsSMTPOptions);
}

// Set prefs
void CPrefsSMTPAccount::SetContent(void* data)
{
	// Check whether first account in prefs
	CSMTPAccount* acct = (CSMTPAccount*) data;
	if (mCopyPrefs->mSMTPAccounts.GetValue().front() == acct)
	{
		if (mTabs.GetItemCount() != 3)
			mTabs.AddPanel(new CPrefsLocalOptions);
	}
	else
	{
		if (mTabs.GetItemCount() == 3)
		{
			// reset current tab if its the one to be deleted
			if (mTabs.GetCurSel() == 2)
			{
				mTabs.SetCurSel(0);
				mTabs.SetPanel(0);
			}
			
			// Delete the one we don't want
			mTabs.RemovePanel(2);
		}
	}
	CPrefsAccountPanel::SetContent(data);
}

// Force update of data
bool CPrefsSMTPAccount::UpdateContent(void* data)
{
	CPrefsAccountPanel::UpdateContent(data);

	// Check whether first account in prefs and propogate local path to all
	CSMTPAccount* acct = (CSMTPAccount*) data;
	if (mCopyPrefs->mSMTPAccounts.GetValue().front() == acct)
	{
		for(CSMTPAccountList::iterator iter = mCopyPrefs->mSMTPAccounts.Value().begin() + 1;
				iter != mCopyPrefs->mSMTPAccounts.Value().end(); iter++)
			(*iter)->GetCWD() = acct->GetCWD();
	}
	
	return true;
}