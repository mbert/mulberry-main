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


// Source for CMailboxInfoTable class

#include "CMailboxInfoTable.h"

#include "CAddress.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CMailControl.h"
#include "CMailboxTitleTable.h"
#include "CMailboxInfoWindow.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionClick.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CStaticText.h"
#include "CUserAction.h"

#include <LBevelButton.h>

// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoTable::CMailboxInfoTable()
{
	InitInfoTable();
}

// Default constructor - just do parents' call
CMailboxInfoTable::CMailboxInfoTable(LStream *inStream)
		: CMailboxTable(inStream)
{
	InitInfoTable();
}

// Default destructor
CMailboxInfoTable::~CMailboxInfoTable()
{
}

// Do common init
void CMailboxInfoTable::InitInfoTable(void)
{
	mSearchResults = false;
	mLastTyping = 0;
	mLastChar = 0;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxInfoTable::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxTable::FinishCreateSelf();

	// Find table view in super view chain
	mInfoTableView = dynamic_cast<CMailboxInfoView*>(mTableView);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);
	AddDragFlavor(cDragMsgType);
	AddDropFlavor(cDragMsgType);
	AddDropFlavor(cDragMboxType);
	AddDropFlavor(cDragMboxRefType);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);
}

// Handle key presses
Boolean CMailboxInfoTable::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{
	// Get state of command modifiers (not shift)
	bool mods = ((inKeyEvent.modifiers & (optionKey | cmdKey | controlKey)) != 0);

	// Look for mode switch
	bool do_mode = false;
	if (mLastTyping != 0)
	{
		unsigned long new_time = TickCount();
		do_mode = (new_time - mLastTyping <= GetDblTime());
	}

	CommandT cmd = 0;
	
	// Look for mode switch
	if (do_mode)
	{
		switch(mLastChar)
		{
		// Thread aggregate mode
		case 't':
			// Must be in thread mode
			if (GetMbox() && (GetMbox()->GetSortBy() == cSortMessageThread))
			{
				switch (inKeyEvent.message & charCodeMask)
				{
				case 'a':
				case 'A':
					cmd = cmd_ThreadSelectAll;
					break;
				case 's':
				case 'S':
					cmd = cmd_ThreadMarkSeen;
					break;
				case 'i':
				case 'I':
					cmd = cmd_ThreadMarkImportant;
					break;
				case 'd':
				case 'D':
					cmd = cmd_ThreadMarkDeleted;
					break;
				case 'k':
				case 'K':
					cmd = cmd_ThreadSkip;
					break;
				default:
					// Beep for illegal mode character
					::SysBeep(1);
					break;
				}
			}
			else
				// Beep for illegal mode
				::SysBeep(1);
			break;
		// Match aggregate mode
		case 'u':
			switch (inKeyEvent.message & charCodeMask)
			{
			case 's':
			case 'S':
				cmd = cmd_MatchUnseen;
				break;
			case 'i':
			case 'I':
				cmd = cmd_MatchImportant;
				break;
			case 'd':
			case 'D':
				cmd = cmd_MatchDeleted;
				break;
			case 't':
			case 'T':
				cmd = cmd_MatchSentToday;
				break;
			case 'f':
			case 'F':
				cmd = cmd_MatchFrom;
				break;
			case 'j':
			case 'J':
				cmd = cmd_MatchSubject;
				break;
			case 'a':
			case 'A':
				cmd = cmd_MatchDate;
				break;
			case 'u':
			case 'U':
				// Toggle toolbar button state to turn off/on match
				if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch))
					cmd = cmd_ToolbarMailboxMatchBtn;
				else
					cmd = cmd_ToolbarMailboxMatchOptionBtn;
				break;
			default:
				// Beep for illegal mode character
				::SysBeep(1);
				break;
			}
			break;
		}

		// Always zero out mode detection
		mLastTyping = 0;
		mLastChar = 0;
	}
	else
	{
		// Always zero out mode detection
		mLastTyping = 0;
		mLastChar = 0;

		switch (inKeyEvent.message & charCodeMask)
		{
		// Select unseen
		case char_Tab:
			// Look for option key
			if ((inKeyEvent.modifiers & optionKey) != 0)
				DoRollover(false);
			else
				SelectNextNew((inKeyEvent.modifiers & shiftKey) != 0);
			return true;

		case '[':
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && mTableView->GetUseSubstitute())
			{
				CycleTabs(false);
				return true;
			}
			break;
		
		case ']':
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && mTableView->GetUseSubstitute())
			{
				CycleTabs(true);
				return true;
			}
			break;

		// Key shortcuts
		case 'a':
		case 'A':
			cmd = cmd_SelectAll;
			break;

		case 'd':
		case 'D':
			cmd = cmd_FlagsDeleted;
			break;

		case 's':
		case 'S':
			cmd = cmd_FlagsSeen;
			break;

		case 'i':
		case 'I':
			cmd = cmd_FlagsImportant;
			break;

		case 'e':
		case 'E':
			cmd = cmd_ExpungeMailbox;
			break;

		case 'l':
		case 'L':
			cmd = cmd_CheckMail;
			break;

		case 'y':
		case 'Y':
			cmd = cmd_SearchMailbox;
			break;

		case 'r':
		case 'R':
			cmd = cmd_ReplyMessage;
			break;

		case 'f':
		case 'F':
			cmd = cmd_ForwardMessage;
			break;

		case 'b':
		case 'B':
			cmd = cmd_BounceMessage;
			break;

		case 'c':
		case 'C':
			cmd = cmd_MessageCopyExplicit;
			break;

		case 'm':
		case 'M':
			cmd = cmd_MessageMoveExplicit;
			break;

		case 'n':
		case 'N':
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				SelectNextNew(false);
				return true;
			}
			break;

		case 'p':
		case 'P':
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				SelectNextNew(true);
				return true;
			}
			break;

		case 'w':
		case 'W':
			// Only close tabs in 3-pane
			if (mInfoTableView->Is3Pane())
				cmd = cmd_Close;
			break;

		case 'z':
		case 'Z':
			cmd = cmd_ToolbarZoomItems;
			break;

		case 't':
		case 'T':
			// Thread mode switch
			mLastTyping = TickCount();
			mLastChar = 't';
			break;

		case 'u':
		case 'U':
			// Match mode switch
			mLastTyping = TickCount();
			mLastChar = 'u';
			break;

		default:;
		}
	}

	// Check for command execute
	if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods && (cmd != 0))
	{
		// Check whether command is valid right now
		Boolean outEnabled;
		Boolean outUsesMark;
		UInt16 outMark;
		Str255 outName;
		FindCommandStatus(cmd, outEnabled, outUsesMark, outMark, outName);
		
		// Execute if enabled
		if (outEnabled)
			ObeyCommand(cmd, NULL);

		return true;
	}

	// Do defauly key press processing
	return CMailboxTable::HandleKeyPress(inKeyEvent);
}

// Respond to commands
Boolean CMailboxInfoTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	ResIDT	menuID;
	SInt16	menuItem;
	bool	cmdHandled = true;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Only do rules if not locked out
		if (CApplyRulesMenu::IsApplyRulesMenu(menuID) && !CAdminLock::sAdminLock.mNoRules)
		{
			DoApplyRuleMailbox(menuItem == CApplyRulesMenu::eApply_All ? -1 : menuItem - CApplyRulesMenu::eApply_First);
			cmdHandled = true;
		}
		else
			cmdHandled = CMailboxTable::ObeyCommand(inCommand, ioParam);
	}
	else
	{
		switch (inCommand)
		{
		case cmd_NewLetter:
			DoNewLetter(COptionMenu::sOptionKey);
			break;
		case cmd_ToolbarNewLetter:
			DoNewLetter(false);
			break;
		case cmd_ToolbarNewLetterOption:
			DoNewLetter(true);
			break;

		case cmd_Close:
			OnFileClose();
			break;

		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
			OnMailboxProperties();
			break;

		case cmd_PuntUnseen:
			OnMailboxPunt();
			break;

		case cmd_SynchroniseMailbox:
			OnMailboxSynchronise();
			break;

		case cmd_ClearDisconnectMailbox:
			OnMailboxClearDisconnect();
			break;

		case cmd_SubscribeMailbox:
			OnMailboxSubscribe();
			break;

		case cmd_UnsubscribeMailbox:
			OnMailboxUnsubscribe();
			break;

		case cmd_CheckMail:
		case cmd_ToolbarCheckMailboxBtn:
			OnMailboxCheck();
			break;

		case cmd_GotoMessage:
			OnMailboxGotoMessage();
			break;

		case cmd_CacheAll:
			OnMailboxCacheAll();
			break;

		case cmd_UpdateMatch:
			OnMailboxUpdateMatch();
			break;

		case cmd_SearchMailbox:
		case cmd_ToolbarSearchMailboxBtn:
			OnSearchMailbox();
			break;

		case cmd_SearchAgainMailbox:
			OnSearchAgainMailbox();
			break;

		case cmd_NextSearchMailbox:
			OnNextSearchMailbox();
			break;

		case cmd_ToolbarMailboxApplyRules:
			OnApplyRuleMailboxToolbar(static_cast<const char*>(ioParam));
			break;

		case cmd_MakeRuleMailbox:
			OnMakeRuleMailbox();
			break;

		case cmd_ToolbarMailboxMatchBtn:
			OnMatchButton(false);
			break;

		case cmd_ToolbarMailboxMatchOptionBtn:
			OnMatchButton(true);
			break;

		case cmd_ToolbarMailboxSelectPopup:
			OnSelectPopup(*(long*) ioParam);
			break;

		case cmd_ReplyMessage:
			DoMessageReply(replyReplyTo, COptionMenu::sOptionKey);
			break;
		case cmd_ToolbarMessageReplyBtn:
		case cmd_ToolbarMessageReplyOptionBtn:
			// Route this through the preview if its available and displaying a message and this table is not the target
			// NB This will only happen for toolbar clicks as menu commands will be routed
			// to the preview when its focussed.
			if (!IsTarget() && mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview() && mInfoTableView->GetPreview()->GetMessage())
				mInfoTableView->GetPreview()->ObeyCommand(inCommand, ioParam);
			else
				DoMessageReply(replyReplyTo, inCommand == cmd_ToolbarMessageReplyOptionBtn);
			break;

		case cmd_ReplyToSenderMessage:
			DoMessageReply(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ReplyToFromMessage:
			DoMessageReply(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ReplyToAllMessage:
		case cmd_ToolbarMessageReplyAllBtn:
			DoMessageReply(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ForwardMessage:
			DoMessageForward(COptionMenu::sOptionKey);
			break;
		case cmd_ToolbarMessageForwardBtn:
		case cmd_ToolbarMessageForwardOptionBtn:
			// Route this through the preview if its available and displaying a message
			// NB This will only happen for toolbar clicks as menu commands will be routed
			// to the preview when its focussed.
			if (mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview() && mInfoTableView->GetPreview()->GetMessage())
				mInfoTableView->GetPreview()->ObeyCommand(inCommand, ioParam);
			else
				DoMessageForward(inCommand == cmd_ToolbarMessageForwardOptionBtn);
			break;

		case cmd_BounceMessage:
		case cmd_ToolbarMessageBounceBtn:
			OnMessageBounce();
			break;

		case cmd_RejectMessage:
		case cmd_ToolbarMessageRejectBtn:
			OnMessageReject();
			break;

		case cmd_SendAgainMessage:
		case cmd_ToolbarMessageSendAgainBtn:
			OnMessageSendAgain();
			break;

		case cmd_CreateDigestMessage:
		case cmd_ToolbarMessageDigestBtn:
			OnMessageCreateDigest();
			break;

		case cmd_FlagsDeleted:
			DoFlagMailMessage(NMessage::eDeleted);
			break;

		case cmd_FlagsSeen:
			DoFlagMailMessage(NMessage::eSeen);
			break;

		case cmd_FlagsAnswered:
			DoFlagMailMessage(NMessage::eAnswered);
			break;

		case cmd_FlagsImportant:
			DoFlagMailMessage(NMessage::eFlagged);
			break;

		case cmd_FlagsDraft:
			DoFlagMailMessage(NMessage::eDraft);
			break;

		case cmd_FlagsLabel1:
		case cmd_FlagsLabel2:
		case cmd_FlagsLabel3:
		case cmd_FlagsLabel4:
		case cmd_FlagsLabel5:
		case cmd_FlagsLabel6:
		case cmd_FlagsLabel7:
		case cmd_FlagsLabel8:
			DoFlagMailMessage(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (inCommand - cmd_FlagsLabel1)));
			break;

		case cmd_ToolbarMessageFlagsBtn:
			switch(*((SInt32*) ioParam))
			{
			case 1:
				DoFlagMailMessage(NMessage::eSeen);
				break;
			case 2:
				DoFlagMailMessage(NMessage::eFlagged);
				break;
			case 3:
				DoFlagMailMessage(NMessage::eAnswered);
				break;
			case 4:
				DoFlagMailMessage(NMessage::eDeleted);
				break;
			case 5:
				DoFlagMailMessage(NMessage::eDraft);
				break;
			default:
				DoFlagMailMessage(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (*((SInt32*) ioParam) - 7)));
				break;
			}
			break;

		// Special mode commands
		case cmd_ThreadSelectAll:
			OnThreadSelectAll();
			break;
		case cmd_ThreadMarkSeen:
			OnThreadMarkSeen();
			break;
		case cmd_ThreadMarkImportant:
			OnThreadMarkImportant();
			break;
		case cmd_ThreadMarkDeleted:
			OnThreadMarkDeleted();
			break;
		case cmd_ThreadSkip:
			OnThreadSkip();
			break;

		case cmd_MatchUnseen:
			OnMatchUnseen();
			break;
		case cmd_MatchImportant:
			OnMatchImportant();
			break;
		case cmd_MatchDeleted:
			OnMatchDeleted();
			break;
		case cmd_MatchSentToday:
			OnMatchSentToday();
			break;
		case cmd_MatchFrom:
			OnMatchFrom();
			break;
		case cmd_MatchSubject:
			OnMatchSubject();
			break;
		case cmd_MatchDate:
			OnMatchDate();
			break;

		// Route these through the preview
		case cmd_ReadPrevMessage:
		case cmd_ToolbarMessagePreviousBtn:
		case cmd_ReadNextMessage:
		case cmd_ToolbarMessageNextBtn:
		case cmd_DeleteReadNextMessage:
		case cmd_ToolbarMessageDeleteNextBtn:
		case cmd_CopyReadNextMessage:
			if (mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview())
			{
				cmdHandled = mInfoTableView->GetPreview()->ObeyCommand(inCommand, ioParam);
				break;
			}
			
			// Fall through
		default:
			cmdHandled = CMailboxTable::ObeyCommand(inCommand, ioParam);
			break;
		}
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMailboxInfoTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	ResIDT	menuID;
	SInt16	menuItem;

	outUsesMark = false;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Check for apply rules and only allow if not locked out
		if (CApplyRulesMenu::IsApplyRulesMenu(menuID))
			outEnabled = !CAdminLock::sAdminLock.mNoRules;
		else
			CMailboxTable::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
	}
	else
	{
		switch (inCommand)
		{
		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
		case cmd_ToolbarMailboxSelectPopup:
		case cmd_PuntUnseen:
		case cmd_CheckMail:
		case cmd_ToolbarCheckMailboxBtn:
		case cmd_GotoMessage:
		case cmd_SearchMailbox:
		case cmd_ToolbarSearchMailboxBtn:
		case cmd_SearchAgainMailbox:
		case cmd_NextSearchMailbox:
		case cmd_ThreadSelectAll:
		case cmd_ThreadMarkSeen:
		case cmd_ThreadMarkImportant:
		case cmd_ThreadMarkDeleted:
		case cmd_ThreadSkip:
		case cmd_MatchUnseen:
		case cmd_MatchImportant:
		case cmd_MatchDeleted:
		case cmd_MatchSentToday:
		case cmd_MatchFrom:
		case cmd_MatchSubject:
		case cmd_MatchDate:
			outEnabled = true;
			break;

		case cmd_Close:
			outEnabled = CanFileClose();
			break;

		case cmd_ApplyRuleMailbox:
		case cmd_ToolbarMailboxApplyRules:
			outEnabled = !CAdminLock::sAdminLock.mNoRules;
			break;

		case cmd_SynchroniseMailbox:
			outEnabled = GetMbox() && GetMbox()->GetProtocol()->CanDisconnect() && !GetMbox()->GetProtocol()->IsDisconnected();
			break;

		case cmd_ClearDisconnectMailbox:
			outEnabled = GetMbox() && GetMbox()->GetProtocol()->CanDisconnect() && mIsSelectionValid;
			break;

		case cmd_SubscribeMailbox:
			outEnabled = GetMbox() && !GetMbox()->IsSubscribed();
			break;

		case cmd_UnsubscribeMailbox:
			outEnabled = GetMbox() && GetMbox()->IsSubscribed();
			break;

		case cmd_CacheAll:
			outEnabled = GetMbox() && !GetMbox()->FullyCached();
			break;

		case cmd_UpdateMatch:
			// In match state but not search results match
			outEnabled = GetMbox() && (GetMbox()->GetViewMode() != NMbox::eViewMode_All) && !mSearchResults;
			break;

		case cmd_ToolbarMailboxMatchBtn:
		case cmd_ToolbarMailboxMatchOptionBtn:
			outEnabled = true;
			outUsesMark = true;
			outMark = (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch)) ? (UInt16)checkMark : (UInt16)noMark;
			break;

		// Must have some undeleted
		case cmd_ReplyMessage:
		case cmd_ToolbarMessageReplyBtn:
		case cmd_ReplyToSenderMessage:
		case cmd_ReplyToFromMessage:
		case cmd_ReplyToAllMessage:
		case cmd_ToolbarMessageReplyAllBtn:
		case cmd_ForwardMessage:
		case cmd_ToolbarMessageForwardBtn:
		case cmd_BounceMessage:
		case cmd_ToolbarMessageBounceBtn:
		case cmd_CreateDigestMessage:
		case cmd_ToolbarMessageDigestBtn:
			outEnabled = false;

			// Enable menu only if there's a selection
			if (mIsSelectionValid)
			{

				// Also only if at least one is not deleted
				outEnabled = !mTestSelectionAndDeleted ||
								CPreferences::sPrefs->mOpenDeleted.GetValue();
			}
			break;

		case cmd_RejectMessage:
		case cmd_ToolbarMessageRejectBtn:
			outEnabled = false;

			// Enable menu only if there's a selection
			if (mIsSelectionValid && CAdminLock::sAdminLock.mAllowRejectCommand)
			{

				// Also only if at least one is not deleted
				outEnabled = !mTestSelectionAndDeleted ||
								CPreferences::sPrefs->mOpenDeleted.GetValue();
			}
			break;

		case cmd_SendAgainMessage:
		case cmd_ToolbarMessageSendAgainBtn:
			outEnabled = false;

			// Enable menu only if there's a selection
			if (mIsSelectionValid)
			{

				// Also only if at least one is not deleted
				outEnabled = (!mTestSelectionAndDeleted ||
								CPreferences::sPrefs->mOpenDeleted.GetValue()) &&
								TestSelectionIgnoreOr(&CMailboxInfoTable::TestSelectionSmart);
			}
			break;

		// Must have a selection and not be locked out
		case cmd_MakeRuleMailbox:
			outEnabled = mIsSelectionValid && !CAdminLock::sAdminLock.mNoRules;
			break;

		case cmd_FlagsMessage:
		case cmd_ToolbarMessageFlagsBtn:
			outEnabled = mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(NMessage::eUserFlags);
			break;

		case cmd_FlagsSeen:
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, NMessage::eSeen);
			break;

		case cmd_FlagsImportant:
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, NMessage::eFlagged);
			break;

		case cmd_FlagsAnswered:
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, NMessage::eAnswered);
			break;

		case cmd_FlagsDeleted:
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, NMessage::eDeleted);
			break;

		case cmd_FlagsDraft:
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, NMessage::eDraft);
			break;

		case cmd_FlagsLabel1:
		case cmd_FlagsLabel2:
		case cmd_FlagsLabel3:
		case cmd_FlagsLabel4:
		case cmd_FlagsLabel5:
		case cmd_FlagsLabel6:
		case cmd_FlagsLabel7:
		case cmd_FlagsLabel8:
		{
			FindFlagCommandStatus(outEnabled, outUsesMark, outMark, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (inCommand - cmd_FlagsLabel1)));
			LStr255 labelName(CPreferences::sPrefs->mLabels.GetValue()[inCommand - cmd_FlagsLabel1]->name);
			::PLstrcpy(outName, labelName);
			break;
		}

		// Route these through the preview
		case cmd_ReadPrevMessage:
		case cmd_ToolbarMessagePreviousBtn:
		case cmd_ReadNextMessage:
		case cmd_ToolbarMessageNextBtn:
		case cmd_DeleteReadNextMessage:
		case cmd_ToolbarMessageDeleteNextBtn:
		case cmd_CopyReadNextMessage:
			if (mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview())
			{
				mInfoTableView->GetPreview()->FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
				break;
			}
			
			// Fall through
		default:
			CMailboxTable::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
		}
	}
}

void CMailboxInfoTable::FindFlagCommandStatus(
	Boolean				&outEnabled,
	Boolean				&outUsesMark,
	UInt16				&outMark,
	NMessage::EFlags	flag)
{
	outEnabled = mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(flag);
	outUsesMark = outEnabled;
	if (outEnabled)
	{
		// Use mark if at least one answered
		if (TestSelectionIgnore1Or(&CMailboxInfoTable::TestSelectionFlag, flag))
		{
			// Use check mark if all answered
			outMark =  (TestSelectionIgnore1And(&CMailboxInfoTable::TestSelectionFlag, flag)) ? checkMark : diamondMark;
		}
		else
			outMark = noMark;
	}
}

#pragma mark ____________________________Click & Draw

void CMailboxInfoTable::ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh)
{
	// Do inherited
	CMailboxTable::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);

	// Do cache increment update
	UpdateCacheIncrement();
}

#pragma mark ____________________________Display updating

// Reset button & caption state as well
void CMailboxInfoTable::Refresh()
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::Refresh();

} // CMailboxInfoTable::Refresh

// Reset button & caption state as well
void CMailboxInfoTable::RefreshSelection()
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::RefreshSelection();

} // CMailboxInfoTable::RefreshSelection

// Reset button & caption state as well
void CMailboxInfoTable::RefreshRow(const TableIndexT aRow)
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::RefreshRow(aRow);
}

void CMailboxInfoTable::DoSelectionChanged(void)
{
	CMailboxTable::DoSelectionChanged();

	// Determine whether preview is triggered
	const CUserAction& preview = mInfoTableView->GetPreviewAction();
	if (preview.GetSelection())
		PreviewMessage();

	// Determine whether full view is triggered
	const CUserAction& fullview = mInfoTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFileOpenMessage(false);
}

// Update after possible change
void CMailboxInfoTable::UpdateItems(void)
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
void CMailboxInfoTable::UpdateState(void)
{
	// Must recalculate these
	mIsSelectionValid = IsSelectionValid() && !TestSelectionAnd((TestSelectionPP) &CMailboxInfoTable::TestSelectionFake);
	mTestSelectionAndDeleted = TestSelectionIgnore1And(&CMailboxInfoTable::TestSelectionFlag, NMessage::eDeleted);
	
	// Broadcast a selection change which will force an update
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

// Update captions
void CMailboxInfoTable::UpdateCaptions(void)
{
	cdstring aStr;
	if (GetMbox())
		aStr = GetMbox()->GetNumberFound();
	else
		aStr = "--";
	mInfoTableView->mTotal->SetText(aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberUnseen();
	else
		aStr = "--";
	mInfoTableView->mUnseen->SetText(aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberDeleted();
	else
		aStr = "--";
	mInfoTableView->mDeleted->SetText(aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberMatch();
	else
		aStr = "--";
	mInfoTableView->mMatched->SetText(aStr);
}

// Check whether drag item is acceptable
Boolean CMailboxInfoTable::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
{
	// Get the flavor
	FlavorType theFlavor;
	if (::GetFlavorType(inDragRef, inItemRef, 1, &theFlavor) != noErr)
		return false;
	
	// Do not allow message drop on read-only mailbox
	if ((theFlavor == cDragMsgType) &&
		GetMbox() && GetMbox()->IsReadOnly())
		return false;

	return CMailboxTable::ItemIsAcceptable(inDragRef, inItemRef);
}
