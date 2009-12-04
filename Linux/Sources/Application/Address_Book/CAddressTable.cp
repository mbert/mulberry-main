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
#include "CClipboard.h"
#include "CCommands.h"
#include "CEditAddressDialog.h"
#include "CGroup.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CUserAction.h"

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXImage.h>
#include <JXTextMenu.h>
#include <JPainter.h>
#include <JTableSelection.h>
#include <jASCIIConstants.h>
#include <jXKeysym.h>

#include <memory>

/////////////////////////////////////////////////////////////////////////////
// CAddressTable

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTable::CAddressTable(JXScrollbarSet* scrollbarSet, 
								 JXContainer* enclosure,
								 const HSizingOption hSizing, 
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

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

void CAddressTable::OnCreate()
{
	// Find table view in super view chain
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CAddressBookView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CAddressBookView*>(dynamic_cast<const CAddressBookView*>(parent));

	CTableDragAndDrop::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sFlavorAddrList);
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAddrList);
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	SetReadOnly(false);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);
	SetAllowMove(false);

	// Context menu
	CreateContextMenu(CMainMenu::eContextAdbkTable);
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
bool CAddressTable::IsDirty()
{
	return mDirty;
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
void CAddressTable::ResetTable()
{
	// Prevent selection changes
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
	Refresh();
}

// Reset the table from the address list
void CAddressTable::ClearTable()
{
	// Prevent selection changes
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

void CAddressTable::OnNewMessage(void)
{
	CreateNewLetter(GetDisplay()->GetLatestKeyModifiers().meta());
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

// Create a new address
void CAddressTable::CreateNewEntry()
{
	// Let DialogHandler process events
	std::auto_ptr<CAdbkAddress> new_addr(new CAdbkAddress);
	if (CEditAddressDialog::PoseDialog(new_addr.get()))
	{
		// Only add if some text available
		if (!new_addr->IsEmpty())
		{
			// Initiate action - copies new_addr
			mTableView->GetUndoer()->PostAction(new CAddressTableNewAction(this, new_addr.release(), IsDirty()));
		}
	}
}

// Edit selected entries
void CAddressTable::DoEditEntry()
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
}

// Edit specified address
bool CAddressTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	// Copy original address
	std::auto_ptr<CAdbkAddress> copy(new CAdbkAddress(*theAddr));
	if (CEditAddressDialog::PoseDialog(copy.get()))
	{
		// Add info to action
		mEditAction->AddEdit(theAddr, copy.release());
		done_edit = true;
	}
	
	return done_edit;
}

// Delete selected entries
void CAddressTable::DoDeleteEntry()
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

	bool added = (addrs->size() != 0);

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
		SetDirty(false);
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

	SetDirty(false);
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
	
	SetDirty(false);
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
void CAddressTable::OnUpdateEditPaste(JXTextMenu* menu, JIndex item)
{
	// Check for text in clipboard
	menu->EnableItem(item);
}

// Command handlers
void CAddressTable::OnEditCut()
{
	// Copy first
	OnEditCopy();
	
	// Delete selection
	DoDeleteEntry();
}

void CAddressTable::OnEditCopy()
{
	cdstring txt;

	// Add all selected address to text
	DoToSelection1((DoToSelection1PP) &CAddressTable::AddAddressText, &txt);

	// Now copy to scrap
	CClipboard::CopyToSecondaryClipboard(GetDisplay(), txt);
}

// Add address as text
bool CAddressTable::AddAddressText(TableIndexT row, cdstring* txt)
{
	// Get selected address
	CAddress* addr = mAdbk->GetAddressList()->at(row - 1);
	
	cdstring full_addr = addr->GetFullAddress();

	// Add CRLF before if multi-copy
	if (txt->length())
		*txt += os_endl;
	*txt += full_addr;
		
	return true;
}		

// Add address as text
bool CAddressTable::AddAdbkAddressText(TableIndexT row, cdstring* txt)
{
	// Get selected address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	std::auto_ptr<const char> temp(mAdbk->ExportAddress(addr));

	*txt += temp.get();

	return true;
}

void CAddressTable::OnEditPaste()
{
	cdstring txt;
	CClipboard::GetSecondaryClipboard(GetDisplay(), txt);
	if (txt.length())
		DropData(GetDisplay()->GetSelectionManager()->GetTextXAtom(), (unsigned char*) txt.c_str(), txt.length());
}

void CAddressTable::ActionDeleted()
{
	if (mEditAction)
		mEditAction = NULL;
}

#pragma mark ____________________________________Keyboard/Mouse

// Clicked item
void CAddressTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	CTableDragAndDrop::LClickCell(inCell, modifiers);
	DoSingleClick(inCell.row, CKeyModifiers(modifiers));
}

// Double-clicked item
void CAddressTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, CKeyModifiers(modifiers));
}

// Handle key down
bool CAddressTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	switch(key)
	{
	case kJDeleteKey:
	case kJForwardDeleteKey:
		DoDeleteEntry();
		return true;

	case '\t':
		//mTableView->FocusGroup();
		//mTableView->SetFocus();
		return true;

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if (!modifiers.control() && !modifiers.meta())
		{
			if ((key < 256) && isalnum(key))
			{
				// See if key hit within standard double click time
				timeval tv;
				::gettimeofday(&tv, NULL);
				unsigned long new_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
				if ((new_time - mLastTyping) / 1000 >= 1)
				{

					// Outside double-click time so new selection
					mLastChars[0] = key;
					mLastChars[1] = 0;
				}
				else
				{
					// Inside double-click time so add char to selection
					short pos = ::strlen(mLastChars);
					if (pos < 31)
					{
						mLastChars[pos++] = key;
						mLastChars[pos] = 0;
					}
				}
				mLastTyping = new_time;

				DoKeySelection();
				return true;
			}
		}
		// Did not handle key
		return CTableDragAndDrop::HandleChar(key, modifiers);
	}
}

// Select from key press
void CAddressTable::DoKeySelection()
{
	// Prevent selection changes while doing multi-select
	StDeferSelectionChanged _defer(this);

	CAddress *temp = NULL;
	
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

bool CAddressTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnNewMessage();
		return true;

	case CCommand::eEditCut:
		OnEditCut();
		return true;
	case CCommand::eEditCopy:
		OnEditCopy();
		return true;
	case CCommand::eEditPaste:
		OnEditPaste();
		return true;

	case CCommand::eAddressEdit:
	case CCommand::eToolbarAddressBookEditBtn:
		DoEditEntry();
		return true;

	case CCommand::eAddressDelete:
	case CCommand::eToolbarAddressBookDeleteBtn:
		DoDeleteEntry();
		return true;

	default:;
	}

	return CTableDragAndDrop::ObeyCommand(cmd, menu);
}

void CAddressTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eEditCut:
	case CCommand::eEditCopy:
	case CCommand::eAddressEdit:
	case CCommand::eToolbarAddressBookEditBtn:
	case CCommand::eAddressDelete:
	case CCommand::eToolbarAddressBookDeleteBtn:
		OnUpdateSelection(cmdui);
		return;

	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eEditPaste:
		OnUpdateAlways(cmdui);
		return;
	default:;
	}

	CTableDragAndDrop::UpdateCommand(cmd, cmdui);
}

// Draw the items
void CAddressTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Draw selection
	DrawCellSelection(pDC, inCell);

	unsigned long bkgnd = GetCellBackground(inCell);

	// Get its address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(inCell.row - 1));

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	cdstring theTxt;
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
	JCoordinate left = inLocalRect.left;
	if (inCell.col == 1)
	{
		JXImage* icon = CIconLoader::GetIcon(IDI_ADDRESS_FLAG, this, 16, bkgnd);
		pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		left += 18;
	}

	::DrawClippedStringUTF8(pDC, theTxt, JPoint(left, inLocalRect.top), inLocalRect, eDrawString_Left);
}

#pragma mark ____________________________________Drag&Drop

// Send data to target
bool CAddressTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	if  (type == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold deleted items
		std::auto_ptr<CAddressList> addrs(new CAddressList);
		addrs->set_delete_data(false);

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, addrs.get());
		seldata->SetData(type, reinterpret_cast<unsigned char*>(addrs.release()), sizeof(CAddressList*));
		rendered = true;
	}

	else if ((type == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (type == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		cdstring txt;

		// Add all selected address to text
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddAddressText, &txt);

		// Allocate global memory for the text if not already
		unsigned long dataLength = txt.length() + 1;
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			::memcpy(data, txt.c_str(), dataLength);
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data into table
bool CAddressTable::DropData(Atom theFlavor, unsigned char* drag_data, unsigned long data_size)
{
	bool added = false;
	CAddressList* new_addrs = new CAddressList;

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			CEnvelope* theEnv = (*iter)->GetEnvelope();

			// Add From
			if (theEnv->GetFrom()->size())
			{
				CAddress* theAddr = theEnv->GetFrom()->front();

				// Check duplicate
				if (!mAdbk->GetAddressList()->IsDuplicate(theAddr))
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

				// Check duplicate
				if (!mAdbk->GetAddressList()->IsDuplicate(theAddr))
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

			// Add all Ccs
			for(CAddressList::const_iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
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
	}
	
	else if (theFlavor == CMulberryApp::sFlavorAddrList)
	{
		CAddressList* addrs = reinterpret_cast<CAddressList*>(drag_data);
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			// Check duplicate
			if(!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				added = new_addrs->InsertUniqueItem(copy);
				if (!added)
					delete copy;
			}
		}
	}
	
	else if (theFlavor == CMulberryApp::sFlavorGrpList)
	{
		CGroupList* grps = reinterpret_cast<CGroupList*>(drag_data);
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		{
			// Copy all addresses and add to list
			for(unsigned long i = 0; i < (*iter)->GetAddressList().size(); i++)
			{
				// Check duplicate
				CAdbkAddress* copy = new CAdbkAddress(CAddress((*iter)->GetAddressList().at(i)));
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
		char* line_start = (char*) drag_data;
		char* line_end = (char*) drag_data;
		char* data_end = (char*) (drag_data + data_size);
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

	else if ((theFlavor == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (theFlavor == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		// Parse text into list
		CAddressList list(reinterpret_cast<const char*>(drag_data),
							::strlen(reinterpret_cast<const char*>(drag_data)));
		
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

	if (added)
	{
		// Create new action - action owns list - and post
		mTableView->GetUndoer()->PostAction(new CAddressTableNewAction(this, new_addrs, IsDirty()));
	}
	else
		delete new_addrs;
	
	return added;
}
