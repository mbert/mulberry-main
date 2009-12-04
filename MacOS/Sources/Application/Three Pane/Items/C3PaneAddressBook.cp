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


// Source for C3PaneAddressBook class

#include "C3PaneAddressBook.h"

#include "CAddressBook.h"
#include "CAddressBookView.h"
#include "CResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneAddressBook::C3PaneAddressBook()
{
	mDoneInit = false;
	mUseSubstitute = false;
}

// Constructor from stream
C3PaneAddressBook::C3PaneAddressBook(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
	mDoneInit = false;
	mUseSubstitute = false;
}

// Default destructor
C3PaneAddressBook::~C3PaneAddressBook()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneAddressBook::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mAddressBookView = (CAddressBookView*) FindPaneByID(paneid_3PaneView);
	
	// Hide it until a mailbox is set
	Hide();
}

CBaseView* C3PaneAddressBook::GetBaseView() const
{
	return mAddressBookView;
}

void C3PaneAddressBook::Init()
{
	// Set the list of mailboxes currently open
	//if (!mDoneInit)
		//mAddressBookView->SetSubstituteItems(CPreferences::sPrefs->m3PaneOpenMailboxes.GetValue());
	mDoneInit = true;
}

void C3PaneAddressBook::GetOpenItems(cdstrvect& items) const
{
}

void C3PaneAddressBook::CloseOpenItems()
{
}

void C3PaneAddressBook::SetOpenItems(const cdstrvect& items)
{
}

void C3PaneAddressBook::DoClose()
{
	// Get the list of mailboxes currently open
	//cdstrvect opened;
	//mAddressBookView->GetSubstituteItems(opened);
	//CPreferences::sPrefs->m3PaneOpenMailboxes.SetValue(opened);

	// Close the server view
	mAddressBookView->DoClose();
}

bool C3PaneAddressBook::TestCloseAll()
{
	// Get the server view to do the test
	//return mAddressBookView->TestCloseAll();
	return false;
}

void C3PaneAddressBook::DoCloseAll()
{
	// Close the server view
	//mAddressBookView->DoCloseAll();
}

bool C3PaneAddressBook::TestCloseOne(unsigned long index)
{
	// Get the server view to do the test
	//return mAddressBookView->TestCloseOne(index);
	return false;
}

void C3PaneAddressBook::DoCloseOne(unsigned long index)
{
	// Close the server view
	//mAddressBookView->DoCloseOne(index);
}

bool C3PaneAddressBook::TestCloseOthers(unsigned long index)
{
	// Get the server view to do the test
	//return mAddressBookView->TestCloseOthers(index);
	return false;
}

void C3PaneAddressBook::DoCloseOthers(unsigned long index)
{
	// Close the server view
	//mAddressBookView->DoCloseOthers(index);
}

void C3PaneAddressBook::SetUseSubstitute(bool subs)
{
	//mUseSubstitute = subs;
	//mAddressBookView->SetUseSubstitute(subs);
}

bool C3PaneAddressBook::IsSubstituteLocked(unsigned long index) const
{
	// Test the view
	//return mAddressBookView->IsSubstituteLocked(index);
	return false;
}

bool C3PaneAddressBook::IsSubstituteDynamic(unsigned long index) const
{
	// Test the view
	//return mAddressBookView->IsSubstituteDynamic(index);
	return false;
}

bool C3PaneAddressBook::IsSpecified() const
{
	return mAddressBookView->GetAddressBook() != NULL;
}

cdstring C3PaneAddressBook::GetTitle() const
{
	CAddressBook* adbk = (mAddressBookView ? mAddressBookView->GetAddressBook() : NULL);
	return adbk ? adbk->GetAccountName() : cdstring::null_str;
}

ResIDT C3PaneAddressBook::GetIconID() const
{
	return ICNx_3PanePreviewAddressBook;
}
