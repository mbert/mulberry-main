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
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressCapture::CPrefsAddressCapture()
{
}

// Constructor from stream
CPrefsAddressCapture::CPrefsAddressCapture(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressCapture::~CPrefsAddressCapture()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressCapture::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mCaptureAddressBook = (CTextFieldX*) FindPaneByID(paneid_ACCaptureAddressBook);
	mCapturePopup = (LPopupButton*) FindPaneByID(paneid_ACCapturePopup);
	mCaptureAllowEdit = (LCheckBox*) FindPaneByID(paneid_ACCaptureAllowEdit);
	mCaptureAllowChoice = (LCheckBox*) FindPaneByID(paneid_ACCaptureAllowChoice);
	mCaptureRead = (LCheckBox*) FindPaneByID(paneid_ACCaptureRead);
	mCaptureRespond = (LCheckBox*) FindPaneByID(paneid_ACCaptureRespond);
	mCaptureFrom = (LCheckBox*) FindPaneByID(paneid_ACCaptureFrom);
	mCaptureCc = (LCheckBox*) FindPaneByID(paneid_ACCaptureCc);
	mCaptureReplyTo = (LCheckBox*) FindPaneByID(paneid_ACCaptureReplyTo);
	mCaptureTo = (LCheckBox*) FindPaneByID(paneid_ACCaptureTo);

	InitAddressBookPopup();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAddressCaptureBtns);
}

// Handle buttons
void CPrefsAddressCapture::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ACCapturePopup:
		{
			cdstring temp = ::GetPopupMenuItemTextUTF8(mCapturePopup);
			mCaptureAddressBook->SetText(temp);
		}
		break;
	}
}

// Set prefs
void CPrefsAddressCapture::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info to into panel
	mCaptureAddressBook->SetText(copyPrefs->mCaptureAddressBook.GetValue());
	mCaptureAllowEdit->SetValue(copyPrefs->mCaptureAllowEdit.GetValue());
	mCaptureAllowChoice->SetValue(copyPrefs->mCaptureAllowChoice.GetValue());
	mCaptureRead->SetValue(copyPrefs->mCaptureRead.GetValue());
	mCaptureRespond->SetValue(copyPrefs->mCaptureRespond.GetValue());
	mCaptureFrom->SetValue(copyPrefs->mCaptureFrom.GetValue());
	mCaptureCc->SetValue(copyPrefs->mCaptureCc.GetValue());
	mCaptureReplyTo->SetValue(copyPrefs->mCaptureReplyTo.GetValue());
	mCaptureTo->SetValue(copyPrefs->mCaptureTo.GetValue());
}

// Force update of prefs
void CPrefsAddressCapture::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mCaptureAddressBook.SetValue(mCaptureAddressBook->GetText());
	copyPrefs->mCaptureAllowEdit.SetValue(mCaptureAllowEdit->GetValue()==1);
	copyPrefs->mCaptureAllowChoice.SetValue(mCaptureAllowChoice->GetValue()==1);
	copyPrefs->mCaptureRead.SetValue(mCaptureRead->GetValue()==1);
	copyPrefs->mCaptureRespond.SetValue(mCaptureRespond->GetValue()==1);
	copyPrefs->mCaptureFrom.SetValue(mCaptureFrom->GetValue()==1);
	copyPrefs->mCaptureCc.SetValue(mCaptureCc->GetValue()==1);
	copyPrefs->mCaptureReplyTo.SetValue(mCaptureReplyTo->GetValue()==1);
	copyPrefs->mCaptureTo.SetValue(mCaptureTo->GetValue()==1);
}

// Init the popup
void CPrefsAddressCapture::InitAddressBookPopup()
{
	// Remove any existing plugin items from main menu
	MenuHandle menuH = mCapturePopup->GetMacMenuH();
	short num_remove = ::CountMenuItems(menuH);
	for(short i = 0; i < num_remove; i++)
		::DeleteMenuItem(menuH, 1);

	// Must have address book manager at this point
	if (CAddressBookManager::sAddressBookManager)
	{
		// Get address book list using account names as opposed to urls
		cdstrvect names;
		CAddressBookManager::sAddressBookManager->GetCurrentAddressBookList(names, false);

		short index = 1;
		for(cdstrvect::const_iterator iter = names.begin(); iter != names.end(); iter++, index++)
		{
			// Empty name => separator for accounts
			if ((*iter).empty())
				::InsertMenuItem(menuH, "\p(-", index);
			else
				::AppendItemToMenu(menuH, index, *iter);
		}
	}

	// Force max/min update
	mCapturePopup->SetMenuMinMax();
}
