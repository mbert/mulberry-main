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

#include "CEditIdentities.h"
#include "CErrorHandler.h"
#include "CMulberryCommon.h"
#include "CPreferencesDialog.h"
#include "CTableScrollbarSet.h"
#include "CTextDisplay.h"
#include "CTextTable.h"

#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CPrefsIdentities::CPrefsIdentities(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

void CPrefsIdentities::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Identities Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXUpRect* obj2 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 370,25);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Identities", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,3, 60,20);
    assert( obj3 != NULL );

    CScrollbarSet* sbs =
        new CScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,55, 370,115);
    assert( sbs != NULL );

    mAddBtn =
        new JXTextButton("Add...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,180, 70,20);
    assert( mAddBtn != NULL );

    mChangeBtn =
        new JXTextButton("Change...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,180, 70,20);
    assert( mChangeBtn != NULL );

    mDuplicateBtn =
        new JXTextButton("Duplicate", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,180, 70,20);
    assert( mDuplicateBtn != NULL );

    mDeleteBtn =
        new JXTextButton("Delete", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 280,180, 70,20);
    assert( mDeleteBtn != NULL );

    mContextTied =
        new JXTextCheckbox("Use Tied Identities with New Drafts", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,210, 225,20);
    assert( mContextTied != NULL );

    mMsgTied =
        new JXTextCheckbox("Choose Identity from Original Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,230, 255,20);
    assert( mMsgTied != NULL );

    mTiedMboxInherit =
        new JXTextCheckbox("Mailboxes Inherit Tied Identities", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,250, 255,20);
    assert( mTiedMboxInherit != NULL );

    mSmartAddressText =
        new CTextDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,300, 370,90);
    assert( mSmartAddressText != NULL );

    JXUpRect* obj4 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,275, 370,25);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Outgoing Addresses to Count as Mine", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,3, 225,20);
    assert( obj5 != NULL );

// end JXLayout1

	// Make fields editable
	mSmartAddressText->SetReadOnly(false);

	mIdentitiesList = new CTextTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,0, 10, 10);
	mIdentitiesList->OnCreate();
	mIdentitiesList->SetSelectionMsg(true);
	mIdentitiesList->SetDoubleClickMsg(true);
	mIdentitiesList->EnableDragAndDrop();

	// Initial state
	mChangeBtn->Deactivate();
	mDuplicateBtn->Deactivate();
	mDeleteBtn->Deactivate();

	// Listen to certain items
	ListenTo(mIdentitiesList);
	mIdentitiesList->Add_Listener(this);
	ListenTo(mAddBtn);
	ListenTo(mChangeBtn);
	ListenTo(mDuplicateBtn);
	ListenTo(mDeleteBtn);
}

void CPrefsIdentities::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mAddBtn)
		{
			DoAddIdentities();
			return;
		}
		else if (sender == mChangeBtn)
		{
			DoEditIdentities();
			return;
		}
		else if (sender == mDuplicateBtn)
		{
			DoDuplicateIdentities();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			DoDeleteIdentities();
			return;
		}
	}
	else if (sender == mIdentitiesList)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			if (mIdentitiesList->IsSelectionValid())
			{
				mChangeBtn->Activate();
				mDuplicateBtn->Activate();
				mDeleteBtn->Activate();
			}
			else
			{
				mChangeBtn->Deactivate();
				mDuplicateBtn->Deactivate();
				mDeleteBtn->Deactivate();
			}
			return;
		}
		else if (message.Is(CTextTable::kLDblClickCell))
		{
			DoEditIdentities();
			return;
		}
	}
	
	CPrefsPanel::Receive(sender, message);
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

// Set prefs
void CPrefsIdentities::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	InitIdentitiesList();

	mContextTied->SetState(JBoolean(copyPrefs->mContextTied.GetValue()));
	mMsgTied->SetState(JBoolean(copyPrefs->mMsgTied.GetValue()));
	mTiedMboxInherit->SetState(JBoolean(copyPrefs->mTiedMboxInherit.GetValue()));

	cdstring smart_addrs;
	for(unsigned long i = 0; i < copyPrefs->mSmartAddressList.GetValue().size(); i++)
	{
		smart_addrs += copyPrefs->mSmartAddressList.GetValue()[i];
		smart_addrs += '\n';
	}
	mSmartAddressText->SetText(smart_addrs);
}

// Force update of prefs
void CPrefsIdentities::UpdatePrefs(CPreferences* prefs)
{
	prefs->mContextTied.SetValue(mContextTied->IsChecked());
	prefs->mMsgTied.SetValue(mMsgTied->IsChecked());
	prefs->mTiedMboxInherit.SetValue(mTiedMboxInherit->IsChecked());

	// Only copy text if dirty
#ifdef NOTYET
	if (mSmartAddressText->IsDirty())
#endif
	{
		// Copy handle to text with null terminator
		cdstring txt(mSmartAddressText->GetText());
		
		char* s = ::strtok(txt.c_str_mod(), "\n");
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);
	
			s = ::strtok(NULL, "\n");
		}
		prefs->mSmartAddressList.SetValue(accumulate);
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
	CIdentity new_identity;
	while(true)
	{
		if (CEditIdentities::PoseDialog(mCopyPrefs, &new_identity))
		{
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
			mIdentitiesList->AddItem(new_identity.GetIdentity());
			break;
		}
		else
			break;
	}
}

// Edit identities
void CPrefsIdentities::DoEditIdentities(void)
{
	// Edit each selected items
	STableCell aCell(0, 0);

	while(mIdentitiesList->GetNextSelectedCell(aCell))
	{
		// Create the dialog
		CIdentity& old_identity = mCopyPrefs->mIdentities.Value()[aCell.row - 1];
		CIdentity edit_identity(old_identity);
		if (CEditIdentities::PoseDialog(mCopyPrefs, &edit_identity))
		{
			// Check validity
			if (edit_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Carry on
			mCopyPrefs->ReplaceIdentity(old_identity, edit_identity);
			mIdentitiesList->SetItemText(aCell.row, edit_identity.GetIdentity());
		}
	}
}

// Duplicate identities
void CPrefsIdentities::DoDuplicateIdentities(void)
{
	ulvector selected;
	mIdentitiesList->GetSelectedRows(selected);
	for(ulvector::iterator iter = selected.begin(); iter != selected.end(); iter++)
	{
		// Create the dialog
		CIdentity new_identity(mCopyPrefs->mIdentities.GetValue()[*iter - 1]);
		new_identity.SetIdentity(new_identity.GetIdentity() + " copy");

		// Add to prefs and popup
		mCopyPrefs->mIdentities.Value().push_back(new_identity);
		mCopyPrefs->mIdentities.SetDirty();

		// Insert item
		mIdentitiesList->AddItem(new_identity.GetIdentity());
	}
}

// Delete identities
void CPrefsIdentities::DoDeleteIdentities(void)
{
	// Get selected items
	STableCell aCell(0, 0);
	while(mIdentitiesList->GetPreviousSelectedCell(aCell))
	{
		mIdentitiesList->RemoveRows(1, aCell.row, true);

		// Remove item from prefs
		mCopyPrefs->DeleteIdentity(&(*(mCopyPrefs->mIdentities.Value().begin() + (aCell.row - 1))));
	}
}

// Move identities
void CPrefsIdentities::DoMoveIdentities(const ulvector& from, unsigned long to)
{
	// Move items in prefs
	mCopyPrefs->MoveIdentity(from, to);
	
	// Redo list display
	InitIdentitiesList();
}
