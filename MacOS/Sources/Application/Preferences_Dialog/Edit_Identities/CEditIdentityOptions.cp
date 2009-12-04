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


// Source for CEditIdentityOptions class

#include "CEditIdentityOptions.h"

#include "CAdminLock.h"
#include "CEditIdentities.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextFieldX.h"

#include <LBevelButton.h>
#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditIdentityOptions::CEditIdentityOptions()
{
}

// Constructor from stream
CEditIdentityOptions::CEditIdentityOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CEditIdentityOptions::~CEditIdentityOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentityOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mCopyToActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentityCopyToActive);
	mUseCopyTo = (LRadioButton*) FindPaneByID(paneid_EditIdentityUseCopyTo);
	mCopyToNone = (LRadioButton*) FindPaneByID(paneid_EditIdentityCopyToNone);
	mCopyToChoose = (LRadioButton*) FindPaneByID(paneid_EditIdentityCopyToChoose);
	mCopyTo = (CTextFieldX*) FindPaneByID(paneid_EditIdentityCopyTo);
	mCopyToPopup = (CMailboxPopup*) FindPaneByID(paneid_EditIdentityCopyToPopup);
	mCopyToPopup->SetCopyTo(false);
	mCopyToPopup->SetDefault();
	if (!CMulberryApp::sApp->LoadedPrefs())
		mCopyToPopup->Disable();
	mCopyReplied = (LCheckBox*) FindPaneByID(paneid_EditIdentityCopyReplied);
	mHeaderActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentityHeaderActive);
	mHeaderBtn = (LBevelButton*) FindPaneByID(paneid_EditIdentityHeader);
	mFooterActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentityFooterActive);
	mFooterBtn = (LBevelButton*) FindPaneByID(paneid_EditIdentityFooter);

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
	{
		mHeaderBtn->Disable();
		mHeaderActive->Disable();
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CEditIdentityOptionsBtns);
}

// Handle buttons
void CEditIdentityOptions::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_IdentityHeader:
			EditHeadFoot(&mHeader, true);
			break;

		case msg_IdentityFooter:
			EditHeadFoot(&mFooter, false);
			break;

		case msg_IdentityUseCopyTo:
			if (*(long*) ioParam)
			{
				mCopyTo->Enable();
				if (CMulberryApp::sApp->LoadedPrefs())
					mCopyToPopup->Enable();
			}
			break;

		case msg_IdentityCopyToNone:
			if (*(long*) ioParam)
			{
				mCopyTo->Disable();
				mCopyToPopup->Disable();
			}
			break;

		case msg_IdentityCopyToChoose:
			if (*(long*) ioParam)
			{
				mCopyTo->Disable();
				mCopyToPopup->Disable();
			}
			break;

		case msg_IdentityCopyToPopup:
		{
			cdstring mbox_name;
			if (mCopyToPopup->GetSelectedMboxName(mbox_name))
			{
				if (mbox_name.empty())
				{
					mCopyToNone->SetValue(1);
					mCopyTo->Disable();
					mCopyToPopup->Disable();
				}
				else if (mbox_name.c_str() == "\1")
				{
					mCopyToChoose->SetValue(1);
					mCopyTo->Disable();
					mCopyToPopup->Disable();
				}
				else
				{
					mCopyTo->SetText(mbox_name);
				}
			}
			break;
		}

		default:
			break;
	}
}

// Toggle display of IC
void CEditIdentityOptions::ToggleICDisplay(bool IC_on)
{
	if (IC_on)
	{
		mHeaderBtn->Disable();
		mFooterBtn->Disable();
	}
	else
	{
		mHeaderBtn->Enable();
		mFooterBtn->Enable();
	}
}

// Set prefs
void CEditIdentityOptions::SetData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	// Copy info
	mCopyTo->SetText(identity->GetCopyTo());
	mCopyToActive->SetValue(identity->UseCopyTo());
	mUseCopyTo->SetValue(!identity->GetCopyToNone() && !identity->GetCopyToChoose());
	mCopyToNone->SetValue(identity->GetCopyToNone());
	mCopyToChoose->SetValue(identity->GetCopyToChoose());
	mCopyReplied->SetValue(identity->GetCopyReplied());

	if (identity->GetCopyToNone() || identity->GetCopyToChoose())
	{
		mCopyTo->Disable();
		mCopyToPopup->Disable();
	}

	mHeaderActive->SetValue(identity->UseHeader());
	mHeader = identity->GetHeader();

	mFooterActive->SetValue(identity->UseSignature());
	mFooter = identity->GetSignatureRaw(false);		// Get sig raw signature data
}

// Force update of prefs
void CEditIdentityOptions::UpdateData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetCopyTo(mCopyTo->GetText(), (mCopyToActive->GetValue() == 1));
	identity->SetCopyToNone(mCopyToNone->GetValue() == 1);
	identity->SetCopyToChoose(mCopyToChoose->GetValue() == 1);
	identity->SetCopyReplied(mCopyReplied->GetValue() == 1);

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		mHeader = cdstring::null_str;
	identity->SetHeader(mHeader, (mHeaderActive->GetValue() == 1));

	identity->SetSignature(mFooter, (mFooterActive->GetValue() == 1));
}

// Edit header or footer
void CEditIdentityOptions::EditHeadFoot(cdstring* text, bool header)
{
	// Allow file for signature only
	CPrefsEditHeadFoot::PoseDialog(header ? "Set X-Header Lines" : "Set Signature", *text, !header,
									CPreferences::sPrefs->spaces_per_tab.GetValue(), CPreferences::sPrefs->wrap_length.GetValue());
}
