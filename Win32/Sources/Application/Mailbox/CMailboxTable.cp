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


// Source for CMailboxTable class


#include "CMailboxTable.h"

#include "CAddressList.h"
#include "CCopyToMenu.h"
#include "CDrawUtils.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMailControl.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxView.h"
#include "CMailCheckThread.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CUnicodeUtils.h"
#include "CUserAction.h"

#include "StValueChanger.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxTable

BEGIN_MESSAGE_MAP(CMailboxTable, CTableDragAndDrop)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateSelectionNotDeleted)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateNever)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateSelectionNotDeleted)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateNever)
	//ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateSelectionNotDeleted)
	//ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(IDM_FILE_OPEN_MESSAGE, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_FILE_OPEN_MESSAGE, OnFileOpenMessage)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_SPEAK, OnUpdateEditSpeak)
	ON_COMMAND(IDM_EDIT_SPEAK, OnEditSpeak)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_EXPUNGE, OnUpdateMailboxExpunge)
	ON_COMMAND(IDM_MAILBOX_EXPUNGE, OnMailboxExpunge)

	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)

	ON_COMMAND(IDC_MAILBOXSORTBTN, OnSortDirection)
	ON_UPDATE_COMMAND_UI(IDM_SORT_TO, OnUpdateSortMenu)

	ON_COMMAND_RANGE(IDM_SORT_TO, IDM_SORT_DISCONNECTED, OnSortItem)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SORT_TO, IDM_SORT_DISCONNECTED, OnUpdateSortMenu)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_CopyToMailboxChoose, IDM_CopyToMailboxEnd, OnUpdateMessageCopy)
	ON_COMMAND_RANGE(IDM_CopyToMailboxChoose, IDM_CopyToMailboxEnd, OnMessageCopy)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnUpdateMessageCopy)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnMessageCopyPopup)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_COPY_NOW, OnUpdateMessageCopy)
	ON_COMMAND(IDM_MESSAGES_COPY_NOW, OnMessageCopyNow)

	ON_COMMAND(IDC_MAILBOXCOPYCMD, OnMessageCopyCmd)
	ON_COMMAND(IDC_MAILBOXMOVECMD, OnMessageMoveCmd)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_DELETE, OnUpdateMessageDelete)
	ON_COMMAND(IDM_MESSAGES_DELETE, OnMessageDelete)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_CAPTURE, OnUpdateSelection)
	ON_COMMAND(IDM_ADDR_CAPTURE, OnCaptureAddress)

	ON_WM_CREATE()
	ON_WM_PAINT()
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMAILBOXEXPUNGEBTN, OnUpdateMailboxExpunge)
	ON_COMMAND(IDC_TOOLBARMAILBOXEXPUNGEBTN, OnMailboxExpunge)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEDELETEBTN, OnUpdateMessageDelete)
	ON_COMMAND(IDC_TOOLBARMESSAGEDELETEBTN, OnMessageDelete)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGECOPYBTN, OnUpdateMessageCopy)
	ON_COMMAND(IDC_TOOLBARMESSAGECOPYBTN, OnMessageCopyNow)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

int CMailboxTable::sSortCol = false;
bool CMailboxTable::sDropOnMailbox = false;

// Default constructor
CMailboxTable::CMailboxTable()
{
	mMbox = NULL;
	mMboxError = false;
	mColumnInfo = NULL;
	mUpdating = false;
	mListChanging = false;
	mUpdateRequired = false;
	mSelectionPreserved = false;
	mResetTable = false;
	mPreviewUID = 0;
	mIsSelectionValid = false;
	mTestSelectionAndDeleted = false;

	mTableGeometry = new CTableRowGeometry(this, 72, 16);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);
}

// Default destructor
CMailboxTable::~CMailboxTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMailboxTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableDragAndDrop::OnCreate(lpCreateStruct) == -1)
		return -1;

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CMailboxView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CMailboxView*>(parent);
	SetSuperCommander(mTableView);

	return 0;
}

// Keep titles in sync
void CMailboxTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Do scroll of main table first to avoid double-refresh
	CTableDragAndDrop::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);

	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);
}

// Handle context menu popup
void CMailboxTable::HandleContextMenu(CWnd*, CPoint point)
{
	// Must update the selection state before doing the popup
	// The selection updating has been deferred at this point
	DoSelectionChanged();

	CMenu menu;
	VERIFY(menu.LoadMenu(mContextMenuID));
	CMenu* pPopup = menu.GetSubMenu(0);
	CMenu* pCopyPopup = pPopup->GetSubMenu(8);

	// Delete existing items in mailbox list
	while(pCopyPopup->DeleteMenu(0, MF_BYPOSITION)) {};

	// Refresh existing menu first in case of change
	CCopyToMenu::ResetMenuList();

	// Add items from existing mailbox menu to new menu
	CMenu* pCopyToPopup = CCopyToMenu::GetPopupMenu(true)->GetSubMenu(0);
	for(int i = 0; i < pCopyToPopup->GetMenuItemCount(); i++)
	{
		UINT nID;
		UINT uMenuState;
		
		uMenuState = pCopyToPopup->GetMenuState(i, MF_BYPOSITION);
		nID = pCopyToPopup->GetMenuItemID(i);
		cdstring name = CUnicodeUtils::GetMenuStringUTF8(pCopyToPopup, i, MF_BYPOSITION);
		
		if (uMenuState & MF_SEPARATOR)
			CUnicodeUtils::AppendMenuUTF8(pCopyPopup, MF_SEPARATOR, nID, name);
		else
			CUnicodeUtils::AppendMenuUTF8(pCopyPopup, LOBYTE(uMenuState), nID, name);
	}

	// Change copy/move title
	CString txt;
	txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
	pPopup->ModifyMenu(8, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) pCopyPopup->m_hMenu, txt);

	// Track the popup
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);
}

int CMailboxTable::GetSortBy() const
{
	if (!GetMbox())
		return cSortMessageNumber;

	return GetMbox()->GetSortBy();
}

// Test each cell in the selection using logical AND
bool CMailboxTable::TestSelectionIgnore(TestSelectionIgnorePP proc, bool and_it)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = and_it ? true : false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
		{
			pair<bool, bool> result = (this->*proc)(aCell.row);
			if (result.second)
				test = (and_it ? (result.first && test) : (result.first || test));
		}
	}

	return test;
}

// Test each cell in the selection using logical AND
bool CMailboxTable::TestSelectionIgnore1(TestSelectionIgnore1PP proc, NMessage::EFlags flag, bool and_it)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = and_it ? true : false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
		{
			pair<bool, bool> result = (this->*proc)(aCell.row, flag);
			if (result.second)
				test = (and_it ? (result.first && test) : (result.first || test));
		}
	}

	return test;
}

#pragma mark ____________________________Display Updating

void CMailboxTable::DoSelectionChanged(void)
{
	mIsSelectionValid = IsSelectionValid() && !TestSelectionAnd((TestSelectionPP) &CMailboxTable::TestSelectionFake);
	mTestSelectionAndDeleted = TestSelectionIgnore1And(&CMailboxTable::TestSelectionFlag, NMessage::eDeleted);
	
	// Do inherited to ensure broadcast
	CTableDragAndDrop::DoSelectionChanged();
}

// Reset the table from the mbox
void CMailboxTable::ResetTable(bool scroll_new)
{
	// Preserve selection and prevent updates
	StMailboxTableSelection preserve_selection(this);
	
	{
		// Prevent screen updates because of multi-thread access
		StValueChanger<bool> _change(mListChanging, true);

		long num_msgs = (GetMbox() ? GetMbox()->GetNumberMessages() : 0);

		TableIndexT	old_rows;
		TableIndexT	old_cols;
		GetTableSize(old_rows, old_cols);

		if (old_rows > num_msgs)
			RemoveRows(old_rows - num_msgs, 1, false);
		else if (old_rows < num_msgs)
			InsertRows(num_msgs - old_rows, 1, NULL, 0, false);

		// Previous selection no longer valid
		UnselectAllCells();
	}
	
	// Scroll to display new messages
	if (scroll_new)
		ScrollForNewMessages();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE);

	// Set flag
	mResetTable = true;
}

// Clear the table of all contents
void CMailboxTable::ClearTable(void)
{
	// Prevent drawing to stop row delete causing immediate redraw and crash when scroll bar dissappears
	StDeferTableAdjustment changing(this);
	StDeferSelectionChanged noupdates(this);

	// Remove all rows then add all new ones
	RemoveAllRows(true);
}

#pragma mark ____________________________________Keyboard/Mouse

// Clicked item
void CMailboxTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	// Need to bypass CTableDragAndDrop since it won't tell us whether
	// a drag operation was done and we don't want to do auto-matching
	// if a drag was done

	// Try drag and drop
	if (mAllowDrag)
	{
		if (DragDetect(mLDownPoint))
		{
			DoDrag(inCell.row);
			return;
		}

		// Active if mouse activate not done
		GetParentFrame()->ActivateFrame();
		SetFocus();
	}

	CTable::LClickCell(inCell, nFlags);
	DoSingleClick(inCell.row, inCell.col, CKeyModifiers(0));
}

// Double-clicked item
void CMailboxTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoDoubleClick(inCell.row, inCell.col, CKeyModifiers(0));
}

// Handle key down
bool CMailboxTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Determine whether preview is triggered
	CKeyModifiers mods(0);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == nChar) &&
		(preview.GetKeyModifiers() == mods))
	{
		PreviewMessage();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == nChar) && (fullview.GetKeyModifiers() == mods))
	{
		DoFileOpenMessage(mods.Get(CKeyModifiers::eAlt));
		return true;
	}

	switch(nChar)
	{
	case VK_SPACE:
		HandleSpacebar(mods.Get(CKeyModifiers::eShift));
		break;

	case VK_BACK:
	case VK_DELETE:
		if (mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(NMessage::eDeleted))
			OnMessageDelete();
		break;

	// Hierarchy arrows left/right
	case VK_LEFT:
	case VK_RIGHT:
		if (GetMbox() && (GetMbox()->GetSortBy() == cSortMessageThread))
		{
			TableIndexT row = GetFirstSelectedRow();
			if (row >= 1)
			{
				// Get message
				const CMessage* thisMsg = GetMbox()->GetMessage(row, true);

				if (thisMsg)
				{
					// Move up/down hierarchy to sibling
					const CMessage* theMsg = (nChar == VK_LEFT) ?
								thisMsg->GetThreadPrevious() : thisMsg->GetThreadNext();

					// Go up to parent if no previous
					if ((nChar == VK_LEFT) && !theMsg)
						theMsg = thisMsg->GetThreadParent();

					// Only if parent exists
					if (theMsg)
					{
						row = GetMbox()->GetMessageIndex(const_cast<CMessage*>(theMsg), true);

						// Select and bring into middle of view if outside
						ScrollToRow(row, true, true, eScroll_Center);
					}
				}
			}
		}
		break;

	default:
		// Did not handle key
		return CTableDragAndDrop::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

#pragma mark ____________________________________Command Updaters

void CMailboxTable::OnUpdateEditSpeak(CCmdUI* pCmdUI)
{
	// Pass to speech synthesis
	if (!CSpeechSynthesis::OnUpdateEditSpeak(IDM_EDIT_SPEAK, pCmdUI))
	{
		CString txt;
		txt.LoadString(IDS_SPEAK_SPEAKSELECTION);
		OnUpdateMenuTitle(pCmdUI, txt);

		// Enable menu only if there's a selection
		if (mIsSelectionValid)
			pCmdUI->Enable(mIsSelectionValid &&
							(!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue()));
	}
}

void CMailboxTable::OnUpdateMailboxExpunge(CCmdUI* pCmdUI)
{
	// Must be some deleted
	pCmdUI->Enable(GetMbox() && GetMbox()->AnyDeleted() && !GetMbox()->IsReadOnly());
}

void CMailboxTable::OnUpdateSortMenu(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current sort method
	pCmdUI->SetCheck(GetMbox() && (GetMbox()->GetSortBy() - 1 == pCmdUI->m_nIndex));

}

void CMailboxTable::OnUpdateSelectionNotDeleted(CCmdUI* pCmdUI)
{
	// Must have selection with all undeleted
	pCmdUI->Enable(IsSelectionValid() &&
					(!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue()));
}

void CMailboxTable::OnUpdateMessageCopy(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	if ((pCmdUI->m_pSubMenu == NULL) && (pCmdUI->m_nID >= IDM_CopyToMailboxChoose) && (pCmdUI->m_nID <= IDM_CopyToMailboxEnd) ||
		(pCmdUI->m_nID >= IDM_CopyToPopupNone) && (pCmdUI->m_nID <= IDM_CopyToPopupEnd))
		CCopyToMenu::ResetMenuList();

	// Do update for non-deleted selection
	bool enable = mIsSelectionValid && (!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue());
	if (pCmdUI->m_pSubMenu)
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | (enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
 	else
		pCmdUI->Enable(enable);

	// Adjust menu title move/copy
	CString txt;
	switch(pCmdUI->m_nID)
	{
	case IDM_MESSAGES_COPY_NOW:
		txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVENOW_CMD_TEXT : IDS_COPYNOW_CMD_TEXT);
		break;
	case IDC_TOOLBARMESSAGECOPYBTN:
		txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETO_CMD_TEXT : IDS_COPYTO_CMD_TEXT);
		break;
	default:
		if ((pCmdUI->m_pSubMenu != NULL) && (pCmdUI->m_pSubMenu->m_hMenu == CCopyToMenu::GetMenu(true)->m_hMenu))
			txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
		break;
	}

	if (!txt.IsEmpty())
	{
		if (pCmdUI->m_pSubMenu)
			pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) pCmdUI->m_pMenu->GetSubMenu(pCmdUI->m_nIndex)->m_hMenu, txt);
		else
			OnUpdateMenuTitle(pCmdUI, txt);
	}
}

void CMailboxTable::OnUpdateMessageDelete(CCmdUI* pCmdUI)
{
	CString txt;
	if (GetMbox() && !GetMbox()->HasAllowedFlag(NMessage::eDeleted))
	{
		pCmdUI->Enable(false);
		txt.LoadString(IDS_DELETE_CMD_TEXT);
	}
	else
	{
		pCmdUI->Enable(mIsSelectionValid);
		txt.LoadString(mTestSelectionAndDeleted ? IDS_UNDELETE_CMD_TEXT : IDS_DELETE_CMD_TEXT);
		if (!pCmdUI->m_pMenu)
			pCmdUI->SetCheck(mTestSelectionAndDeleted);
	}
	OnUpdateMenuTitle(pCmdUI, txt);
}

#pragma mark ____________________________________Command Handlers

void CMailboxTable::OnFileOpenMessage(void)
{
	// Open it
	DoFileOpenMessage(::GetKeyState(VK_MENU) < 0);
}

void CMailboxTable::OnFilePrint(void)
{
	// Print each selected message
	bool preview = false;
	DoToSelection1((DoToSelection1PP) &CMailboxTable::PrintMailMessage, &preview);
}

void CMailboxTable::OnFilePrintPreview(void)
{
	// Print preview each selected message
	bool preview = true;
	DoToSelection1((DoToSelection1PP) &CMailboxTable::PrintMailMessage, &preview);
}

// Print/preview a specified mail message
bool CMailboxTable::PrintMailMessage(TableIndexT row, void* preview)
{
	if (!GetMbox())
		return false;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake()) return false;

	// Don't print if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
	{
		::MessageBeep(-1);
		return false;
	}

	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(theMsg);

	if (theWindow)
	{
		// Found existing window so print and return
		theWindow->SendMessage(WM_COMMAND, (*(bool*) preview) ? ID_FILE_PRINT_PREVIEW : ID_FILE_PRINT);
		return false;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Read the message
		//theMsg->ReadPart();

		// Create the message window and print save command to it
		newWindow = CMessageWindow::ManualCreate();
		newWindow->ShowWindow(SW_HIDE);
		newWindow->SetMessage(theMsg);
		newWindow->SendMessage(WM_COMMAND, (*(bool*) preview) ? ID_FILE_PRINT_PREVIEW : ID_FILE_PRINT);
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Window failed to be created
		if (CMessageWindow::FindWindow(theMsg))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return false;
}

void CMailboxTable::OnEditSpeak(void)
{
	if (!CSpeechSynthesis::OnEditSpeak(IDM_EDIT_SPEAK))
		DoSpeakMessage();
}

void CMailboxTable::OnMailboxExpunge(void)
{
	BeginMboxUpdate();
	
	try
	{
		DoMailboxExpunge(false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		EndMboxUpdate(false);
	}

	EndMboxUpdate();

	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
}

#pragma mark ____________________________Sorting

void CMailboxTable::OnSortDirection(void)
{
	if (!GetMbox())
		return;

	EShowMessageBy show = GetMbox()->GetShowBy();
	show = (EShowMessageBy) (3 - show);
	mTableView->GetSortBtn()->SetPushed(show == cShowMessageDescending);
	mTableView->SetShowBy(show);

	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
}

void CMailboxTable::OnSortItem(UINT nID)
{
	mTableView->SetSortBy(static_cast<ESortMessageBy>(cSortMessageTo + nID - IDM_SORT_TO));
	
	// Always refresh titles as it is a separate control rather thanpart of the table
	FRAMEWORK_REFRESH_WINDOW(mTableView->GetBaseTitles())
}

#pragma mark ____________________________Commands

// Copy the message
void CMailboxTable::OnMessageCopy(UINT nID)
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);

	// Find copied to mbox
	CMbox* copy_mbox = NULL;
	if (CCopyToMenu::GetMbox(true, nID - IDM_CopyToMailboxChoose, copy_mbox))
	{
		if (copy_mbox)
			DoMessageCopy(copy_mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

// Copy the message
void CMailboxTable::OnMessageCopyPopup(UINT nID)
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);

	// Find copied to mbox
	CMbox* copy_mbox = NULL;
	if (CCopyToMenu::GetPopupMbox(true, nID - IDM_CopyToPopupChoose, copy_mbox))
	{
		if (copy_mbox && (copy_mbox != (CMbox*) -1L))
			DoMessageCopy(copy_mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

// Copy the message
void CMailboxTable::OnMessageCopyNow()
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);
	OnMessageCopyIt(option_key);
}

// Copy the message
void CMailboxTable::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	OnMessageCopyIt(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMailboxTable::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	OnMessageCopyIt(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
}

// Common copy behaviour
void CMailboxTable::OnMessageCopyIt(bool option_key)
{
	// Get mailbox from toolbar button
	CMbox* mbox = NULL;
	if (mTableView->GetCopyBtn() && mTableView->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			DoMessageCopy(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

void CMailboxTable::OnMessageDelete(void)
{
	DoFlagMailMessage(NMessage::eDeleted);

	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
}

void CMailboxTable::OnCaptureAddress(void)
{
	DoCaptureAddress();
}

#pragma mark ____________________________Drawing

void CMailboxTable::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkColor(::GetSysColor(COLOR_WINDOW));
	dc.SelectObject(GetFont());

	CRect	updateRect = dc.m_ps.rcPaint;

	DrawBackground(&dc, updateRect);

		// Determine cells that need updating. Rather than checking
		// on a cell by cell basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.
	
	STableCell	topLeftCell, botRightCell;
	FetchIntersectingCells(updateRect, topLeftCell, botRightCell);
	
	// Only if open and not changing
	if (mTableView->IsOpen() && !mListChanging)
	{
		// Draw each cell within the update rect
		for (TableIndexT row = topLeftCell.row; row <= botRightCell.row; row++)
			DrawRow(&dc, row, topLeftCell.col, botRightCell.col);
	}
	
	// Trigger update aftr current op ends
	else if (mTableView->IsOpen() && mListChanging)
		mUpdateRequired = true;
}

// Draw the titles
void CMailboxTable::DrawRow(CDC* pDC, TableIndexT row, TableIndexT start_col, TableIndexT stop_col)
{	
	if (!GetMbox())
		return;

	// Allow get message to fail during caching process
	CMessage* theMsg = NULL;
	try
	{
		theMsg = GetMbox()->GetMessage(row, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
	if (!theMsg) return;
	
	// Check whether already cached
	if (!theMsg->IsFullyCached())
	{
		// do not allow caching while processing another network command
		if (GetMbox()->GetMsgProtocol() && !GetMbox()->GetMsgProtocol()->_get_mutex().is_locked())
		{
			StValueChanger<bool> _change(mListChanging, true);

			{
				// Preserve selection without scroll
				StMailboxTableSelection preserve_selection(this, false);
				
				try
				{
					theMsg = GetMbox()->GetCacheMessage(row, true);
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					theMsg = NULL;
				}
				if (!theMsg) return;
			}

			UpdateItems();

			// If sorting by anything other than by message number, must do entire refresh
			//if (GetMbox()->GetSortBy() != cSortMessageNumber)
				RedrawWindow(NULL, NULL, RDW_INVALIDATE);
		}
		else
		{
			// Give the mail check thread a quick boost so it gets a chance to run and clear the mutex we blocked on
			CMailCheckThread::BoostPriority();

			// Force refresh of cell for when protocol is unblocked
			RefreshRow(row);
			return;
		}
	}

	// Check state
	StDCState save(pDC);
	COLORREF text_color = RGB(0, 0, 0);
	short text_style = normal;
	bool strikeThrough = false;

	// Select appropriate color and style of text
	GetDrawStyle(theMsg, text_color, text_style, strikeThrough);

	for(TableIndexT col = start_col; col <= stop_col; col++)
	{
		STableCell inCell(row, col);
		CRect	inLocalRect;
		GetLocalCellRect(inCell, inLocalRect);
		
		bool selected = CellIsSelected(inCell);

		// Check for coloured background - only do when there is no selection, or
		// the selection is an outline
		if (UsesBackground(theMsg))
		{
			COLORREF rgb = GetBackground(theMsg);
			pDC->SetBkColor(rgb);

			// Adjust erase rect based on whether an active selection is present or not
		   	CRect eraseRect = inLocalRect;
			if (selected && !mHasFocus && !mIsDropTarget)
			{
				if (col == 1)
					eraseRect.left++;
				else if (col == mCols)
					eraseRect.right--;
			   	eraseRect.top++;
			   	eraseRect.bottom--;
			}
			else
			   	eraseRect.bottom--;

		   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
			pDC->ExtTextOut(eraseRect.left, eraseRect.top, ETO_OPAQUE, eraseRect, _T(""), 0, NULL);
		}

		// Draw selection
		if (DrawCellSelection(pDC, inCell))
			text_color = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

		// Draw the actual cell data
		DrawMessage(pDC, theMsg, inCell, inLocalRect, text_color, text_style, strikeThrough);
	}
}

// Draw the items
void CMailboxTable::DrawMessage(CDC* pDC, const CMessage* aMsg, const STableCell& inCell, const CRect &inLocalRect,
								COLORREF text_color, short text_style, bool strike_through)
{
	// Save text state in stack object
	cdstring		theTxt;
	UINT			smart_flag = 0;
	bool			multi = false;
	unsigned long	depth = 0;

	const CEnvelope* theEnv = aMsg->GetEnvelope();
	if (!theEnv)
		return;

	// Move to origin for text
	int x = inLocalRect.left + 4;
	int y = inLocalRect.top + mTextOrigin;

	UINT plot_flag = 0;
	bool do_plot = false;
	bool selected = CellIsSelected(inCell);

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	// Determine which heading it is and draw it
	bool right_just = false;
	switch (col_info.column_type)
	{

		case eMboxColumnSmart:
			// Set up the icons
			if (aMsg->IsSmartFrom())
				smart_flag = IDI_FLAG_SMART_FROM;
			else if (aMsg->IsSmartTo())
				smart_flag = IDI_FLAG_SMART_TO;
			else if (aMsg->IsSmartCC())
				smart_flag = IDI_FLAG_SMART_CC;
			else
				smart_flag = IDI_FLAG_SMART_BCC;

			// Check for message From current user
			if (aMsg->IsSmartFrom())
			{
				// Display first to address
				if (theEnv->GetTo()->size())
					theTxt = theEnv->GetTo()->front()->GetNamedAddress();
				multi = (theEnv->GetTo()->size() > 1);
			}
			else
			{
				// Display first from address (or if empty first to address)
				if (theEnv->GetFrom()->size())
				{
					theTxt = theEnv->GetFrom()->front()->GetNamedAddress();
					multi = (theEnv->GetFrom()->size() > 1);
				}
				else
				{
					if (theEnv->GetTo()->size())
						theTxt = theEnv->GetTo()->front()->GetNamedAddress();
					multi = (theEnv->GetTo()->size() > 1);
					smart_flag = IDI_FLAG_SMART_FROM;
				}
			}
			break;

		case eMboxColumnFrom:
			if (theEnv->GetFrom()->size())
				theTxt = theEnv->GetFrom()->front()->GetNamedAddress();
			multi = (theEnv->GetFrom()->size() > 1);
			break;

		case eMboxColumnTo:
			if (theEnv->GetTo()->size())
				theTxt = theEnv->GetTo()->front()->GetNamedAddress();
			multi = (theEnv->GetTo()->size() > 1);
			break;

		case eMboxColumnReplyTo:
			if (theEnv->GetReplyTo()->size())
				theTxt = theEnv->GetReplyTo()->front()->GetNamedAddress();
			multi = (theEnv->GetReplyTo()->size() > 1);
			break;

		case eMboxColumnSender:
			if (theEnv->GetSender()->size())
				theTxt = theEnv->GetSender()->front()->GetNamedAddress();
			multi = (theEnv->GetSender()->size() > 1);
			break;

		case eMboxColumnCc:
			if (theEnv->GetCC()->size())
				theTxt = theEnv->GetCC()->front()->GetNamedAddress();
			multi = (theEnv->GetCC()->size() > 1);
			break;

		case eMboxColumnSubject:
		case eMboxColumnThread:
			// Check that fake subject exists
			if (aMsg->IsFake() && !aMsg->GetEnvelope()->HasFakeSubject())
				const_cast<CMessage*>(aMsg)->MakeFakeSubject();

			theTxt = theEnv->GetSubject();
			
			// Change depth if doing thread sort
			if (GetMbox()->GetSortBy() == cSortMessageThread)
				depth = aMsg->GetThreadDepth();
			break;

		case eMboxColumnDateSent:
			theTxt = theEnv->GetTextDate(true);
			break;

		case eMboxColumnDateReceived:
			theTxt = aMsg->GetTextInternalDate(true);
			break;

		case eMboxColumnSize:
			{
				unsigned long msg_size = aMsg->GetSize();
				theTxt = ::GetNumericFormat(msg_size);

				right_just = true;
			}
			break;

		case eMboxColumnFlags:
			do_plot = true;
			plot_flag = IDI_FLAG_DELETED + GetPlotFlag(aMsg);
			break;

		case eMboxColumnNumber:
			theTxt = (long) aMsg->GetMessageNumber();
			right_just = true;
			break;

		case eMboxColumnAttachments:
			{
				do_plot = true;
				
				bool is_styled = aMsg->GetBody()->HasStyledText();
				bool is_attach = aMsg->GetBody()->HasNonText();
				bool is_calendar = aMsg->GetBody()->HasCalendar();
				bool is_signed = aMsg->GetBody()->IsVerifiable();
				bool is_encrypted = aMsg->GetBody()->IsDecryptable();

				// Encrypted data always shown with single flag
				if (is_encrypted)
					plot_flag = IDI_FLAG_ENCRYPTED;

				// Check for signed data of various types
				else if (is_signed)
				{
					if (is_styled && is_attach)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE_ATTACHMENT;
					else if (is_styled && is_calendar)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE_CALENDAR;
					else if (is_styled)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE;
					else if (is_calendar)
						plot_flag = IDI_FLAG_SIGNED_CALENDAR;
					else if (is_attach)
						plot_flag = IDI_FLAG_SIGNED_ATTACHMENT;
					else
						plot_flag = IDI_FLAG_SIGNED;
				}

				// Check for unsigned data of various types
				else
				{
					if (is_styled && is_attach)
						plot_flag = IDI_FLAG_ALTERNATIVE_ATTACHMENT;
					else if (is_styled && is_calendar)
						plot_flag = IDI_FLAG_ALTERNATIVE_CALENDAR;
					else if (is_styled)
						plot_flag = IDI_FLAG_ALTERNATIVE;
					else if (is_calendar)
						plot_flag = IDI_FLAG_CALENDAR;
					else if (is_attach)
						plot_flag = IDI_FLAG_ATTACHMENT;
					else
						plot_flag = 0;
				}
			}
			break;

		case eMboxColumnParts:
			theTxt = (long) aMsg->GetBody()->CountParts();
			right_just = true;
			break;

		case eMboxColumnMatch:
			do_plot = true;
			if (aMsg->IsSearch())
				plot_flag = IDI_FLAG_MATCH;
			break;

		case eMboxColumnDisconnected:
			do_plot = true;
			if (aMsg->IsFullLocal())
				plot_flag = IDI_FLAG_CACHED;
			else if (aMsg->IsPartialLocal())
				plot_flag = IDI_FLAG_PARTIAL;
			break;
	}


	if (multi)
	{
		text_style |= (CPreferences::sPrefs->mMultiAddress.GetValue() & 0x007F);
		strike_through |= ((CPreferences::sPrefs->mMultiAddress.GetValue() & 0x0080) != 0);
	}

	// If message is fake and text is empty, use questiuon marks
	if (aMsg->IsFake() && (col_info.column_type != eMboxColumnSubject) && (col_info.column_type != eMboxColumnThread))
		theTxt = "???";

	// Draw the text
	if (!do_plot)
	{
		switch(col_info.column_type)
		{
		case eMboxColumnSmart:
			// Only draw icon if not fake
			if (!aMsg->IsFake())
			{
				// Check for smart address
				CIconLoader::DrawIcon(pDC, inLocalRect.left + 2, inLocalRect.top + mIconOrigin, smart_flag, 16);
			}

			// Advance over icon (even if not present)
			x += 12;
			break;
		case eMboxColumnSubject:
		case eMboxColumnThread:
		{
			// Check for thread
			if (depth)
			{
				{
					// Set up dotted pen - use text color if selected to get proper inversion
					CBrush brush(selected ? text_color : RGB(0x44, 0x44, 0x44));
					LOGBRUSH lb;
					brush.GetLogBrush(&lb);
					CPen temp;
					if (::GetVersion() < 0x80000000)
						temp.CreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lb);
					else
						temp.CreatePen(PS_SOLID, 0, selected ? text_color : RGB(0x44, 0x44, 0x44));
					CPen* old_pen = (CPen*) pDC->SelectObject(&temp);

					// Prevent drawing outside of cell area
					unsigned long max_depth_offset = inLocalRect.Width() - 48;

					// Draw right-angle line
					unsigned long depth_offset = (depth - 1) * 16;
					if (depth_offset < max_depth_offset)
					{
						pDC->MoveTo(inLocalRect.left + depth_offset + 8, inLocalRect.top);
						pDC->LineTo(inLocalRect.left + depth_offset + 8, (inLocalRect.top + inLocalRect.bottom)/2);
						pDC->LineTo(inLocalRect.left + depth_offset + 16, (inLocalRect.top + inLocalRect.bottom)/2);
					}

					// Check for parent/sibling lines
					const CMessage* parent = aMsg;
					while(parent)
					{
						// Draw vert line if the parent has a sibling after it (clip to cell width)
						if (parent->GetThreadNext() && (depth_offset < max_depth_offset))
						{
							pDC->MoveTo(inLocalRect.left + depth_offset + 8, inLocalRect.top);
							pDC->LineTo(inLocalRect.left + depth_offset + 8, inLocalRect.bottom);
						}
						
						// No more vert lines if depth offset at first 'column'
						if (!depth_offset)
							break;
							
						// Bump back to the next parent 'column'
						depth_offset -= 16;
						parent = parent->GetThreadParent();
					}						

					pDC->SelectObject(old_pen);
				}

				// Indent
				unsigned long text_offset = min((unsigned long)(inLocalRect.Width() - 32), 16 * depth);
				x += text_offset;
			}
			break;
		}
		default:;
		}

		// Set appropriate font & color
		pDC->SetTextColor(text_color);
		switch(text_style)
		{
		case normal:
		default:
			pDC->SelectObject(CFontCache::GetListFont());
			break;
		case bold:
			pDC->SelectObject(CFontCache::GetListFontBold());
			break;
		case italic:
			pDC->SelectObject(CFontCache::GetListFontItalic());
			break;
		case bold + italic:
			pDC->SelectObject(CFontCache::GetListFontBoldItalic());
			break;
		case underline:
			pDC->SelectObject(CFontCache::GetListFontUnderline());
			break;
		case bold + underline:
			pDC->SelectObject(CFontCache::GetListFontBoldUnderline());
			break;
		case italic + underline:
			pDC->SelectObject(CFontCache::GetListFontItalicUnderline());
			break;
		case bold + italic + underline:
			pDC->SelectObject(CFontCache::GetListFontBoldItalicUnderline());
			break;
		}

		// Draw text
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, right_just ? eDrawString_Right : eDrawString_Left);

		// Don't strike out fakes
		if (strike_through && !aMsg->IsFake())
		{
			CPen temp(PS_SOLID, 1, pDC->GetTextColor());
			CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
			pDC->MoveTo(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->SelectObject(old_pen);
		}
	}

	// Draw icon
	else
	{
		// Don't draw if its a fake
		if (!aMsg->IsFake())
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, plot_flag, 16);

		// Don't strike out fakes or flags
		if (strike_through && !aMsg->IsFake() && (col_info.column_type != eMboxColumnFlags))
		{
			CPen temp(PS_SOLID, 1, pDC->GetTextColor());
			CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
			pDC->MoveTo(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->SelectObject(old_pen);
		}
	}
}

void CMailboxTable::GetDrawStyle(const CMessage* aMsg, COLORREF& color, short& style, bool& strike) const
{
	bool deleted = false;
	if (aMsg->IsDeleted())
	{
		color = CPreferences::sPrefs->deleted.GetValue().color;
		style = CPreferences::sPrefs->deleted.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->deleted.GetValue().style & 0x0080) != 0);
		deleted = true;
	}
	else if (aMsg->IsFlagged())
	{
		color = CPreferences::sPrefs->important.GetValue().color;
		style = CPreferences::sPrefs->important.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->important.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsAnswered())
	{
		color = CPreferences::sPrefs->answered.GetValue().color;
		style = CPreferences::sPrefs->answered.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->answered.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsUnseen())
	{
		color = CPreferences::sPrefs->unseen.GetValue().color;
		style = CPreferences::sPrefs->unseen.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->unseen.GetValue().style & 0x0080) != 0);
	}
	else
	{
		// Set default seen style
		color = CPreferences::sPrefs->seen.GetValue().color;
		style = CPreferences::sPrefs->seen.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->seen.GetValue().style & 0x0080) != 0);
	}
	
	// Look for labels only if not deleted
	if (!deleted)
	{
		bool got_label = false;
		for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (aMsg->HasLabel(i))
			{
				// Only do coloring once
				if (!got_label && CPreferences::sPrefs->mLabels.GetValue()[i]->usecolor)
					color = CPreferences::sPrefs->mLabels.GetValue()[i]->color;
				style |= CPreferences::sPrefs->mLabels.GetValue()[i]->style & 0x007F;
				strike |= ((CPreferences::sPrefs->mLabels.GetValue()[i]->style & 0x0080) != 0);
				
				// The first label set always wins for colors
				got_label = true;
			}
		}
	}
}

int CMailboxTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return eDeleted_Flag;
	else if (aMsg->IsDraft())
		return eDraft_Flag;
	else if (aMsg->IsAnswered())
		return eAnswered_Flag;
	else if (aMsg->IsUnseen() && aMsg->IsRecent())
		return eRecent_Flag;
	else if (aMsg->IsUnseen())
		return eUnseen_Flag;
	else
		return eRead_Flag;
}

bool CMailboxTable::UsesBackground(const CMessage* aMsg) const
{
	// Must have mailbox
	if (!GetMbox())
		return false;

	// If not matching and background flag for matched/unmatched is on
	if (GetMbox()->GetViewMode() == NMbox::eViewMode_AllMatched)
	{
		if (aMsg->IsSearch() && CPreferences::sPrefs->mMatch.GetValue().style)
			return true;
		else if (!aMsg->IsSearch() && CPreferences::sPrefs->mNonMatch.GetValue().style)
			return true;
		
		// Fall through to look for labels
	}
	
	// Look for labels if not deleted
	if (!aMsg->IsDeleted())
	{
		for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (aMsg->HasLabel(i))
			{
				// The first label set always wins for colors
				return CPreferences::sPrefs->mLabels.GetValue()[i]->usebkgcolor;
			}
		}
	}

	return false;
}

COLORREF CMailboxTable::GetBackground(const CMessage* aMsg) const
{
	// Use background for matched/unmatched
	if (aMsg->IsSearch() && (GetMbox()->GetViewMode() == NMbox::eViewMode_AllMatched))
		return CPreferences::sPrefs->mMatch.GetValue().color;
	else
	{
		// Look for labels if not deleted
		if (!aMsg->IsDeleted())
		{
			for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
			{
				if (aMsg->HasLabel(i))
				{
					// The first label set always wins for colors
					if (CPreferences::sPrefs->mLabels.GetValue()[i]->usebkgcolor)
						return CPreferences::sPrefs->mLabels.GetValue()[i]->bkgcolor;
				}
			}
		}
		
		// Usse default n on matched colour	
		return CPreferences::sPrefs->mNonMatch.GetValue().color;
	}
}

// Get horiz scrollbar pos
void CMailboxTable::GetScrollPos(long& h, long& v) const
{
	h = CWnd::GetScrollPos(SB_HORZ);
	v = CWnd::GetScrollPos(SB_VERT);
}

// Get horiz scrollbar pos
void CMailboxTable::SetScrollPos(long h, long v)
{
	ScrollPinnedImageTo(h, v, true);
}

#pragma mark ____________________________________Drag&Drop

BOOL CMailboxTable::DoDrag(TableIndexT row)
{
	if (!GetMbox())
		return false;

	// Add allowed flavors to source
	for(int i = 0; i < mDragFlavors.GetSize(); i++)
	{
		unsigned int theFlavor = mDragFlavors[i];
	
		mDrag.DelayRenderData(theFlavor);
	}
	
	// Get row rect
	CRect rowRect;
	GetLocalRowRect(row, rowRect);

	// Check delete state here (must do test as selection change is deferred)
	bool delete_test = !TestSelectionIgnore1And(&CMailboxTable::TestSelectionFlag, NMessage::eDeleted);

	// Get list of selected rows before copy which may change them
	ulvector nums;
	GetSelectedRows(nums);

	// Must use unsorted numbers between two IMAP commands
	ulvector actual_nums;
	GetMbox()->MapSorted(actual_nums, nums, true);

	// Need to preserve message selection as Copy operation may result in table reset
	ulvector preserve;
	for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		preserve.push_back(reinterpret_cast<unsigned long>(GetMbox()->GetMessage(*iter)));

	// Reset table reset flag
	mResetTable = false;

	// Set flag so we know if messages were dropped on another mailbox
	sDropOnMailbox = false;

	sTableDragSource = this;
	sTableDropTarget = NULL;
	CTableDropSource* pDropSource = new CTableDropSource;
	DROPEFFECT de = mDrag.DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_SCROLL, rowRect, pDropSource);
	delete pDropSource;
	sTableDragSource = NULL;

	// Handle move operation
	if (de == DROPEFFECT_MOVE)
	{
		// If drag OK and drop on another mailbox then delete selection if required and if not all already deleted
		if (sDropOnMailbox && CPreferences::sPrefs->deleteAfterCopy.GetValue() && delete_test &&
			GetMbox()->HasAllowedFlag(NMessage::eDeleted))
		{
			// Look for possible table reset and redo message number array
			if (mResetTable)
			{
				// Redo message numbers for delete operation
				actual_nums.clear();
				for(ulvector::const_iterator iter = preserve.begin(); iter != preserve.end(); iter++)
				{
					// See if message still exists and if so where
					unsigned long index = GetMbox()->GetMessageIndex(reinterpret_cast<CMessage*>(*iter));
					if (index)
						actual_nums.push_back(index);	
				}
			}
			
			// Set deleted flag on chosen messages
			GetMbox()->SetFlagMessage(actual_nums, false, NMessage::eDeleted, true, false);
		}
	}
	
	return (de != DROPEFFECT_NONE);
}

// Add selected mboxes to list
bool CMailboxTable::AddSelectionToDrag(TableIndexT row, CMessageList* list)
{
	if (!GetMbox())
		return false;

	// Determine message
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg || theMsg->IsFake())
		return false;

	list->push_back(theMsg);
	return true;
}

BOOL CMailboxTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	// Make list of selected messages
	CMessageList msgs;
	DoToSelection1((DoToSelection1PP) &CMailboxTable::AddSelectionToDrag, &msgs);

	if (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMsgList)
	{
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, msgs.size() * sizeof(CMessage*) + sizeof(int));
		if (*phGlobal)
		{
			// Copy to global after lock
			CMessage** pAddr = (CMessage**) ::GlobalLock(*phGlobal);
			*((int*) pAddr) = msgs.size();
			pAddr += sizeof(int);
			for(CMessageList::iterator iter = msgs.begin(); iter != msgs.end(); iter++)
				*pAddr++ = *iter;
			//::memcpy(pAddr, msgs.begin(), msgs.size() * sizeof(CMessage*));
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}
	else if (lpFormatEtc->cfFormat == CF_UNICODETEXT)
	{
		// Add text of each message
		cdstring txt;
		for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
		{
			// Check message size first
			if (!CMailControl::CheckSizeWarning(*iter))
				return false;

			// Read in message first
			CMailControl::BlockBusy(true);
			const char* data = NULL;
			try
			{
				data = (*iter)->ReadPart();
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				CMailControl::BlockBusy(false);
				
				// Throw out of here as it is fatal
				CLOG_LOGRETHROW;
				throw;
			}
			CMailControl::BlockBusy(false);
			
			// Add header to stream if required
			if (CPreferences::sPrefs->saveMessageHeader.GetValue())
				txt += (*iter)->GetHeader();

			// Add message text to stream (include terminator)
			if (data)
				txt += data;

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
	}

	// Do not delete originals
	msgs.clear();
	
	return rendered;
}

// Determine effect
DROPEFFECT CMailboxTable::GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	DROPEFFECT de;

	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		// Toggle based on modifiers
		bool option_key = ((dwKeyState & MK_ALT) == MK_ALT);

		if (CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key)
			de = DROPEFFECT_MOVE;
		else
			de = DROPEFFECT_COPY;
	}
	else if ((theFlavor == CMulberryApp::sFlavorMboxList) ||
				(theFlavor == CMulberryApp::sFlavorMboxRefList))
	{
		// Mailbox drop is treated as a move
		de = DROPEFFECT_MOVE;
	}
	else
		// Anything else is an error!
		de = DROPEFFECT_MOVE;

	return de;
}

// Drop data into table
bool CMailboxTable::DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size)
{
	if (!GetMbox())
		return false;

	// Set flag
	sDropOnMailbox = true;

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		// Get list of chosen message nums
		ulvector nums;
		CMessage* theMsg = NULL;
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			theMsg = ((CMessage**) drag_data)[i];

			// Do not allow copy to same mailbox
			nums.push_back(theMsg->GetMessageNumber());
		}

		// Do mail message copy from mbox in drag to this mbox
		if (theMsg && (nums.size() > 0))
		{
			try
			{
				ulmap temp;
				theMsg->GetMbox()->CopyMessage(nums, false, GetMbox(), temp);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Make sure not signalled as mailbox to prevent delete after copy
				sDropOnMailbox = false;
				
				CLOG_LOGRETHROW;
				throw;
			}
		}

		// Force reset of table due to incoming
		ResetTable();
	}
	else if (theFlavor == CMulberryApp::sFlavorMboxList)
	{
		// Only handle the first one in the list
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		if (count)
		{
			CMbox* mbox = ((CMbox**) drag_data)[0];
			if (mbox)
				mTableView->Recycle(mbox);
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMboxRefList)
	{
		// Only handle the first one in the list
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		if (count)
		{
			CMboxRef* mboxref = ((CMboxRef**) drag_data)[0];
			CMbox* mbox = mboxref->ResolveMbox();
			if (mbox)
				mTableView->Recycle(mbox);
		}
	}
		
	return true;
}
