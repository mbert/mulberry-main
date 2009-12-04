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


// Source for CNewSearchDialog class

#include "CNewSearchDialog.h"

#include "CBalloonDialog.h"
#include "CINETCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

enum
{
	menu_Criteria_StartsWith = 1,
	menu_Criteria_EndsWith,
	menu_Criteria_Contains,
	menu_Criteria_Is,
	menu_Criteria_SubHierarchy,
	menu_Criteria_FullHierarchy
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewSearchDialog::CNewSearchDialog()
{
}

// Constructor from stream
CNewSearchDialog::CNewSearchDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CNewSearchDialog::~CNewSearchDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewSearchDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Make text edit field active
	mAccount = (LPopupButton*) FindPaneByID(paneid_NewSearchAccount);
	mCriteria = (LPopupButton*) FindPaneByID(paneid_NewSearchCriteria);
	mCriteria->SetValue(menu_Criteria_SubHierarchy);
	mCriteria->AddListener(this);
	mText = (CTextFieldX*) FindPaneByID(paneid_NewSearchText);
	mFullHierarchy = (LRadioButton*) FindPaneByID(paneid_NewSearchFullHierarchy);
	mTopHierarchy = (LRadioButton*) FindPaneByID(paneid_NewSearchTopHierarchy);
	mTopHierarchy->SetValue(1);

	InitAccountMenu();

	SetLatentSub(mText);
}

// Handle buttons
void CNewSearchDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_NewSearchCriteria:
			switch(*(long*) ioParam)
			{
				case menu_Criteria_StartsWith:
				case menu_Criteria_EndsWith:
				case menu_Criteria_Contains:
				case menu_Criteria_Is:
					mFullHierarchy->Disable();
					mTopHierarchy->Disable();
					mText->Enable();
					break;
				case menu_Criteria_SubHierarchy:
					mFullHierarchy->Enable();
					mTopHierarchy->Enable();
					mText->Enable();
					break;
				case menu_Criteria_FullHierarchy:
					mFullHierarchy->Enable();
					mTopHierarchy->Enable();
					mText->Disable();
					break;
			}
			break;

		default:
			break;
	}
}

void CNewSearchDialog::SetDetails(bool single, unsigned long index)
{
	mSingle = single;
	if (single)
	{
		// Disable all menu items except for relevant one
		MenuHandle menuH = mAccount->GetMacMenuH();
		for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		{
			if (i != index)
				::DisableItem(menuH, i);
		}
	}

	mAccount->SetValue(index ? index : 1);
}

// Called during idle
void CNewSearchDialog::GetDetails(cdstring& change, unsigned long& index)
{
	// Get account index
	index = mAccount->GetValue();

	// Get criteria and name
	short criteria = mCriteria->GetValue();
	cdstring copyStr = mText->GetText();
	bool full = mFullHierarchy->GetValue();

	// Set leading criteria value
	switch(criteria)
	{
	case menu_Criteria_EndsWith:
	case menu_Criteria_Contains:
		change += cWILDCARD;
		break;
	case menu_Criteria_StartsWith:
	case menu_Criteria_Is:
	case menu_Criteria_SubHierarchy:
	case menu_Criteria_FullHierarchy:
		break;
	}

	// Only if not full
	if (criteria != menu_Criteria_FullHierarchy)
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
	case menu_Criteria_SubHierarchy:
		{
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(index - 1)->GetDirDelim();
			// Append dir delim if not already
			if (dir_delim && change.length() && (change.c_str()[change.length() - 1] != dir_delim))
				change += dir_delim;
		}
		// Append full wildcard
		if (full)
			change += cWILDCARD;
		break;

	case menu_Criteria_FullHierarchy:
		// Append appropriate wildcard
		if (full)
			change += cWILDCARD;
		else
			change += cWILDCARD_NODIR;
		break;
	}

}

// Called during idle
void CNewSearchDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccount->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());

	// Force max/min update
	mAccount->SetMenuMinMax();
}

bool CNewSearchDialog::PoseDialog(bool single, unsigned long& index, cdstring& wd)
{
	bool result = false;

	CBalloonDialog	dlog(paneid_NewSearchDialog, CMulberryApp::sApp);
	((CNewSearchDialog*) dlog.GetDialog())->SetDetails(single, index);

	if (dlog.DoModal() == msg_OK)
	{
		((CNewSearchDialog*) dlog.GetDialog())->GetDetails(wd, index);
		result = true;
	}

	return result;
}
