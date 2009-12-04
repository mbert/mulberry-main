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


// Source for CPrefsAddressCapture class

#include "CPrefsAddressCapture.h"

#include "CAddressBookManager.h"
#include "CPreferences.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressCapture::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Capture to:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,7, 70,20);
    assert( obj1 != NULL );

    mCaptureAddressBook =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,5, 220,20);
    assert( mCaptureAddressBook != NULL );

    mCapturePopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,7, 30,16);
    assert( mCapturePopup != NULL );

    mCaptureAllowEdit =
        new JXTextCheckbox("Display Edit Dialog on Capture", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 255,20);
    assert( mCaptureAllowEdit != NULL );

    mCaptureAllowChoice =
        new JXTextCheckbox("Allow Choice when Capturing Multiple", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 255,20);
    assert( mCaptureAllowChoice != NULL );

    mCaptureRead =
        new JXTextCheckbox("Capture when Reading a Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 255,20);
    assert( mCaptureRead != NULL );

    mCaptureRespond =
        new JXTextCheckbox("Capture when Responding to a Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 265,20);
    assert( mCaptureRespond != NULL );

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,119, 340,95);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("In a Message:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,109, 90,20);
    assert( obj3 != NULL );

    mCaptureFrom =
        new JXTextCheckbox("Capture From Addresses", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,11, 170,20);
    assert( mCaptureFrom != NULL );

    mCaptureCc =
        new JXTextCheckbox("Capture Cc Addresses", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,31, 170,20);
    assert( mCaptureCc != NULL );

    mCaptureReplyTo =
        new JXTextCheckbox("Capture Reply-To Addresses", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,51, 190,20);
    assert( mCaptureReplyTo != NULL );

    mCaptureTo =
        new JXTextCheckbox("Capture To Addresses", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,71, 190,20);
    assert( mCaptureTo != NULL );

// end JXLayout1

	// Start listening
	InitAddressBookPopup();
	ListenTo(mCapturePopup);
}

void CPrefsAddressCapture::Receive(JBroadcaster* sender, const Message& message)
{
	if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mCapturePopup)
    	{
			const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnChangeAdbk(is->GetIndex());
			return;
		} 
	}
}

// Set prefs
void CPrefsAddressCapture::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info to into panel
	mCaptureAddressBook->SetText(copyPrefs->mCaptureAddressBook.GetValue());
	mCaptureAllowEdit->SetState(JBoolean(copyPrefs->mCaptureAllowEdit.GetValue()));
	mCaptureAllowChoice->SetState(JBoolean(copyPrefs->mCaptureAllowChoice.GetValue()));
	mCaptureRead->SetState(JBoolean(copyPrefs->mCaptureRead.GetValue()));
	mCaptureRespond->SetState(JBoolean(copyPrefs->mCaptureRespond.GetValue()));
	mCaptureFrom->SetState(JBoolean(copyPrefs->mCaptureFrom.GetValue()));
	mCaptureCc->SetState(JBoolean(copyPrefs->mCaptureCc.GetValue()));
	mCaptureReplyTo->SetState(JBoolean(copyPrefs->mCaptureReplyTo.GetValue()));
	mCaptureTo->SetState(JBoolean(copyPrefs->mCaptureTo.GetValue()));
}

// Force update of prefs
bool CPrefsAddressCapture::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mCaptureAddressBook.SetValue(cdstring(mCaptureAddressBook->GetText()));
	copyPrefs->mCaptureAllowEdit.SetValue(mCaptureAllowEdit->IsChecked());
	copyPrefs->mCaptureAllowChoice.SetValue(mCaptureAllowChoice->IsChecked());
	copyPrefs->mCaptureRead.SetValue(mCaptureRead->IsChecked());
	copyPrefs->mCaptureRespond.SetValue(mCaptureRespond->IsChecked());
	copyPrefs->mCaptureFrom.SetValue(mCaptureFrom->IsChecked());
	copyPrefs->mCaptureCc.SetValue(mCaptureCc->IsChecked());
	copyPrefs->mCaptureReplyTo.SetValue(mCaptureReplyTo->IsChecked());
	copyPrefs->mCaptureTo.SetValue(mCaptureTo->IsChecked());

	return true;
}

// Init the popup
void CPrefsAddressCapture::InitAddressBookPopup()
{
	// Remove any existing items
	mCapturePopup->RemoveAllItems();

	// Must have address book manager at this point
	if (CAddressBookManager::sAddressBookManager)
	{
		// Get address book list using account names as opposed to urls
		cdstrvect names;
		CAddressBookManager::sAddressBookManager->GetCurrentAddressBookList(names, false);

		cdstring menu_items;
		for(cdstrvect::const_iterator iter = names.begin(); iter != names.end(); iter++)
		{
			// Empty name => separator for accounts
			if ((*iter).length() != 0)
			{
				if (menu_items.length() != 0)
					menu_items += " |";
				menu_items += *iter;
				menu_items += " %r";
			}
			else
				menu_items += " %l";
		}

		mCapturePopup->SetMenuItems(menu_items);

		// Force max/min update
		mCapturePopup->SetUpdateAction(JXMenu::kDisableNone);
	}
}

// Change move to
void CPrefsAddressCapture::OnChangeAdbk(JIndex nID)
{
	cdstring adbk_name(mCapturePopup->GetItemText(nID));
	mCaptureAddressBook->SetText(adbk_name);
}
