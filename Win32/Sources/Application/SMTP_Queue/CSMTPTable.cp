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


// Source for CSMTPTable class


#include "CSMTPTable.h"

#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPFrame.h"
#include "CSMTPWindow.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSMTPTable

IMPLEMENT_DYNCREATE(CSMTPTable, CTableDragAndDrop)

BEGIN_MESSAGE_MAP(CSMTPTable, CMailboxTable)
	ON_WM_CREATE()

	ON_COMMAND(IDM_MESSAGES_DELETE, OnSMTPDelete)
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARMESSAGEDELETEBTN, OnSMTPDelete)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSMTPHOLDBTN, OnUpdateSMTPHoldMessage)
	ON_COMMAND(IDC_TOOLBARSMTPHOLDBTN, OnSMTPHoldMessage)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSMTPPRIORITYBTN, OnUpdateSMTPPriority)
	ON_COMMAND(IDC_TOOLBARSMTPPRIORITYBTN, OnSMTPPriority)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSMTPACCOUNTSPOPUP, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_SMTP_ACCOUNTS_Start, IDM_SMTP_ACCOUNTS_Stop, OnAccountsPopup)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSMTPENABLEBTN, OnUpdateSMTPEnable)
	ON_COMMAND(IDC_TOOLBARSMTPENABLEBTN, OnSMTPEnableQueue)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPTable::CSMTPTable()
{
	mTestSelectionAndHold = false;
	mTestSelectionAndPriority = false;
}

// Default destructor
CSMTPTable::~CSMTPTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CSMTPTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxTable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sFlavorMsgList);
	AddDragFlavor(CF_UNICODETEXT);

	SetReadOnly(true);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);

	mSMTPView = dynamic_cast<CSMTPView*>(mTableView);

	return 0;
}

#pragma mark ____________________________Drawing

void CSMTPTable::GetDrawStyle(const CMessage* aMsg, COLORREF& color, short& style, bool& strike) const
{
	if (aMsg->IsDeleted())	// Deleted
	{
		color = CPreferences::sPrefs->deleted.GetValue().color;
		style = CPreferences::sPrefs->deleted.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->deleted.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsPriority())	// High priority send
	{
		color = CPreferences::sPrefs->important.GetValue().color;
		style = CPreferences::sPrefs->important.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->important.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsSendNow())	// Being sent
	{
		color = CPreferences::sPrefs->answered.GetValue().color;
		style = CPreferences::sPrefs->answered.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->answered.GetValue().style & 0x0080) != 0);
	}
	else if (!aMsg->IsHold())	// Not held
	{
		color = CPreferences::sPrefs->unseen.GetValue().color;
		style = CPreferences::sPrefs->unseen.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->unseen.GetValue().style & 0x0080) != 0);
	}
	else	// Held
	{
		color = CPreferences::sPrefs->seen.GetValue().color;
		style = CPreferences::sPrefs->seen.GetValue().style & 0x007F;
		strike = ((CPreferences::sPrefs->seen.GetValue().style & 0x0080) != 0);
	}
}

int CSMTPTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return eDeleted_Flag;
	else if (aMsg->IsHold())
		return aMsg->IsSendError() ? eSendError_Flag : ePaused_Flag;
	else if (aMsg->IsSendNow())
		return eSending_Flag;
	else
		return eDraft_Flag;
}

bool CSMTPTable::UsesBackground(const CMessage* aMsg) const
{
	// Answered means 'in process of being sent'
	return aMsg->IsSendNow();
}

COLORREF CSMTPTable::GetBackground(const CMessage* aMsg) const
{
	// Answered means 'in process of being sent'
	return aMsg->IsSendNow() ?
				CPreferences::sPrefs->mMatch.GetValue().color :
				CPreferences::sPrefs->mNonMatch.GetValue().color;
}

#pragma mark ____________________________Command Updating

void CSMTPTable::OnUpdateSMTPHoldMessage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectionValid());
	pCmdUI->SetCheck(mTestSelectionAndHold);
}

void CSMTPTable::OnUpdateSMTPPriority(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectionValid());
	pCmdUI->SetCheck(mTestSelectionAndPriority);
}

void CSMTPTable::OnUpdateSMTPEnable(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CConnectionManager::sConnectionManager.IsConnected());
	
	// In disconnected mode it must not be checked
	bool checked = CConnectionManager::sConnectionManager.IsConnected() &&
					CSMTPAccountManager::sSMTPAccountManager && CSMTPAccountManager::sSMTPAccountManager->GetConnected();
	pCmdUI->SetCheck(checked);
	CString title;
	title.LoadString(checked ? IDS_DISABLE_CMD_TEXT : IDS_ENABLE_CMD_TEXT);
	pCmdUI->SetText(title);
}

#pragma mark ____________________________Display Updating

// Update captions & buttons
BOOL CSMTPTable::RedrawWindow(LPCRECT lpRectUpdate, CRgn* prgnUpdate, UINT flags)
{
	// Update buttons then do inherited
	UpdateItems();

	return CWnd::RedrawWindow(lpRectUpdate, prgnUpdate, flags);
}

// Reset button & caption state as well
void CSMTPTable::RefreshSelection(void)
{
	// Update buttons then do inherited
	UpdateItems();

	CMailboxTable::RefreshSelection();

} // CSMTPTable::RefreshSelection

// Reset button & caption state as well
void CSMTPTable::RefreshRow(TableIndexT row)
{
	// Update buttons then do inherited
	UpdateItems();

	CMailboxTable::RefreshRow(row);
}

void CSMTPTable::DoSelectionChanged(void)
{
	// Get these setup before doing inherited call which will update toolbar
	mTestSelectionAndHold = TestSelectionIgnore1And(&CSMTPTable::TestSelectionFlag, NMessage::eMDNSent);
	mTestSelectionAndPriority = TestSelectionIgnore1And(&CSMTPTable::TestSelectionFlag, NMessage::eFlagged);

	CMailboxTable::DoSelectionChanged();
	
	// Determine whether preview is triggered
	const CUserAction& preview = mSMTPView->GetPreviewAction();
	if (preview.GetSelection())
		PreviewMessage();

	// Determine whether full view is triggered
	const CUserAction& fullview = mSMTPView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFileOpenMessage(false);
}

// Update after possible change
void CSMTPTable::UpdateItems(void)
{
	// Don't update items until fully open
	if (mTableView->IsOpening())
		return;

	// Update buttons then do inherited
	if (GetMbox() && !mUpdating)
		UpdateState();
	if (!mUpdating)
		UpdateCaptions();
}

// Update delete buttons
void CSMTPTable::UpdateState(void)
{
	// Broadcast a selection change which will force an update
	Broadcast_Message(eBroadcast_SelectionChanged, this);
	
	// Change window title as well
	static_cast<CSMTPWindow*>(mTableView->GetMailboxWindow())->UpdateTitle();
}

// Update captions
void CSMTPTable::UpdateCaptions(void)
{
	cdstring	aStr;
	if (GetMbox())
		aStr = GetMbox()->GetNumberFound();
	else
		aStr = "--";
	CUnicodeUtils::SetWindowTextUTF8(&static_cast<CSMTPView*>(mTableView)->mTotalText, aStr);
}

// Update captions
void CSMTPTable::OnAccountsPopup(UINT nID)
{
	unsigned long pos = nID - IDM_SMTP_ACCOUNTS_Start;
	//mSMTPView->mAccountsPopup->SetValue(nID);
	OnSMTPAccountPopup(pos);
}

