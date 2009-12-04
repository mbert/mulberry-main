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


// Source for CAddressBookView class

#include "CAddressBookView.h"

#include "CAdbkManagerTable.h"
#include "CAdbkProtocol.h"
#include "CAddressBook.h"
#include "CAddressBookToolbar.h"
#include "CAddressBookWindow.h"
#include "CAddressComparators.h"
#include "CAddressTable.h"
#include "CAddressView.h"
#include "CErrorHandler.h"
#include "CGroupTable.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CTableViewWindow.h"
#include "CTaskClasses.h"
#include "C3PaneWindow.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif

#include <algorithm>

void CAddressBookView::ListenTo_Message(long msg, void* param)
{
	// Change the toolbar commander when a table is activated
	switch(msg)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	case CTableDrag::eBroadcast_Activate:
	case CHierarchyTableDrag::eBroadcast_Activate:
#else
	case CTable::eBroadcast_Activate:
#endif
		if (param == GetAddressTable())
		{
			// Set the address table as the toolbar commander
			mToolbar->AddCommander(GetAddressTable());
			
			// Force selection change to ensure preview resets to the active table
			GetAddressTable()->SelectionChanged();
		}
		else
		{
			// Set the group table as the toolbar commander
			mToolbar->AddCommander(GetGroupTable());
			
			// Force selection change to ensure preview resets to the active table
			GetGroupTable()->SelectionChanged();
		}
		break;
	case CAdbkProtocol::eBroadcast_ClearList:
		{
			// Handle close of protocol
			CAdbkProtocol* proto = reinterpret_cast<CAdbkProtocol*>(param);
			DoProtocolClearList(proto);
		}
		break;
	case CINETProtocol::eBroadcast_Logoff:
		{
			// Handle close of protocol
			CINETProtocol* proto = reinterpret_cast<CINETProtocol*>(param);
			if (dynamic_cast<CAdbkProtocol*>(proto))
				DoProtocolLogoff(dynamic_cast<CAdbkProtocol*>(proto));
		}
		break;
	}
	
	// Always do inherited
	CTableView::ListenTo_Message(msg, param);
}		

// Return user action data
const CUserAction& CAddressBookView::GetPreviewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsPreview);
}

// Return user action data
const CUserAction& CAddressBookView::GetFullViewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView);
}

// Handle protocol clearing list of address books
void CAddressBookView::DoProtocolClearList(CAdbkProtocol* proto)
{
	// Just do logoff action for now
	DoProtocolLogoff(proto);
}

// Handle close of protocol
void CAddressBookView::DoProtocolLogoff(CAdbkProtocol* proto)
{
	// See if it matches current address book
	if (proto && (GetAddressBook()->GetProtocol() == proto))
	{
		// Special case 3-pane/1-pane
		if (Is3Pane())
			SetAddressBook(NULL);
		else
			// Just close this view to force window to close
			DoClose();
	}
}

CAddressBookWindow* CAddressBookView::GetAddressBookWindow(void) const
{
	return !Is3Pane() ? static_cast<CAddressBookWindow*>(GetOwningWindow()) : NULL;
}

// Check that close is allowed
bool CAddressBookView::TestClose()
{
	// Can always close remote
	return true;
}

// Close it
void CAddressBookView::DoClose()
{
	// Only meaningful if not already closed
	if (IsClosed())
		return;

	// Set status
	SetClosing();

	// Save state before wiping the address book as state needs address book
	// name for record in preferences
	SaveState();

	// Close its address book
	if (GetAddressBook() && GetAddressBook()->IsOpen())
	{
		// Close any view first to flush pending address/group changes
		if (GetPreview())
			GetPreview()->DoClose();
		
		// Now close the address book if this is the last view containing it
		GetAddressBook()->Close();
	}
	
	// Always NULL out address book as it will not be valid from this point on
	mAdbk = NULL;
	
	// Always clear the old address book out of any tables etc
	ResetAddressBook();

	// Set status
	SetClosed();
	
	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		CCloseAddressBookWindowTask* task = new CCloseAddressBookWindowTask(GetAddressBookWindow());
		task->Go();
	}
}

void CAddressBookView::SetUsePreview(bool use_view)
{
	// Only if different
	if (mUsePreview == use_view)
		return;

	// Clear preview pane if its being hidden
	if (mUsePreview && (GetPreview() != NULL))
	{
		GetPreview()->ClearAddress();
		GetPreview()->ClearGroup();
	}

	mUsePreview = use_view;
}

// Reset the table
void CAddressBookView::ResetTable(void)
{
	// reset table contents
	GetAddressTable()->ResetTable();
	GetGroupTable()->ResetTable();
}

// Clear the table
void CAddressBookView::ClearTable(void)
{
	// reset table contents
	GetAddressTable()->ClearTable();
	GetGroupTable()->ClearTable();
}

// Find the corresponding window using URL
CAddressBookView* CAddressBookView::FindView(const CAddressBook* adbk)
{
	if (!adbk)
		return NULL;

	// Does window already exist?
	cdmutexprotect<CAddressBookViewList>::lock _lock(sAddressBookViews);
	for(CAddressBookViewList::iterator iter = sAddressBookViews->begin(); iter != sAddressBookViews->end(); iter++)
	{
		// Compare with the one we want
		if ((*iter)->GetAddressBook() &&
			((*iter)->GetAddressBook() == adbk))
		{
			// Set return items if found
			return *iter;
		}
	}

	return NULL;
}

// Find the corresponding window using URL
unsigned long CAddressBookView::CountViews(const CAddressBook* adbk)
{
	if (!adbk)
		return 0;

	unsigned long ctr = 0;

	// Does window already exist?
	cdmutexprotect<CAddressBookViewList>::lock _lock(sAddressBookViews);
	for(CAddressBookViewList::iterator iter = sAddressBookViews->begin(); iter != sAddressBookViews->end(); iter++)
	{
		// Compare with the one we want
		if ((*iter)->GetAddressBook() && ((*iter)->GetAddressBook() == adbk))
			ctr++;
	}

	return ctr;
}

// Check for window
bool CAddressBookView::ViewExists(const CAddressBookView* wnd)
{
	cdmutexprotect<CAddressBookViewList>::lock _lock(sAddressBookViews);
	CAddressBookViewList::iterator found = std::find(sAddressBookViews->begin(), sAddressBookViews->end(), wnd);
	return found != sAddressBookViews->end();
}

void CAddressBookView::ViewAddressBook(CAddressBook* adbk)
{
}

void CAddressBookView::SetAddressBook(CAddressBook* adbk)
{
	// Only if different
	if (GetAddressBook() == adbk)
		return;

	// Close any existing address book
	if (GetAddressBook())
	{
		// Close any view first to flush pending address/group changes
		if (GetPreview())
			GetPreview()->DoClose();
		
		// Clear out the tables now
		GetAddressTable()->ClearTable();
		GetGroupTable()->ClearTable();
		
		// Force state to be saved (before null'ing out address book)
		SaveState();

		// Now close the address book
		// Reset the adbk value BEFORE doing the close to prevent an
		// an additional close via a task on the new adbk
		CAddressBook* old_adbk = GetAddressBook();
		mAdbk = NULL;

		old_adbk->Close();
		
		// Stop listening to previous address book's protocol
		old_adbk->GetProtocol()->Remove_Listener(this);
	}

	mAdbk = adbk;
	
	// Open the address book first
	if (adbk)
	{
		GetAddressBook()->Open();
			
		// Start listening to new address book's protocol
		GetAddressBook()->GetProtocol()->Add_Listener(this);
	}

	// Reset table items and initiate sort
	GetAddressTable()->SetAddressBook(adbk);
	GetGroupTable()->SetAddressBook(adbk);
	SetSortBy(eAddrColumnName);
	UpdateCaptions();

	SetOpen();

	// Enable disable it based on adbk
	if (adbk)
	{
		// Enable it
		FRAMEWORK_ENABLE_WINDOW(this);
	}
	else
	{
		// Disable it
		FRAMEWORK_DISABLE_WINDOW(this);
	}

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
	
	// Reset window state
	ResetState();

	// Force any preview to be empty
	//GetAddressTable()->PreviewAddress(true);
}

void CAddressBookView::ResetAddressBook()
{
	// Reset table items and initiate sort
	GetAddressTable()->SetAddressBook(mAdbk);
	GetGroupTable()->SetAddressBook(mAdbk);
	UpdateCaptions();

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}


ESortAddressBy CAddressBookView::MapColumnToSort(EAddrColumn col) const
{
	// Map column type to sort method
	switch (col)
	{
	case eAddrColumnName:
	default:
		return cSortAddressLastName;

	case eAddrColumnNameLastFirst:
		return cSortAddressFirstName;

	case eAddrColumnNickName:
		return cSortAddressNickName;
	
	case eAddrColumnEmail:
		return cSortAddressEmail;

	case eAddrColumnCompany:
		return cSortAddressCompany;
	}
}

EAddrColumn CAddressBookView::MapSortToColumn(ESortAddressBy sort) const
{
	// Map sort method to column type
	switch (sort)
	{
	case cSortAddressFirstName:
		return eAddrColumnNameLastFirst;

	case cSortAddressLastName:
	default:
		return eAddrColumnName;

	case cSortAddressNickName:
		return eAddrColumnNickName;
	
	case cSortAddressEmail:
	case cSortAddressEmailDomain:
		return eAddrColumnEmail;

	case cSortAddressCompany:
		return eAddrColumnCompany;
	}
}

// Force change of sort
void CAddressBookView::SetSortBy(int sort)
{
	// Must have valid address book
	CAddressBook* adbk = GetAddressBook();
	if (!adbk)
		return;

	// Change sort method
	switch (static_cast<EAddrColumn>(sort))
	{
	case eAddrColumnName:
		adbk->GetAddressList()->SetComparator(new CLastNameComparator);
		break;

	case eAddrColumnNameLastFirst:
		adbk->GetAddressList()->SetComparator(new CFirstNameComparator);
		break;

	case eAddrColumnNickName:
		adbk->GetAddressList()->SetComparator(new CNickNameComparator);
		break;
	
	case eAddrColumnEmail:
		adbk->GetAddressList()->SetComparator(new CEmailComparator);
		break;

	case eAddrColumnCompany:
		adbk->GetAddressList()->SetComparator(new CCompanyComparator);
		break;

	case eAddrColumnAddress:
	case eAddrColumnPhoneWork:
	case eAddrColumnPhoneHome:
	case eAddrColumnFax:
	case eAddrColumnURLs:
	case eAddrColumnNotes:
	default:
		adbk->GetAddressList()->SetComparator(new CFirstNameComparator);
		break;
	}
	mSortColumnType = static_cast<EAddrColumn>(sort);
	
	// Redisplay
	FRAMEWORK_REFRESH_WINDOW(this);
}

// Get sort type
int CAddressBookView::GetSortBy()
{
	// Get sort method
	return mSortColumnType;
}

// Synchronise address book
void CAddressBookView::OnSynchroniseAddressBook()
{
	try
	{
		mAdbk->Synchronise(false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Clear Disconnected address book
void CAddressBookView::OnClearDisconnectAddressBook()
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Adbk::ReallyClearDisconnect") == CErrorHandler::Ok)
	{
		try
		{
			mAdbk->ClearDisconnect();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}
