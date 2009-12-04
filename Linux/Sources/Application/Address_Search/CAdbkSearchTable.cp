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


// Source for CAdbkSearchTable class


#include "CAdbkSearchTable.h"

#include "CAdbkSearchWindow.h"
#include "CAdbkServerPopup.h"
#include "CClipboard.h"
#include "CCommands.h"
#include "CEditAddressDialog.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CTextField.h"
#include "CTitleTable.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JTableSelection.h>
#include <jASCIIConstants.h>
#include <jXKeysym.h>
#include <JPainter.h>

#include "StPenState.h"

#include <memory>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTable::CAdbkSearchTable(JXScrollbarSet* scrollbarSet, 
									JXContainer* enclosure,
									const HSizingOption hSizing, 
									const VSizingOption vSizing,
									const JCoordinate x, const JCoordinate y,
									const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);

	mLastParent = 0;
}

// Default destructor
CAdbkSearchTable::~CAdbkSearchTable()
{
}

void CAdbkSearchTable::OnCreate()
{
	mWindow = dynamic_cast<CAdbkSearchWindow*>(GetWindow()->GetDirector());

	CHierarchyTableDrag::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sApp->sFlavorAddrList);
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	SetReadOnly(true);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);
	SetAllowMove(false);

	// Force update to existing reults
	ResetTable();

	// Context menu
	CreateContextMenu(CMainMenu::eContextAdbkSearch);
}

bool CAdbkSearchTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eAddressNewMessage:
		if (IsSelectionValid())
		{
			OnNewLetter();
			return true;
		}
		break;

	case CCommand::eEditCopy:
		OnEditCopy();
		return true;

	case CCommand::eEditDelete:
		OnClear();
		return true;

	default:;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

void CAdbkSearchTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eAddressNewMessage:
		if (IsSelectionValid())
		{
			OnUpdateAlways(cmdui);
			return;
		}
		break;

	case CCommand::eEditCopy:
	case CCommand::eEditDelete:
		OnUpdateSelection(cmdui);
		return;

	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

// Keep titles in sync
void CAdbkSearchTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mWindow->mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Double-clicked item
void CAdbkSearchTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoEditEntry();
}

// Handle key down
bool CAdbkSearchTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	case kJReturnKey:
		DoEditEntry();
		return true;
	case kJDeleteKey:
	case kJForwardDeleteKey:
		OnDeleteAddress();
		return true;
	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleChar(key, modifiers);
	}
}

SColumnInfo	CAdbkSearchTable::GetColumnInfo(TableIndexT col)
{
	return mWindow->GetColumnInfo()[col - 1];
}

// Test for selected addr
bool CAdbkSearchTable::TestSelectionAddr(TableIndexT row)
{
	TableIndexT	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = GetNestingLevel(woRow);
	
	return (nestingLevel > 0);
}

// New address book
void CAdbkSearchTable::OnNewLetter(void)
{
	CreateNewLetter(GetDisplay()->GetLatestKeyModifiers().control());
}

// Create new letter from selection
void CAdbkSearchTable::CreateNewLetter(bool option_key)
{
	// Create list of selected addresses
	CAddressList list;

	// Copy selection into list for active list only
	AddSelectionToList(&list);

	// Process addressing info
	CReplyChooseDialog::ProcessChoice(&list, !(CPreferences::sPrefs->mOptionKeyAddressDialog.GetValue() ^ option_key));
}

void CAdbkSearchTable::OnEditCopy()
{
	cdstring txt;

	// Add all selected address to text
	DoToSelection1((DoToSelection1PP) &CAdbkSearchTable::AddAddressText, &txt);

	// Now copy to scrap
	CClipboard::CopyToSecondaryClipboard(GetDisplay(), txt);
}

// Add address as text
bool CAdbkSearchTable::AddAddressText(TableIndexT row, cdstring* txt)
{
	int	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = GetNestingLevel(woRow);

	// Only if address
	if (!nestingLevel)
		return false;

	// Get the relevant address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(GetCellData(row));
	
	cdstring full_addr = addr->GetFullAddress();

	// Add CRLF before if multi-copy
	if (txt->length())
		*txt += os_endl;
	*txt += full_addr;
		
	return true;
}		

// Open address book
void CAdbkSearchTable::OnSearch(void)
{
	switch(mWindow->mSourcePopup->GetValue())
	{
	case menu_AddrSearchMethodMulberry:
		{
			// Check that search address books exist
			if (!CAddressBookManager::sAddressBookManager->CanSearch())
			{
				// Display warning
				CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::NoSearch");
				return;
			}

			// Get details from controls
			cdstring lookup_name = mWindow->mSearchText->GetText();
			
			CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mWindow->mFieldPopup->GetValue() - 1);
			cdstring field_name = mWindow->mFieldPopup->GetCurrentItemText().GetCString();

			try
			{
				// Do a search
				CAdbkAddress::EAddressMatch match = (CAdbkAddress::EAddressMatch) mWindow->mMethodPopup->GetValue();
				CAddressBookManager::sAddressBookManager->SearchAddress(lookup_name, match, field, field_name);

				// Append last item to table
				AppendItem(CAddressBookManager::sAddressBookManager->GetSearchResultList().back());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not throw up
			}
		}
		break;

	case menu_AddrSearchMethodLDAP:
		{
			// Get details from controls
			cdstring lookup_name = mWindow->mSearchText->GetText();
			
			try
			{
				// Do a search
				CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mWindow->mFieldPopup->GetValue() - 1);
				CAdbkAddress::EAddressMatch match = (CAdbkAddress::EAddressMatch) mWindow->mMethodPopup->GetValue();
				CAddressBookManager::sAddressBookManager->SearchAddress(CAddressBookManager::eLDAPLookup, false, lookup_name, match, field);
				
				// Append last item to table
				AppendItem(CAddressBookManager::sAddressBookManager->GetSearchResultList().back());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not throw up
			}
		}
		break;

	default:
		break;
	}
}

// Rename address books
void CAdbkSearchTable::OnClear()
{
	// Clear all results and force table reset
	CAddressBookManager::sAddressBookManager->ClearSearch();
	ResetTable();
}

// Edit selected entries
void CAdbkSearchTable::DoEditEntry(void)
{
	// Edit each selected message
	DoToSelection((DoToSelectionPP) &CAdbkSearchTable::EditEntry);
}

// Edit specified address
bool CAdbkSearchTable::EditEntry(TableIndexT row)
{
	TableIndexT	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = GetNestingLevel(woRow);
	bool done_edit = false;

	if (nestingLevel)
	{
		CAdbkAddress* data = static_cast<CAdbkAddress*>(GetCellData(row));

		// Create the dialog - prevent address edit
		CEditAddressDialog::PoseDialog(data, false);
	}
	
	return done_edit;
}

// Delete address books
void CAdbkSearchTable::OnDeleteAddress(void)
{
	// Delete each selected message in reverse
	DoToSelection((DoToSelectionPP) &CAdbkSearchTable::DeleteAddress, false);
}

// Delete specified address book
bool CAdbkSearchTable::DeleteAddress(TableIndexT row)
{
	TableIndexT	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	void* data = GetCellData(row);
	UInt32	nestingLevel = GetNestingLevel(woRow);
	
	// Delete whole item or single address
	if (nestingLevel)
	{
		// Get parent item
		TableIndexT parentRow = GetParentIndex(woRow);
		void* item = mData.at(parentRow - 1);
		CAddressBookManager::sAddressBookManager->ClearSearchItemAddress((CAddressSearchResult*) item, (CAdbkAddress*) data);
		RemoveRows(1, woRow, true);
		mData.erase(mData.begin() + (woRow - 1));
		if (woRow < mLastParent)
			mLastParent--;
	}
	else
	{
		size_t count = 1 + ((CAddressSearchResult*) data)->second->size();
		CAddressBookManager::sAddressBookManager->ClearSearchItem((CAddressSearchResult*) data);
		RemoveRows(1, woRow, true);
		mData.erase(mData.begin() + (woRow - 1), mData.begin() + (woRow - 1 + count));
		if (woRow < mLastParent)
			mLastParent -= count;
		else if (woRow == mLastParent)
		{
			mLastParent = GetParentIndex(mCollapsableTree->CountNodes());
		}
	}
	
	return true;
}

// Reset the table from the mboxList
void CAdbkSearchTable::ResetTable(void)
{
	// Delete all existing rows (do not allow redraw)
	Clear();
	mData.clear();

	// Get list from manager
	const CAddressSearchResultList& results = CAddressBookManager::sAddressBookManager->GetSearchResultList();

	// Add each list entry
	mLastParent = 0;
	for(CAddressSearchResultList::const_iterator iter = results.begin(); iter != results.end(); iter++)
	{
		TableIndexT parent_row;
		mData.push_back(*iter);
		mLastParent = InsertSiblingRows(1, mLastParent, &(*iter), sizeof(void*), true, false);
		parent_row = mLastParent;
		AddList((*iter)->second, parent_row);
		CollapseRow(mLastParent);
	}

} // CAdbkSearchTable::ResetTable

// Add an item to end of list
void CAdbkSearchTable::AppendItem(const CAddressSearchResult* item)
{
	// Insert each address as child
	mData.push_back((void*) item);
	mLastParent = InsertSiblingRows(1, mLastParent, &item, sizeof(void*), true, true);
	TableIndexT parent_row = mLastParent;
	AddList(item->second, parent_row);
	ExpandRow(mLastParent);
	RefreshRowsBelow(mLastParent);
}

// Add a node to the list
void CAdbkSearchTable::AddList(const CAddressList* list, TableIndexT& row)
{
	// Insert each address as child
	for(CAddressList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		mData.push_back(*iter);
		AddLastChildRow(row, &(*iter), sizeof(CAdbkAddress*), false, false);
	}
}

// Get the data
void* CAdbkSearchTable::GetCellData(TableIndexT row)
{
	int	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	return mData.at(woRow - 1);
}

// Add selected addresses to list
void CAdbkSearchTable::AddSelectionToList(CAddressList* list)
{
	// Add all selected address to drag
	DoToSelection1((DoToSelection1PP) &CAdbkSearchTable::CopyEntryToList, list);
}

// Copy address to list
bool CAdbkSearchTable::CopyEntryToList(TableIndexT row, CAddressList* list)
{
	TableIndexT	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = GetNestingLevel(woRow);

	// Only if address
	if (!nestingLevel)
		return false;

	// Get the relevant address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(GetCellData(row));

	// Copy address
	CAdbkAddress* copy = new CAdbkAddress(*addr);

	// Save in selected list if not duplicate
	if (list->InsertUniqueItem(copy))
		return true;
	else
	{
		delete copy;
		return false;
	}
}		

// Draw the titles
void CAdbkSearchTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState save(pDC);

	// Get cell data item
	int	woRow = GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	void* data = GetCellData(inCell.row);
	UInt32	nestingLevel = GetNestingLevel(woRow);

	unsigned long bkgnd = 0x00FFFFFF;
	if (nestingLevel && CellIsSelected(inCell) && IsActive() && HasFocus())
	{
		JColorIndex bkgnd_index = GetColormap()->GetDefaultSelectionColor();
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	cdstring theTxt;

	// Use different color for result heading
	if (!nestingLevel)
	{
		StPenState save(pDC);
		JRect rect(inLocalRect);
		rect.bottom--;
		JColorIndex bkgnd_index;
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerBkgndStyle.GetValue().color, &bkgnd_index);
		pDC->SetPenColor(bkgnd_index);
		pDC->SetFilling(kTrue);
		pDC->RectInside(rect);
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}
	if (!nestingLevel)
	{
		JFontStyle text_style;
		text_style.bold = kTrue;
		pDC->SetFontStyle(text_style);
	}

	// Determine which heading it is
	SColumnInfo col_info = GetColumnInfo(inCell.col);

	ResIDT iconID = nestingLevel ? IDI_ADDRESS_FLAG : IDI_ADDRESS_RESULT;
	JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);

	if (nestingLevel)
	{
		switch(col_info.column_type)
		{
		case eAddrColumnName:
		case eAddrColumnNameLastFirst:
			theTxt = ((CAdbkAddress*) data)->GetName();
			break;
		
		case eAddrColumnNickName:
			theTxt = static_cast<CAdbkAddress*>(data)->GetADL();
			break;
		
		case eAddrColumnEmail:
			theTxt = static_cast<CAdbkAddress*>(data)->GetMailAddress();
			break;
		
		case eAddrColumnCompany:
			theTxt = static_cast<CAdbkAddress*>(data)->GetCompany();
			break;
		
		case eAddrColumnAddress:
			theTxt = static_cast<CAdbkAddress*>(data)->GetAddress(CAdbkAddress::eDefaultAddressType);
			break;
		
		case eAddrColumnPhoneWork:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eWorkPhoneType);
			break;
		
		case eAddrColumnPhoneHome:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eHomePhoneType);
			break;
		
		case eAddrColumnFax:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eFaxType);
			break;
		
		case eAddrColumnURLs:
			theTxt = static_cast<CAdbkAddress*>(data)->GetURL();
			break;
		
		case eAddrColumnNotes:
			theTxt = static_cast<CAdbkAddress*>(data)->GetNotes();
			break;
		
		default:
			break;
		}
		if (inCell.col == 1)
			DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, icon);
		else
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Left);
	}
	else if (inCell.col == 1)
	{
		theTxt = static_cast<CAddressSearchResult*>(data)->first;
		DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, icon);
	}
}

// Send data to target
bool CAdbkSearchTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	if (type == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold deleted items
		std::auto_ptr<CAddressList> addrs(new CAddressList);
		addrs->set_delete_data(false);

		// Add each selected address
		AddSelectionToList(addrs.get());
		seldata->SetData(type, reinterpret_cast<unsigned char*>(addrs.release()), sizeof(CAddressList*));
		rendered = true;
	}

	else if ((type == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (type == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		cdstring txt;
		bool first = true;

		// Create list to hold deleted items
		CAddressList addrs;

		// Add each selected address
		AddSelectionToList(&addrs);

		for(CAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
		{
			if (first)
				first = false;
			else
				txt += os_endl;
			txt +=(*iter)->GetFullAddress();
		}
	
		// Allocate global memory for the text if not already
		unsigned long dataLength = txt.length() + 1;
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			::memcpy(data, txt.c_str(), dataLength);
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
		
		// Do not delete originals
		addrs.clear_without_delete();
	}
	
	return rendered;
}
