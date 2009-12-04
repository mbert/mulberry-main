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

#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPView.h"
#include "CSMTPWindow.h"
#include "CStaticText.h"

#include "CResources.h"

// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPTable::CSMTPTable()
{
	InitSMTPTable();
}

// Default constructor - just do parents' call
CSMTPTable::CSMTPTable(LStream *inStream)
		: CMailboxTable(inStream)
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
void CSMTPTable::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxTable::FinishCreateSelf();

	// Find table view in super view chain
	mSMTPView = dynamic_cast<CSMTPView*>(mTableView);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);
	AddDragFlavor(cDragMsgType);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(true);
}

// Respond to commands
Boolean CSMTPTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ToolbarSMTPHoldBtn:
		OnSMTPHoldMessage();
		break;

	case cmd_ToolbarSMTPPriorityBtn:
		OnSMTPPriority();
		break;

	case cmd_DeleteMessage:
	case cmd_ToolbarMessageDeleteBtn:
		OnSMTPDelete();
		break;

	case cmd_ToolbarSMTPAccountsPopup:
		OnSMTPAccountPopup((*(long*) ioParam) - 1);
		break;

	case cmd_ToolbarSMTPEnableBtn:
		OnSMTPEnableQueue();
		break;

	default:
		cmdHandled = CMailboxTable::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CSMTPTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_ToolbarSMTPHoldBtn:
		outEnabled = mIsSelectionValid;
		outUsesMark = true;
		outMark = mTestSelectionAndHold ? (UInt16)checkMark : (UInt16)noMark;
		break;
	case cmd_ToolbarSMTPPriorityBtn:
		outEnabled = mIsSelectionValid;
		outUsesMark = true;
		outMark = mTestSelectionAndPriority ? (UInt16)checkMark : (UInt16)noMark;
		break;
	case cmd_ToolbarSMTPEnableBtn:
		outEnabled = CConnectionManager::sConnectionManager.IsConnected();
		outUsesMark = true;

		// In disconnected mode it must not be checked
		bool checked = CConnectionManager::sConnectionManager.IsConnected() &&
						CSMTPAccountManager::sSMTPAccountManager && CSMTPAccountManager::sSMTPAccountManager->GetConnected();
		outMark = checked ? (UInt16)checkMark : (UInt16)noMark;
		{
			LStr255 txt(STRx_Standards, checked ? str_Disable : str_Enable);
			::PLstrcpy(outName, txt);
		}
		break;

	default:
		CMailboxTable::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

#pragma mark ____________________________Drawing

void CSMTPTable::GetDrawStyle(const CMessage* aMsg, RGBColor& text_color, Style& text_style, bool& strikeThrough) const
{
	if (aMsg->IsDeleted())	// Deleted
	{
		text_color = CPreferences::sPrefs->deleted.GetValue().color;
		text_style = CPreferences::sPrefs->deleted.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->deleted.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsPriority())	// High priority send
	{
		text_color = CPreferences::sPrefs->important.GetValue().color;
		text_style = CPreferences::sPrefs->important.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->important.GetValue().style & 0x0080) != 0);
	}
	else if (aMsg->IsSendNow())	// Being sent
	{
		text_color = CPreferences::sPrefs->answered.GetValue().color;
		text_style = CPreferences::sPrefs->answered.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->answered.GetValue().style & 0x0080) != 0);
	}
	else if (!aMsg->IsHold())		// Not held
	{
		text_color = CPreferences::sPrefs->unseen.GetValue().color;
		text_style = CPreferences::sPrefs->unseen.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->unseen.GetValue().style & 0x0080) != 0);
	}
	else	// Held
	{
		text_color = CPreferences::sPrefs->seen.GetValue().color;
		text_style = CPreferences::sPrefs->seen.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->seen.GetValue().style & 0x0080) != 0);
	}
}

ResIDT CSMTPTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return ICNx_Deleted_Flag;
	else if (aMsg->IsHold())
		return aMsg->IsSendError() ? ICNx_SendError_Flag : ICNx_Paused_Flag;
	else if (aMsg->IsSendNow())
		return ICNx_Sending_Flag;
	else
		return ICNx_Draft_Flag;	// Always drafts
}

bool CSMTPTable::UsesBackground(const CMessage* aMsg) const
{
	// Answered means 'in process of being sent'
	return aMsg->IsSendNow();
}

RGBColor CSMTPTable::GetBackground(const CMessage* aMsg) const
{
	// Answered means 'in process of being sent'
	return aMsg->IsSendNow() ?
				CPreferences::sPrefs->mMatch.GetValue().color :
				CPreferences::sPrefs->mNonMatch.GetValue().color;
}


#pragma mark ____________________________Display updating

// Reset button & caption state as well
void CSMTPTable::Refresh()
{
	// Update buttons then do inherited
	UpdateItems();
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
void CSMTPTable::RefreshRow(const TableIndexT aRow)
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
	cdstring aStr;
	if (GetMbox())
		aStr = GetMbox()->GetNumberFound();
	else
		aStr = "--";
	static_cast<CSMTPView*>(mTableView)->mTotal->SetText(aStr);
}
