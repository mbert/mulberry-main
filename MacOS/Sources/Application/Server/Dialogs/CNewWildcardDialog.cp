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


// Source for CNewWildcardDialog class

#include "CNewWildcardDialog.h"

#include "CBalloonDialog.h"
#include "CINETCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

enum
{
	menu_Account_All = 1,
	menu_Account_Divider,
	menu_Account_First
};

enum
{
	menu_Criteria_StartsWith = 1,
	menu_Criteria_EndsWith,
	menu_Criteria_Contains,
	menu_Criteria_Is,
	menu_Criteria_FullHierarchy,
	menu_Criteria_TopHierarchy
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewWildcardDialog::CNewWildcardDialog()
{
}

// Constructor from stream
CNewWildcardDialog::CNewWildcardDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CNewWildcardDialog::~CNewWildcardDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewWildcardDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Make text edit field active
	mAccount = (LPopupButton*) FindPaneByID(paneid_NewWildcardAccount);
	mCriteria = (LPopupButton*) FindPaneByID(paneid_NewWildcardCriteria);
	mCriteria->AddListener(this);
	mCriteria->SetValue(menu_Criteria_FullHierarchy);
	mText = (CTextFieldX*) FindPaneByID(paneid_NewWildcardText);

	InitAccountMenu();

	SetLatentSub(mText);
}

// Called during idle
void CNewWildcardDialog::GetDetails(cdstring& change)
{
	// Add account name
	if (mAccount->GetValue() == menu_Account_All)
		change += cWILDCARD;
	else
	{
		change += ::GetPopupMenuItemTextUTF8(mAccount);
	}
	change += cMailAccountSeparator;

	// Get criteria and name
	short criteria = mCriteria->GetValue();
	cdstring copyStr = mText->GetText();

	// Set leading criteria value
	switch(criteria)
	{
	case menu_Criteria_EndsWith:
	case menu_Criteria_Contains:
		change += cWILDCARD;
		break;
	case menu_Criteria_StartsWith:
	case menu_Criteria_Is:
	case menu_Criteria_FullHierarchy:
	case menu_Criteria_TopHierarchy:
		break;
	}

	change += copyStr;

	// Set trailing criteria value
	switch(criteria)
	{
	case menu_Criteria_StartsWith:
	case menu_Criteria_Contains:
		change += cWILDCARD;
		break;
	case menu_Criteria_EndsWith:
	case menu_Criteria_Is:
		break;
	case menu_Criteria_FullHierarchy:
	case menu_Criteria_TopHierarchy:
		{
			short acct_num = mAccount->GetValue() - menu_Account_First;
			if (acct_num < 0)
				acct_num = 0;
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

			// Append dir delim if not already
			if (dir_delim && change.length() && (change.c_str()[change.length() - 1] != dir_delim) && copyStr.length())
				change += dir_delim;

			// Append appropriate wildcard
			if (criteria == menu_Criteria_FullHierarchy)
				change += cWILDCARD;
			else
				change += cWILDCARD_NODIR;
		}
		break;
	}

}

// Called during idle
void CNewWildcardDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccount->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= menu_Account_First; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = menu_Account_First;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());

	// Force max/min update
	mAccount->SetMenuMinMax();
}

bool CNewWildcardDialog::PoseDialog(cdstring& name)
{
	bool result = false;

	CBalloonDialog	dlog(paneid_NewWildcardDialog, CMulberryApp::sApp);
	if (dlog.DoModal() == msg_OK)
	{
		((CNewWildcardDialog*) dlog.GetDialog())->GetDetails(name);
		result = true;
	}

	return result;
}
