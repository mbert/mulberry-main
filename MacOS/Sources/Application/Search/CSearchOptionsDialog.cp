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


// Source for CSearchOptionsDialog class

#include "CSearchOptionsDialog.h"

#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSearchOptionsDialog::CSearchOptionsDialog()
{
}

// Constructor from stream
CSearchOptionsDialog::CSearchOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CSearchOptionsDialog::~CSearchOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSearchOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	mMultiple = (LCheckBoxGroupBox*) FindPaneByID(paneid_SearchOptionsUseMultiple);
	mMaximum = (CTextFieldX*) FindPaneByID(paneid_SearchOptionsMaxMultiple);
	mLoadBalance = (LCheckBox*) FindPaneByID(paneid_SearchOptionsLoadBalance);
	mOpenFirst = (LCheckBox*) FindPaneByID(paneid_SearchOptionsOpenFirst);

	SetItems();

	SetLatentSub(mMaximum);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSearchOptionsDialog);
}

// Handle buttons
void CSearchOptionsDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_SearchOptionsUseMultiple:
		if (*((long*) ioParam))
		{
			mMaximum->Enable();
			mLoadBalance->Enable();
		}
		else
		{
			mMaximum->Disable();
			mLoadBalance->Disable();
		}
		break;
	}
}

// Set items from prefs
void CSearchOptionsDialog::SetItems()
{
	mMultiple->SetValue(CPreferences::sPrefs->mMultipleSearch.GetValue());
	mMaximum->SetNumber(CPreferences::sPrefs->mNumberMultipleSearch.GetValue());
	mLoadBalance->SetValue(CPreferences::sPrefs->mLoadBalanceSearch.GetValue());
	mOpenFirst->SetValue(CPreferences::sPrefs->mOpenFirstSearchResult.GetValue());
}

// Set prefs from items
void CSearchOptionsDialog::GetItems()
{
	CPreferences::sPrefs->mMultipleSearch.SetValue(mMultiple->GetValue());
	CPreferences::sPrefs->mNumberMultipleSearch.SetValue(mMaximum->GetNumber());
	CPreferences::sPrefs->mLoadBalanceSearch.SetValue(mLoadBalance->GetValue());
	CPreferences::sPrefs->mOpenFirstSearchResult.SetValue(mOpenFirst->GetValue());
}
