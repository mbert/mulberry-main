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

//#include "StValueChanger.h"

#include "CAdbkSearchWindow.h"
#include "CReplyChooseDialog.h"
#include "CDrawUtils.h"
#include "CEditAddressDialog.H"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CUnicodeUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


IMPLEMENT_DYNCREATE(CAdbkSearchTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CAdbkSearchTable, CHierarchyTableDrag)
	ON_UPDATE_COMMAND_UI(IDM_FILE_NEW_DRAFT, OnUpdateSelection)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnNewLetter)

	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)

	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTable::CAdbkSearchTable()
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);

	mLastParent = 0;
}

// Default destructor
CAdbkSearchTable::~CAdbkSearchTable()
{
}

int CAdbkSearchTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info

	AddDragFlavor(CMulberryApp::sFlavorAddrList);
	AddDragFlavor(CF_UNICODETEXT);

	SetReadOnly(true);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);
	SetAllowMove(false);

	mWindow = (CAdbkSearchWindow*) GetParent();

	// Force update to existing reults
	ResetTable();

	return 0;
}

// Keep titles in sync
void CAdbkSearchTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mWindow->mTitles.ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Double-clicked item
void CAdbkSearchTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoEditEntry();
}

// Handle key down
bool CAdbkSearchTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		DoEditEntry();
		break;
	case VK_BACK:
	case VK_DELETE:
		OnDeleteAddress();
		break;
	default:
		return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
	}
	return true;
}

// Test for selected addr
bool CAdbkSearchTable::TestSelectionAddr(TableIndexT row)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	
	return (nestingLevel > 0);
}

void CAdbkSearchTable::OnEditCopy()
{
	cdstring txt;

	// Create list to hold deleted items
	CAddressList addrs;
	bool first = true;

	// Add each selected address
	AddSelectionToList(&addrs);

	for(CAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
	{
		if (first)
			first = false;
		else
			txt += "\r\n";
		txt += (*iter)->GetFullAddress();
	}
	
	// Convert to utf16
	cdustring utf16(txt);

	// Now copy to scrap
	if (!::OpenClipboard(*this))
		return;
	::EmptyClipboard();

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
	::CloseClipboard();
}

// New address book
void CAdbkSearchTable::OnNewLetter(void)
{
	CreateNewLetter(::GetKeyState(VK_MENU) < 0);
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

// Open address book
void CAdbkSearchTable::OnSearch(void)
{
	switch(mWindow->mSourcePopup.GetValue())
	{
	case IDM_ADBK_SOURCE_MULBERRY:
		{
			// Check that search address books exist
			if (!CAddressBookManager::sAddressBookManager->CanSearch())
			{
				// Display warning
				CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::NoSearch");
				return;
			}

			// Get details from controls
			cdstring lookup_name;
			mWindow->mSearchText.GetText(lookup_name);
			
			CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mWindow->mFieldPopup.GetValue() - IDM_ADBK_FIELD_NAME);
			cdstring field_name = CUnicodeUtils::GetWindowTextUTF8(&mWindow->mFieldPopup);

			try
			{
				// Do a search
				CAdbkAddress::EAddressMatch match = (CAdbkAddress::EAddressMatch) (mWindow->mMethodPopup.GetValue() - IDM_ADBK_METHOD_IS + 1);
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

	case IDM_ADBK_SOURCE_LDAP:
		{
			// Get details from controls
			cdstring lookup_name;
			mWindow->mSearchText.GetText(lookup_name);

			
			try
			{
				// Do a search
				CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mWindow->mFieldPopup.GetValue() - IDM_ADBK_FIELD_NAME);
				CAdbkAddress::EAddressMatch match = (CAdbkAddress::EAddressMatch) (mWindow->mMethodPopup.GetValue() - IDM_ADBK_METHOD_IS + 1);
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

	case IDM_ADBK_SOURCE_WHOISPP:
	case IDM_ADBK_SOURCE_FINGER:
		break;
	}
}

// Rename address books
void CAdbkSearchTable::OnClear(void)
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
	
	SetFocus();
}

// Edit specified address
bool CAdbkSearchTable::EditEntry(TableIndexT row)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool done_edit = false;

	if (nestingLevel)
	{
		CAdbkAddress* data = static_cast<CAdbkAddress*>(GetCellData(row));

		// Do the dialog
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
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	void* data = GetCellData(row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	
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
	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	// Start cursor for busy operation
	CWaitCursor wait;

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
	int	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

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
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

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
void CAdbkSearchTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Get cell data item
	int	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	void* data = GetCellData(inCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	cdstring theTxt;

	// Use different color for result heading
	if (!nestingLevel)
	{
		pDC->SetBkColor(CDrawUtils::sGrayColor);

	   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
	   	CRect cellRect = inLocalRect;
	   	cellRect.bottom--;
		pDC->ExtTextOut(cellRect.left, cellRect.top, ETO_OPAQUE, cellRect, _T(""), 0, nil);

		// Use bold font
		pDC->SelectObject(CFontCache::GetListFontBold());
	}

	// Determine which heading it is
	SColumnInfo col_info = (*mColumnInfo)[inCell.col - 1];

	UINT iconID = nestingLevel ? IDI_ADDRESS_FLAG : IDI_ADDRESS_RESULT;

	if (nestingLevel)
	{
		switch(col_info.column_type)
		{
		case eAddrColumnName:
		case eAddrColumnNameLastFirst:
			theTxt = static_cast<CAdbkAddress*>(data)->GetName();
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
			DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, iconID, CellIsSelected(inCell));
		else
		{
			// Move to origin for text
			int x = inLocalRect.left + 4;
			int y = inLocalRect.top + mTextOrigin;

			::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, eDrawString_Left);
		}
	}
	else if (inCell.col == 1)
	{
		theTxt = static_cast<CAddressSearchResult*>(data)->first;
		DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, iconID);
	}
}

// Send data to target
BOOL CAdbkSearchTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold deleted items
		CAddressList addrs;

		// Add each selected address
		AddSelectionToList(&addrs);
		
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
				txt += "\r\n";
			txt +=(*iter)->GetFullAddress();
		}
	
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
		
		// Do not delete originals
		addrs.clear_without_delete();
	}
	
	return rendered;
}
