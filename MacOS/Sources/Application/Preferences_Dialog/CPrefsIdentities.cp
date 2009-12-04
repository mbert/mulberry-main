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


// Source for CPrefsIdentities class

#include "CPrefsIdentities.h"

#include "CPreferences.h"

#include "CBalloonDialog.h"
#include "CEditIdentities.h"
#include "CErrorHandler.h"
#include "CMulberryCommon.h"
#include "CPreferencesDialog.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextTable.h"

#include <LCheckBox.h>
#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsIdentities::CPrefsIdentities()
{
}

// Constructor from stream
CPrefsIdentities::CPrefsIdentities(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsIdentities::~CPrefsIdentities()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsIdentities::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	mIdentitiesList = (CTextTable*) FindPaneByID(paneid_IdentitiesList);
	mIdentitiesList->EnableDragAndDrop();
	mIdentitiesList->AddListener(this);
	mIdentitiesList->Add_Listener(this);
	mChangeBtn = (LPushButton*) FindPaneByID(paneid_ChangeIdentitiesBtn);
	mChangeBtn->Disable();
	mDuplicateBtn = (LPushButton*) FindPaneByID(paneid_DuplicateIdentitiesBtn);
	mDuplicateBtn->Disable();
	mDeleteBtn = (LPushButton*) FindPaneByID(paneid_DeleteIdentitiesBtn);
	mDeleteBtn->Disable();

	mContextTied = (LCheckBox*) FindPaneByID(paneid_ContextTied);
	mMsgTied = (LCheckBox*) FindPaneByID(paneid_MsgTied);
	mTiedMboxInherit = (LCheckBox*) FindPaneByID(paneid_TiedMboxInherit);

	mSmartAddressText = (CTextDisplay*) FindPaneByID(paneid_SmartText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsIdentitiesBtns);

}

// Handle buttons
void CPrefsIdentities::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AddIdentities:
			DoAddIdentities();
			break;

		case msg_ChangeIdentities:
			DoEditIdentities();
			break;

		case msg_DuplicateIdentities:
			DoDuplicateIdentities();
			break;

		case msg_DeleteIdentities:
			DoDeleteIdentities();
			break;

		case msg_SelectIdentities:
			if (mIdentitiesList->IsSelectionValid())
			{
				mChangeBtn->Enable();
				mDuplicateBtn->Enable();
				mDeleteBtn->Enable();
			}
			else
			{
				mChangeBtn->Disable();
				mDuplicateBtn->Disable();
				mDeleteBtn->Disable();
			}
			break;

		default:
			break;
	}
}

void CPrefsIdentities::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTextTable::eBroadcast_Drag:
		{
			CTextTable::SBroadcastDrag* data = static_cast<CTextTable::SBroadcastDrag*>(param);
			if (data->mTable == mIdentitiesList)
				// Do drag move action
				DoMoveIdentities(data->mDragged, data->mDropped);
		}
		break;
	default:;
	}
}

// Toggle display of IC items
void CPrefsIdentities::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsIdentities::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	InitIdentitiesList();

	mContextTied->SetValue(copyPrefs->mContextTied.GetValue());
	mMsgTied->SetValue(copyPrefs->mMsgTied.GetValue());
	mTiedMboxInherit->SetValue(copyPrefs->mTiedMboxInherit.GetValue());

	cdstring smart_addrs;
	for(cdstrvect::const_iterator iter = copyPrefs->mSmartAddressList.GetValue().begin(); iter != copyPrefs->mSmartAddressList.GetValue().end(); iter++)
	{
		smart_addrs += *iter;
		smart_addrs += os_endl;
	}
	mSmartAddressText->SetText(smart_addrs);
}

// Force update of prefs
void CPrefsIdentities::UpdatePrefs(void)
{
	mCopyPrefs->mContextTied.SetValue(mContextTied->GetValue());
	mCopyPrefs->mMsgTied.SetValue(mMsgTied->GetValue());
	mCopyPrefs->mTiedMboxInherit.SetValue(mTiedMboxInherit->GetValue());

	// Only copy text if dirty
	if (mSmartAddressText->IsDirty())
	{
		// Copy handle to text with null terminator
		cdstring txt;
		mSmartAddressText->GetText(txt);

		char* s = ::strtok(txt.c_str_mod(), CR);
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, CR);
		}
		mCopyPrefs->mSmartAddressList.SetValue(accumulate);
	}
}

// Initialise the list
void CPrefsIdentities::InitIdentitiesList(void)
{
	cdstrvect items;
	for(CIdentityList::const_iterator iter = mCopyPrefs->mIdentities.GetValue().begin();
			iter != mCopyPrefs->mIdentities.GetValue().end(); iter++)
		items.push_back((*iter).GetIdentity());

	mIdentitiesList->SetContents(items);
}

// Add an identity
void CPrefsIdentities::DoAddIdentities(void)
{
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsEditIdentity, prefs_dlog);
		CIdentity new_identity;
		((CEditIdentities*) theHandler.GetDialog())->SetIdentity(mCopyPrefs, &new_identity);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy changed identity
				((CEditIdentities*) theHandler.GetDialog())->GetIdentity(&new_identity);

				// Check validity
				if (new_identity.GetIdentity().empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
					continue;
				}

				// Add to prefs and popup
				mCopyPrefs->mIdentities.Value().push_back(new_identity);
				mCopyPrefs->mIdentities.SetDirty();

				// Insert item
				const cdstring& txt = new_identity.GetIdentity();
				mIdentitiesList->InsertRows(1, mCopyPrefs->mIdentities.GetValue().size(), txt, txt.length() + 1, true);
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	prefs_dlog->SwitchTarget(prefs_dlog);
}

// Edit identities
void CPrefsIdentities::DoEditIdentities(void)
{
	STableCell aCell(0, 0);

	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

	// Delete entire selection in reverse to stay in sync
	while(mIdentitiesList->GetNextSelectedCell(aCell))
	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsEditIdentity, prefs_dlog);
		CIdentity& old_identity = mCopyPrefs->mIdentities.Value()[aCell.row - 1];
		((CEditIdentities*) theHandler.GetDialog())->SetIdentity(mCopyPrefs, &old_identity);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy changed identity - init to old one to copy over fields not edited by the dialog
				CIdentity edit_identity(old_identity);
				((CEditIdentities*) theHandler.GetDialog())->GetIdentity(&edit_identity);

				// Check validity
				if (edit_identity.GetIdentity().empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
					continue;
				}

				// Carry on
				mCopyPrefs->ReplaceIdentity(old_identity, edit_identity);

				const cdstring& txt = old_identity.GetIdentity();
				mIdentitiesList->SetCellData(aCell, txt, txt.length() + 1);
				mIdentitiesList->RefreshRow(aCell.row);
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	prefs_dlog->SwitchTarget(prefs_dlog);
}

// Duplicate identities
void CPrefsIdentities::DoDuplicateIdentities(void)
{
	STableCell aCell(0, 0);

	// Duplicate entire selection
	while(mIdentitiesList->GetNextSelectedCell(aCell))
	{
		// Duplicate list item
		CIdentity new_identity(mCopyPrefs->mIdentities.GetValue()[aCell.row - 1]);
		new_identity.SetIdentity(new_identity.GetIdentity() + " copy");

		// Add to prefs and popup
		mCopyPrefs->mIdentities.Value().push_back(new_identity);
		mCopyPrefs->mIdentities.SetDirty();

		// Insert item
		const cdstring& txt = new_identity.GetIdentity();
		mIdentitiesList->InsertRows(1, mCopyPrefs->mIdentities.GetValue().size(), txt, txt.length() + 1, true);
	}
}

// Delete identities
void CPrefsIdentities::DoDeleteIdentities(void)
{
	STableCell aCell(0, 0);

	// Delete entire selection in reverse to stay in sync
	while(mIdentitiesList->GetPreviousSelectedCell(aCell))
	{
		// Delete list item
		mIdentitiesList->FocusDraw();
		mIdentitiesList->RemoveRows(1, aCell.row, true);

		// Remove item from prefs
		mCopyPrefs->DeleteIdentity(&(*(mCopyPrefs->mIdentities.Value().begin() + (aCell.row - 1))));
	}

	// Force button update
	ListenToMessage(msg_SelectIdentities, 0L);
}

// Move identities
void CPrefsIdentities::DoMoveIdentities(const ulvector& from, unsigned long to)
{
	// Move items in prefs
	mCopyPrefs->MoveIdentity(from, to);
	
	// Redo list display
	InitIdentitiesList();
}
