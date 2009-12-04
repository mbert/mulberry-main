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


// Source for CAddressTable class


#include "CAddressTable.h"

#include "CAddressBook.h"
#include "CAddressBookView.h"
#include "CAddressTableAction.h"
#include "CAddressView.h"
#include "CGroup.h"
#include "CDrawUtils.h"
#include "CEditAddressDialog.h"
#include "CLocalAddressBook.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CUserAction.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressTable

IMPLEMENT_DYNCREATE(CAddressTable, CTableDragAndDrop)

BEGIN_MESSAGE_MAP(CAddressTable, CTableDragAndDrop)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnNewMessage)
	ON_COMMAND(IDC_ADDRESS_NEWMSG, OnNewMessage)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_CLEAR, DeleteSelection)
	ON_COMMAND(IDC_ActionDeleted, OnCmdActionDeleted)

	ON_UPDATE_COMMAND_UI(IDC_ADDRESS_EDIT, OnUpdateSelection)
	ON_COMMAND(IDC_ADDRESS_EDIT, OnEditAddress)

	ON_UPDATE_COMMAND_UI(IDC_ADDRESS_DELETE, OnUpdateSelection)
	ON_COMMAND(IDC_ADDRESS_DELETE, OnDeleteAddress)

	ON_WM_CREATE()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnNewMessage)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnNewMessageOption)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADDRESSBOOKEDITBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARADDRESSBOOKEDITBTN, OnEditAddress)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADDRESSBOOKDELETEBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARADDRESSBOOKDELETEBTN, OnDeleteAddress)
	
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTable::CAddressTable() : CTableDragAndDrop()
{
	mTableGeometry = new CTableRowGeometry(this, 16, 16);
	mTableSelector = new CTableRowSelector(this);

	mTableView = NULL;
	mAdbk = NULL;
	mEditAction = NULL;

	mLastTyping = 0UL;
	mLastChars[0] = 0;

	SetRowSelect(true);

}

// Default destructor
CAddressTable::~CAddressTable()
{
	if (mEditAction)
	{
		mTableView->GetUndoer()->PostAction(NULL);
		mEditAction = NULL;
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CAddressTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableDragAndDrop::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sFlavorAddrList);
	AddDragFlavor(CF_UNICODETEXT);

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAddrList);
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDropFlavor(CF_UNICODETEXT);

	SetReadOnly(false);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);
	SetAllowMove(false);

	// Find table view in super view chain
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CAddressBookView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CAddressBookView*>(parent);

	return 0;
}

// Keep titles in sync
void CAddressTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Do scroll of main table first to avoid double-refresh
	CTableDragAndDrop::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);

	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);
}

// Set dirty flag
void  CAddressTable::SetDirty(bool dirty)
{
	mDirty = dirty;
}

// Is it dirty
bool  CAddressTable::IsDirty(void)
{
	return dynamic_cast<CLocalAddressBook*>(mAdbk) && mDirty;
}

void CAddressTable::DoSelectionChanged()
{
	CTableDragAndDrop::DoSelectionChanged();
	
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSelection())
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFullView();
}

// Handle single click
void CAddressTable::DoSingleClick(TableIndexT row, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSingleClick() &&
		(preview.GetSingleClickModifiers() == mods))
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSingleClick() &&
		(fullview.GetSingleClickModifiers() == mods))
		DoFullView();
}

// Handle double click
void CAddressTable::DoDoubleClick(TableIndexT row, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetDoubleClick() &&
		(preview.GetDoubleClickModifiers() == mods))
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetDoubleClick() &&
		(fullview.GetDoubleClickModifiers() == mods))
		DoFullView();
}

void CAddressTable::DoPreview()
{
	// Ignore if no preview pane
	if (!mTableView->GetPreview())
		return;

	CAdbkAddress* addr = NULL;

	// Only bother if single selection
	if (IsSingleSelection())
		{
		// Add each selected address
		CAddressList addrs;
		addrs.set_delete_data(false);
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, &addrs);

		// Set to single group only
		if (addrs.size() == 1)
			addr = static_cast<CAdbkAddress*>(addrs.front());
	}

	// Only do preview if there has been a change
	if (addr != mTableView->GetPreview()->GetAddress())
	{
		// Always clear out message preview immediately, otherwise
		// do preview at idle time to prevent re-entrant network calls
		if (addr)
		{
			// Give it to preview (if its NULL the preview will be cleared)
			CAddressPreviewTask* task = new CAddressPreviewTask(mTableView->GetPreview(), mAdbk, addr);
			task->Go();
		}
		else
			// do immediate update of preview
			mTableView->GetPreview()->SetAddress(mAdbk, addr);
	}
}

// Just edit the item
void CAddressTable::DoFullView()
{
	DoEditEntry();
}

// Set the entry list
void CAddressTable::SetAddressBook(CAddressBook* adbk)
{
	// Save address list
	mAdbk = adbk;

	// Calculate number of rows for first time
	ResetTable();
}

// Reset the table from the address list
void CAddressTable::ResetTable(void)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	unsigned long num_addrs = (mAdbk ? mAdbk->GetAddressList()->size() : 0);
	TableIndexT old_rows = mRows;

	if (old_rows > num_addrs)
		RemoveRows(old_rows - num_addrs, 1, false);
	else if (old_rows < num_addrs)
		InsertRows(num_addrs - old_rows, 1, NULL, 0, false);

	// Previous selection no longer valid
	UnselectAllCells();

	mTableView->UpdateCaptions();
	RedrawWindow();
}

// Reset the table from the address list
void CAddressTable::ClearTable()
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Remove all rows
	RemoveAllRows(true);
	
	// Update captions to empty
	FRAMEWORK_REFRESH_WINDOW(this)
}

int CAddressTable::GetSortBy() const
{
	return mTableView->GetSortBy();
}

void CAddressTable::OnNewAddress(void)
{
	CreateNewEntry();
}

void CAddressTable::OnEditAddress(void)
{
	DoEditEntry();
}

void CAddressTable::OnDeleteAddress(void)
{
	DoDeleteEntry();
}

// Create new letter from selection
void CAddressTable::CreateNewLetter(bool option_key)
{
	// Create list of selected addresses
	CAddressList list;

	// Copy selection into list for active list only
	AddSelectionToList(&list);

	CReplyChooseDialog::ProcessChoice(&list,
					!(CPreferences::sPrefs->mOptionKeyAddressDialog.GetValue() ^ option_key) && (list.size() > 0));
}

void CAddressTable::OnNewMessage(void)
{
	CreateNewLetter(false);
}

void CAddressTable::OnNewMessageOption(void)
{
	CreateNewLetter(true);
}

// Create a new address
void CAddressTable::CreateNewEntry(void)
{
	auto_ptr<CAdbkAddress> new_addr(new CAdbkAddress);
	if (CEditAddressDialog::PoseDialog(new_addr.get()))
	{
		// Only add if some text available
		if (!new_addr->IsEmpty())
		{
			// Initiate action - copies new_addr
			mTableView->GetUndoer()->PostAction(new CAddressTableNewAction(this, new_addr.release(), IsDirty()));
		}
	}

	SetFocus();
}

// Edit selected entries
void CAddressTable::DoEditEntry(void)
{
	// Create new edit action ready to store edits
	CAddressTableEditAction* oldEditAction = mEditAction;
	mEditAction = new CAddressTableEditAction(this, IsDirty());

	// Do edit for each selection
	if (TestSelectionOr((TestSelectionPP) &CAddressTable::EditEntry))

		// Initiate action - will commit previous
		mTableView->GetUndoer()->PostAction(mEditAction);

	else
	{
		// Delete attempted action and restore previous
		delete mEditAction;
		mEditAction = oldEditAction;
	}
	
	SetFocus();
}

// Edit specified address
bool CAddressTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row -1));

	// Copy original address
	auto_ptr<CAdbkAddress> copy(new CAdbkAddress(*theAddr));
	if (CEditAddressDialog::PoseDialog(copy.get()))
	{
		// Add info to action
		mEditAction->AddEdit(theAddr, copy.release());
		done_edit = true;
	}

	return done_edit;
}

// Delete selected entries
void CAddressTable::DoDeleteEntry(void)
{
	// Create list to hold deleted items
	CAddressList* addrs = new CAddressList();

	// Add each selected address
	DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, addrs);

	// Initiate delete action if any selected
	if (addrs->size() > 0)
		mTableView->GetUndoer()->PostAction(new CAddressTableDeleteAction(this, addrs, IsDirty()));
	else
		delete addrs;
}

// Add from list - not unique
void CAddressTable::AddAddressesFromList(CAddressList* addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	bool added = false;

	// Add all addresses
	mAdbk->AddAddress(addrs, true);

	if (added)
	{
		bool got_one = false;

		// Reset to include new item
		UnselectAllCells();
		ResetTable();

		// Select added addresses
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			long index = mAdbk->GetAddressList()->FetchIndexOf(*iter);
			if (index)
				SelectRow(index);
		}
	
		// Bring first item into view
		ShowFirstSelection();
		SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
	}
}

// Change from list
void CAddressTable::ChangeAddressesFromList(CAddressList* old_addrs, CAddressList* new_addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->UpdateAddress(old_addrs, new_addrs);

	// Reset only after all changes have happened
	UnselectAllCells();
	ResetTable();

	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
}

// Delete from list
void CAddressTable::RemoveAddressesFromList(CAddressList* addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->RemoveAddress(addrs);

	// Reset only after all changes have happened
	UnselectAllCells();
	ResetTable();
	
	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
}

// Add selected addresses to list
void CAddressTable::AddSelectionToList(CAddressList* list)
{
	// Add all selected address to drag
	DoToSelection1((DoToSelection1PP) &CAddressTable::CopyEntryToList, list);
}

// Copy address to list
bool CAddressTable::CopyEntryToList(TableIndexT row, CAddressList* list)
{
	// Get selected address
	CAddress* theAddr = mAdbk->GetAddressList()->at(row - 1);
	
	// Copy address
	CAddress* copy = new CAddress(*theAddr);

	// Save in selected list if not duplicate
	if (list->InsertUniqueItem(copy))
		return true;
	else
	{
		delete copy;
		return false;
	}
}		

// Add address to list
bool CAddressTable::AddEntryToList(TableIndexT row, CAddressList* list)
{
	// Get selected address
	CAddress* theAddr = mAdbk->GetAddressList()->at(row - 1);
	
	// Save in selected list if not duplicate
	list->push_back(theAddr);
	return true;
}		

// Common updaters
void CAddressTable::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT));
}

// Command handlers
void CAddressTable::OnEditCut(void)
{
	// Copy first
	OnEditCopy();
	
	// Delete selection
	DeleteSelection();
}

void CAddressTable::OnEditCopy(void)
{
	// Now copy to scrap
	if (!::OpenClipboard(*this))
		return;
	::EmptyClipboard();

	{
		// Add all selected address to text
		cdstring txt;
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddAddressText, &txt);
		cdustring utf16(txt);

		// Allocate global memory for the text
		HANDLE hglb = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
		if (hglb)
		{
			// Copy to global after lock
			unichar_t* lptstr = (unichar_t*) ::GlobalLock(hglb);
			::unistrcpy(lptstr, utf16);
			::GlobalUnlock(hglb);
			
			::SetClipboardData(CF_UNICODETEXT, hglb);
		}
	}
	{
		// Add all selected address to text
		cdstring txt;
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddAdbkAddressText, &txt);

		// Allocate global memory for the text
		HANDLE hglb = ::GlobalAlloc(GMEM_DDESHARE, txt.length() + 1);
		if (hglb)
		{
			// Copy to global after lock
			char* lptstr = (char*) ::GlobalLock(hglb);
			::strcpy(lptstr, txt);
			::GlobalUnlock(hglb);
			
			::SetClipboardData(CMulberryApp::sFlavorAdbkAddrList, hglb);
		}
	}
	
	::CloseClipboard();
}

// Add address as text
bool CAddressTable::AddAddressText(TableIndexT row, cdstring* txt)
{
	// Get selected address
	CAddress* addr = mAdbk->GetAddressList()->at(row - 1);
	
	cdstring full_addr = addr->GetFullAddress();

	// Add CRLF before if multi-copy
	if (txt->length())
		*txt += "\r\n";
	*txt += full_addr;
		
	return true;
}		

// Add address as text
bool CAddressTable::AddAdbkAddressText(TableIndexT row, cdstring* txt)
{
	// Get selected address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	auto_ptr<const char> temp(mAdbk->ExportAddress(addr));

	*txt += temp.get();

	return true;
}

void CAddressTable::OnEditPaste(void)
{
	if (!::OpenClipboard(*this))
		return;
	HANDLE hglb = ::GetClipboardData(CMulberryApp::sFlavorAdbkAddrList);
	if (hglb)
	{
		char* lptstr = (char*) ::GlobalLock(hglb);
		DropData(CMulberryApp::sFlavorAdbkAddrList, lptstr, ::strlen(lptstr));
		::GlobalUnlock(hglb);
	}
	else if ((hglb = ::GetClipboardData(CF_UNICODETEXT)) != NULL)
	{
		unichar_t* lptstr = (unichar_t*) ::GlobalLock(hglb);
		DropData(CF_UNICODETEXT, (char*)lptstr, ::unistrlen(lptstr) * sizeof(unichar_t));
		::GlobalUnlock(hglb);
	}
	::CloseClipboard();
}

void CAddressTable::OnCmdActionDeleted(void)
{
	if (mEditAction)
		mEditAction = NULL;
}

#pragma mark ____________________________________Keyboard/Mouse

// Clicked item
void CAddressTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	CTableDragAndDrop::LClickCell(inCell, nFlags);
	DoSingleClick(inCell.row, CKeyModifiers(0));
}

// Double-clicked item
void CAddressTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoEditEntry();
}

// Handle key down
bool CAddressTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(0);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == nChar) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == nChar) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	switch(nChar)
	{
	case VK_BACK:
	case VK_DELETE:
		DoDeleteEntry();
		break;

	case VK_TAB:
		mTableView->FocusGroup();
		mTableView->SetFocus();
		break;

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if ((::GetKeyState(VK_CONTROL) >= 0) &&
			(::GetKeyState(VK_MENU) >= 0))
		{
			char hit_key = (nChar & 0x000000FF);
			if (::isalnum(hit_key))
			{
				// See if key hit within standard double click time
				unsigned long new_time = clock()/CLOCKS_PER_SEC;
				if (new_time - mLastTyping >= 1)
				{
				
					// Outside double-click time so new selection
					mLastChars[0] = hit_key;
					mLastChars[1] = 0;
				}
				else
				{
					// Inside double-click time so add char to selection
					short pos = ::strlen(mLastChars);
					if (pos < 31)
					{
						mLastChars[pos++] = hit_key;
						mLastChars[pos] = 0;
					}
				}
				mLastTyping = new_time;
				
				DoKeySelection();
				return true;
			}
		}
		// Did not handle key
		return CTableDragAndDrop::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

// Select from key press
void CAddressTable::DoKeySelection(void)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	CAddress* temp = NULL;
	
	// Create new address for typed chars
	switch (static_cast<EAddrColumn>(GetSortBy()))
	{
	case eAddrColumnName:
	case eAddrColumnNameLastFirst:
		temp = new CAdbkAddress(NULL, NULL, mLastChars, NULL);
		break;

	case eAddrColumnNickName:
		temp = new CAdbkAddress(NULL, NULL, NULL, mLastChars);
		break;
	
	case eAddrColumnEmail:
		temp = new CAdbkAddress(NULL, mLastChars, NULL, NULL);
		break;

	case eAddrColumnCompany:
		temp = new CAdbkAddress(NULL, NULL, NULL, NULL, mLastChars);
		break;

	default:
		return;
	}
	
	// Get low bound insert pos
	long select = mAdbk->GetAddressList()->FetchInsertIndexOf(temp);
	
	// Clip to end of list
	if (select > mAdbk->GetAddressList()->size()) select--;

	STableCell aCell(select, 2);
	UnselectAllCells();
	FakeClickSelect(aCell, false);
	ScrollCellIntoFrame(aCell, true);

	delete temp;

}

// Draw the items
void CAddressTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	// Save text state in stack object
	cdstring		theTxt;
	StDCState		save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get its address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(inCell.row - 1));

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAddrColumnName:
	case eAddrColumnNameLastFirst:
		theTxt = addr->GetName();
		break;
	
	case eAddrColumnNickName:
		theTxt = addr->GetADL();
		break;
	
	case eAddrColumnEmail:
		theTxt = addr->GetMailAddress();
		break;
	
	case eAddrColumnCompany:
		theTxt = addr->GetCompany();
		break;
	
	case eAddrColumnAddress:
		theTxt = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
		break;
	
	case eAddrColumnPhoneWork:
		theTxt = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
		break;
	
	case eAddrColumnPhoneHome:
		theTxt = addr->GetPhone(CAdbkAddress::eHomePhoneType);
		break;
	
	case eAddrColumnFax:
		theTxt = addr->GetPhone(CAdbkAddress::eFaxType);
		break;
	
	case eAddrColumnURLs:
		theTxt = addr->GetURL();
		break;
	
	case eAddrColumnNotes:
		theTxt = addr->GetNotes();
		break;
	
	default:
		break;
	}

	// Always draw icon in first column
	CRect drawRect(inLocalRect);
	if (inCell.col == 1)
	{
		CIconLoader::DrawIcon(pDC, drawRect.left, drawRect.top + mIconOrigin, IDI_ADDRESS_FLAG, 16);
		drawRect.left += 18;
	}
	else
		drawRect.left += 4;

	// Draw the string
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(drawRect.left, drawRect.top + mTextOrigin), drawRect, eDrawString_Left);
}

#pragma mark ____________________________________Drag&Drop

// Send data to target
BOOL CAddressTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold deleted items
		CAddressList addrs;

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, &addrs);
		
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, addrs.size() * sizeof(CAddress*) + sizeof(int));
		if (*phGlobal)
		{
			// Copy to global after lock
			CAddress** pAddr = (CAddress**) ::GlobalLock(*phGlobal);
			*((int*) pAddr) = addrs.size();
			pAddr += sizeof(int);
			for(CAddressList::iterator iter = addrs.begin(); iter != addrs.end(); iter++)
				*pAddr++ = *iter;
			//::memcpy(pAddr, addrs.begin(), addrs.size() * sizeof(CAddress*));
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
		
		// Do not delete originals
		addrs.clear_without_delete();
	}

	else if (lpFormatEtc->cfFormat == CF_UNICODETEXT)
	{
		// Add all selected address to text
		cdstring txt;
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddAddressText, &txt);

		// Convert to utf16
		cdustring utf16(txt);

		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
		if (*phGlobal)
		{
			// Copy to global after lock
			unichar_t* lptstr = (unichar_t*) ::GlobalLock(*phGlobal);
			::unistrcpy(lptstr, utf16);
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data into table
bool CAddressTable::DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size)
{
	bool added = false;
	CAddressList* new_addrs = new CAddressList;

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMessage* theMsg = ((CMessage**) drag_data)[i];
			CEnvelope* theEnv = theMsg->GetEnvelope();

			// Add From
			if (theEnv->GetFrom()->size())
			{
				CAddress* theAddr = theEnv->GetFrom()->front();

				// Check duplicate and smart address
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					CAdbkAddress* copy = new CAdbkAddress(*theAddr);
					if (new_addrs->InsertUniqueItem(copy))
						added = true;
					else
						delete copy;
				}
			}
			
			// Add Reply-To
			if (theEnv->GetReplyTo()->size())
			{
				CAddress* theAddr = theEnv->GetReplyTo()->front();

				// Check duplicate and smart address
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					CAdbkAddress* copy = new CAdbkAddress(*theAddr);
					if (new_addrs->InsertUniqueItem(copy))
						added = true;
					else
						delete copy;
				}
			}
			
			// Add all To
			for(CAddressList::const_iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				// Check duplicate and smart address
				if (!CPreferences::sPrefs->TestSmartAddress(**iter) &&
					!mAdbk->GetAddressList()->IsDuplicate(*iter))
				{
					// Add to list
					CAdbkAddress* copy = new CAdbkAddress(**iter);
					if (new_addrs->InsertUniqueItem(copy))
						added = true;
					else
						delete copy;
				}
			}

			// Add all Ccs
			for(CAddressList::const_iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				// Check duplicate and smart address
				if (!CPreferences::sPrefs->TestSmartAddress(**iter) &&
					!mAdbk->GetAddressList()->IsDuplicate(*iter))
				{
					// Add to list
					CAdbkAddress* copy = new CAdbkAddress(**iter);
					if (new_addrs->InsertUniqueItem(copy))
						added = true;
					else
						delete copy;
				}
			}
		}
	}
	
	else if (theFlavor == CMulberryApp::sFlavorAddrList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CAddress* theAddr = ((CAddress**) drag_data)[i];

			// Check duplicate and smart address
			if(!mAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(*theAddr);
				added = new_addrs->InsertUniqueItem(copy);
				if (!added)
					delete copy;
			}
		}
	}
	
	else if (theFlavor == CMulberryApp::sFlavorGrpList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CGroup* theGrp = ((CGroup**) drag_data)[i];

			// Copy all addresses and add to list
			for(short i = 0; i < theGrp->GetAddressList().size(); i++)
			{
				// Check duplicate and smart address
				CAdbkAddress* copy = new CAdbkAddress(CAddress(theGrp->GetAddressList().at(i)));
				if (!mAdbk->GetAddressList()->IsDuplicate(copy))
				{
					// Add to list
					if (new_addrs->InsertUniqueItem(copy))
						added = true;
					else
						delete copy;
				}
				else
					delete copy;
			}
		}
	}

	else if (theFlavor == CMulberryApp::sFlavorAdbkAddrList)
	{
		// Parse text into list
		CAddressList list;
		unsigned long size = 0;
		char* line_start = drag_data;
		char* line_end = drag_data;
		char* data_end = drag_data + data_size;
		while(line_end < data_end)
		{
			// Skip CR,LF
			while((line_end < data_end) && ((*line_end == '\r') || (*line_end == '\n')))
				line_end++;
			line_start = line_end;
			
			// Skip to line end or end of data
			while((line_end < data_end) && (*line_end != '\r') && (*line_end != '\n'))
				line_end++;
			
			// Import line data
			if (line_start < line_end)
			{
				CAdbkAddress* addr = NULL;
				cdstring temp(line_start, line_end - line_start);
				mAdbk->ImportAddress(temp.c_str_mod(), false, &addr, NULL);
				if (addr)
					list.push_back(addr);
			}
		}

		// Try to add these to table
		for(CAddressList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			// Check duplicate
			if (!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}
	}

	else if (theFlavor == CF_UNICODETEXT)
	{
		// Parse text into list
		cdustring utf16((unichar_t*) drag_data, ::unistrlen((unichar_t*) drag_data));
		cdstring utf8 = utf16.ToUTF8();
		CAddressList list(utf8, utf8.length());
		
		// Try to add these to table
		for(CAddressList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			// Check duplicate and smart address
			if (!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}
	}

	if (added)
	{
		// Create new action - action owns list - and post
		mTableView->GetUndoer()->PostAction(new CAddressTableNewAction(this, new_addrs, IsDirty()));
	}
	else
		delete new_addrs;
	
	return added;
}

