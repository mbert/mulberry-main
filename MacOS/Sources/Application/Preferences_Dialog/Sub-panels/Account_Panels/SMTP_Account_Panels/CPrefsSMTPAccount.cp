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
#include "CPrefsSubTab.h"
#include "CSMTPAccount.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSMTPAccount::CPrefsSMTPAccount()
{
}

// Constructor from stream
CPrefsSMTPAccount::CPrefsSMTPAccount(LStream *inStream)
		: CPrefsAccountPanel(inStream)
{
}

// Default destructor
CPrefsSMTPAccount::~CPrefsSMTPAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSMTPAccount::InitTabs()
{
	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAccountAuth);
	mTabs->AddPanel(paneid_PrefsSMTPOptions);
	mTabs->AddPanel(paneid_PrefsAccountLocal);
}

// Set prefs
void CPrefsSMTPAccount::SetData(void* data)
{
	// Check whether first account in prefs
	CSMTPAccount* acct = (CSMTPAccount*) data;
	mTabs->EnableTab(3, mCopyPrefs->mSMTPAccounts.GetValue().front() == acct);
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
