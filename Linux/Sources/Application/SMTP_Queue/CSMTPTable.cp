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

#include "CCommands.h"
#include "CConnectionManager.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPView.h"

#include <JXStaticText.h>

// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPTable::CSMTPTable(JXScrollbarSet* scrollbarSet,
						 JXContainer* enclosure,
						 const HSizingOption hSizing,
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h)
	: CMailboxTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	InitSMTPTable();
}

// Default destructor
CSMTPTable::~CSMTPTable()
{
}

// Do common init
void CSMTPTable::InitSMTPTable(void)
{
	mTestSelectionAndHold = false;
	mTestSelectionAndPriority = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSMTPTable::OnCreate()
{
	// Do inherited
	CMailboxTable::OnCreate();

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sApp->sFlavorMsgList);

	// Set read only status of Drag and Drop
	SetReadOnly(true);

	mSMTPView = dynamic_cast<CSMTPView*>(mTableView);

	// Context menu
	CreateContextMenu(CMainMenu::eContextMailboxTable);
}

//	Pass back status of a (menu) command
void CSMTPTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eToolbarSMTPHoldBtn:
		OnUpdateSMTPHoldMessage(cmdui);
		return;

	case CCommand::eToolbarSMTPPriorityBtn:
		OnUpdateSMTPPriority(cmdui);
		return;

	case CCommand::eToolbarSMTPEnableBtn:
		OnUpdateSMTPEnable(cmdui);
		return;

	default:;
	}

	CMailboxTable::UpdateCommand(cmd, cmdui);
}

// Respond to commands
bool CSMTPTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eToolbarSMTPHoldBtn:
		OnSMTPHoldMessage();
		return true;

	case CCommand::eToolbarSMTPPriorityBtn:
		OnSMTPPriority();
		return true;

	case CCommand::eMessagesDelete:
	case CCommand::eToolbarMessageDeleteBtn:
		OnSMTPDelete();
		return true;

	case CCommand::eToolbarSMTPAccountsPopup:
		OnSMTPAccountPopup(menu->mIndex - 1);
		return true;

	case CCommand::eToolbarSMTPEnableBtn:
		OnSMTPEnableQueue();
		return true;

	default:;
	}

	return CMailboxTable::ObeyCommand(cmd, menu);
}

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
	// Disable if in disconnected mode
	pCmdUI->Enable(CConnectionManager::sConnectionManager.IsConnected());
	
	// In disconnected mode it must not be checked
	bool checked = CConnectionManager::sConnectionManager.IsConnected() &&
					CSMTPAccountManager::sSMTPAccountManager && CSMTPAccountManager::sSMTPAccountManager->GetConnected();
	pCmdUI->SetCheck(checked);
	cdstring title;
	title.FromResource(checked ? IDS_DISABLE_CMD_TEXT : IDS_ENABLE_CMD_TEXT);
	pCmdUI->SetText(title);
}

#pragma mark ____________________________Drawing

void CSMTPTable::GetDrawStyle(const CMessage* aMsg, JRGB& text_color, short& text_style) const
{
	if (aMsg->IsDeleted())	// Deleted
	{
		text_color = CPreferences::sPrefs->deleted.GetValue().color;
		text_style = CPreferences::sPrefs->deleted.GetValue().style;
	}
	else if (aMsg->IsPriority())	// High priority send
	{
		text_color = CPreferences::sPrefs->important.GetValue().color;
		text_style = CPreferences::sPrefs->important.GetValue().style;
	}
	else if (aMsg->IsSendNow())	// Being sent
	{
		text_color = CPreferences::sPrefs->answered.GetValue().color;
		text_style = CPreferences::sPrefs->answered.GetValue().style;
	}
	else if (!aMsg->IsHold())		// Not held
	{
		text_color = CPreferences::sPrefs->unseen.GetValue().color;
		text_style = CPreferences::sPrefs->unseen.GetValue().style;
	}
	else	// Held
	{
		text_color = CPreferences::sPrefs->seen.GetValue().color;
		text_style = CPreferences::sPrefs->seen.GetValue().style;
	}
}

long CSMTPTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return eDeleted_Flag;
	else if (aMsg->IsHold())
		return aMsg->IsSendError() ? eSendError_Flag : ePaused_Flag;
	else if (aMsg->IsSendNow())
		return eSending_Flag;
	else
		return eDraft_Flag;	// Always drafts
}

bool CSMTPTable::UsesBackground(const CMessage* aMsg) const
{
	// Answered means 'in process of being sent'
	return aMsg->IsSendNow();
}

JColorIndex CSMTPTable::GetBackground(const CMessage* aMsg) const
{
	JRGB theColor;
	// Answered means 'in process of being sent'
	theColor = aMsg->IsSendNow() ?
				CPreferences::sPrefs->mMatch.GetValue().color :
				CPreferences::sPrefs->mNonMatch.GetValue().color;
	return sColorList->Add(theColor);
}


#pragma mark ____________________________Display updating

// Reset button & caption state as well
void CSMTPTable::Refresh() const
{
	// Update buttons then do inherited
	const_cast<CSMTPTable*>(this)->UpdateItems();
	CMailboxTable::Refresh();

} // CSMTPTable::Refresh

// Reset button & caption state as well
void CSMTPTable::RefreshSelection()
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::RefreshSelection();

} // CSMTPTable::RefreshSelection

// Reset button & caption state as well
void CSMTPTable::RefreshRow(TableIndexT aRow)
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::RefreshRow(aRow);
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

// Update enable button
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
	char aStr[32];
	if (GetMbox())
		::snprintf(aStr, 32, "%ld", GetMbox()->GetNumberFound());
	else
		::snprintf(aStr, 32, "--");
	mSMTPView->mTotal->SetText(aStr);
}
