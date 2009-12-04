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
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CLetterWindow.h"
#include "CMailControl.h"
#include "CMailboxTitleTable.h"
#include "CMessageWindow.h"
#include "CMailboxInfoView.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStatusWindow.h"
#include "CWindow.h"

#include <JXStaticText.h>
#include <JXTextMenu.h>


// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoTable::CMailboxInfoTable(JXScrollbarSet* scrollbarSet,
										 JXContainer* enclosure,
										 const HSizingOption hSizing,
										 const VSizingOption vSizing,
										 const JCoordinate x, const JCoordinate y,
										 const JCoordinate w, const JCoordinate h)
	: CMailboxTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	InitInfoTable();
}


// Default destructor
CMailboxInfoTable::~CMailboxInfoTable()
{
}

// Do common init
void CMailboxInfoTable::InitInfoTable()
{
	// Zero out
	mSearchResults = false;
	mLastTyping = 0;
	mLastChar = 0;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxInfoTable::OnCreate()
{
	// Do inherited
	CMailboxTable::OnCreate();

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sApp->sFlavorMsgList);
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());
	AddDropFlavor(CMulberryApp::sApp->sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sApp->sFlavorMboxList);
	AddDropFlavor(CMulberryApp::sApp->sFlavorMboxRefList);

	SetReadOnly(false);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);

	mInfoTableView = dynamic_cast<CMailboxInfoView*>(mTableView);

	// Context menu
	CreateContextMenu(CMainMenu::eContextMailboxTable);
}

// Handle key presses
bool CMailboxInfoTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Get state of command modifiers (not shift)
	bool mods = modifiers.shift() || modifiers.control() || modifiers.alt() || modifiers.meta();

	// Look for mode switch
	bool do_mode = false;
	if (mLastTyping != 0)
	{
		timeval tv;
		::gettimeofday(&tv, NULL);
		unsigned long new_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
		do_mode = (new_time - mLastTyping <= 1000);
	}

	unsigned long cmd = 0;
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
				switch (key)
				{
				case 'a':
				case 'A':
					cmd = CCommand::eCommandMailboxThreadAll;
					break;
				case 's':
				case 'S':
					cmd = CCommand::eCommandMailboxThreadMarkSeen;
					break;
				case 'd':
				case 'D':
					cmd = CCommand::eCommandMailboxThreadMarkDeleted;
					break;
				case 'k':
				case 'K':
					cmd = CCommand::eCommandMailboxThreadSkip;
					break;
				default:
					// Beep for illegal mode character
					::MessageBeep(1);
					break;
				}
			}
			else
				// Beep for illegal mode
				::MessageBeep(1);
			break;
		// Match aggregate mode
		case 'u':
			switch (key)
			{
			case 's':
			case 'S':
				cmd = CCommand::eCommandMailboxMatchUnseen;
				break;
			case 'i':
			case 'I':
				cmd = CCommand::eCommandMailboxMatchImportant;
				break;
			case 'd':
			case 'D':
				cmd = CCommand::eCommandMailboxMatchDeleted;
				break;
			case 't':
			case 'T':
				cmd = CCommand::eCommandMailboxMatchSentToday;
				break;
			case 'f':
			case 'F':
				cmd = CCommand::eCommandMailboxMatchFrom;
				break;
			case 'j':
			case 'J':
				cmd = CCommand::eCommandMailboxMatchSubject;
				break;
			case 'a':
			case 'A':
				cmd = CCommand::eCommandMailboxMatchDate;
				break;
			case 'u':
			case 'U':
				// Toggle toolbar button state to turn off match
				if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch))
					cmd = CCommand::eToolbarMailboxMatchBtn;
				else
					cmd = CCommand::eToolbarMailboxMatchOptionBtn;
				break;
			default:
				// Beep for illegal mode character
				::MessageBeep(1);
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

		switch (key)
		{
		// Select unseen
		case '\t':
			// Look for option key
			if (modifiers.control()) 
				DoRollover(false);
			else
				SelectNextNew(modifiers.shift());
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
			cmd = CCommand::eEditSelectAll;
			break;

		case 'd':
		case 'D':
			cmd = CCommand::eFlagsDeleted;
			break;

		case 's':
		case 'S':
			cmd = CCommand::eFlagsSeen;
			break;

		case 'i':
		case 'I':
			cmd = CCommand::eFlagsImportant;
			break;

		case 'e':
		case 'E':
			cmd = CCommand::eMailboxExpunge;
			break;

		case 'l':
		case 'L':
			cmd = CCommand::eMailboxCheckMail;
			break;

		case 'y':
		case 'Y':
			cmd = CCommand::eMailboxSearch;
			break;

		case 'r':
		case 'R':
			cmd = CCommand::eMessagesReply;
			break;

		case 'f':
		case 'F':
			cmd = CCommand::eMessagesForward;
			break;

		case 'b':
		case 'B':
			cmd = CCommand::eMessagesBounce;
			break;

		case 'c':
		case 'C':
			cmd = CCommand::eCommandMessageCopy;
			break;

		case 'm':
		case 'M':
			cmd = CCommand::eCommandMessageMove;
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
				cmd = CCommand::eFileClose;
			break;

		case 'z':
		case 'Z':
			cmd = CCommand::eToolbarZoomItems;
			break;

		case 't':
		case 'T':
			// Thread mode switch
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				timeval tv;
				::gettimeofday(&tv, NULL);
				mLastTyping = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
				mLastChar = 't';
				return true;
			}
			break;

		case 'u':
		case 'U':
			// Match mode switch
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				timeval tv;
				::gettimeofday(&tv, NULL);
				mLastTyping = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
				mLastChar = 'u';
				return true;
			}
			break;

		default:;
		}
	}

	// Check for command execute
	if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods && (cmd != 0))
	{
		// Check whether command is valid right now
		CCmdUI cmdui;
		cmdui.mCmd = cmd;
		
		// Only if commander available
		UpdateCommand(cmd, &cmdui);
		
		// Execute if enabled
		if (cmdui.GetEnabled())
			ObeyCommand(cmd, NULL);

		return true;
	}

	// Do default key press processing
	return CMailboxTable::HandleChar(key, modifiers);
}

void CMailboxInfoTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
	case CCommand::eToolbarMailboxSelectPopup:
	case CCommand::eMailboxMarkContentsSeen:
	case CCommand::eMailboxCheckMail:
	case CCommand::eToolbarCheckMailboxBtn:
	case CCommand::eMailboxGotoMessage:
	case CCommand::eMailboxSearch:
	case CCommand::eToolbarSearchMailboxBtn:
	case CCommand::eMailboxSearchAgain:
	case CCommand::eMailboxNextSearch:
	case CCommand::eCommandMailboxThreadAll:
	case CCommand::eCommandMailboxThreadMarkSeen:
	case CCommand::eCommandMailboxThreadMarkImportant:
	case CCommand::eCommandMailboxThreadMarkDeleted:
	case CCommand::eCommandMailboxThreadSkip:
	case CCommand::eCommandMailboxMatchUnseen:
	case CCommand::eCommandMailboxMatchImportant:
	case CCommand::eCommandMailboxMatchDeleted:
	case CCommand::eCommandMailboxMatchSentToday:
	case CCommand::eCommandMailboxMatchFrom:
	case CCommand::eCommandMailboxMatchSubject:
	case CCommand::eCommandMailboxMatchDate:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eFileClose:
		OnUpdateFileClose(cmdui);
		return;

	case CCommand::eMailboxSynchronise:
		OnUpdateMailboxSynchronise(cmdui);
		return;

	case CCommand::eMailboxClearDisconnected:
		OnUpdateMailboxClearDisocnnect(cmdui);
		return;

	case CCommand::eMailboxSubscribe:
		OnUpdateMailboxSubscribe(cmdui);
		return;

	case CCommand::eMailboxUnsubscribe:
		OnUpdateMailboxUnsubscribe(cmdui);
		return;

	case CCommand::eMailboxLoadAllMessages:
		OnUpdateMailboxPartialCache(cmdui);
		return;

	case CCommand::eMailboxUpdateMatch:
		OnUpdateMailboxUpdateMatch(cmdui);
		return;

	case CCommand::eMailboxApplyRules:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eToolbarMailboxApplyRules:
		OnUpdateMailboxApplyRulesBtn(cmdui);
		return;

	case CCommand::eMailboxMakeRule:
		OnUpdateMailboxMakeRule(cmdui);
		return;

	case CCommand::eToolbarMailboxMatchBtn:
	case CCommand::eToolbarMailboxMatchOptionBtn:
		OnUpdateMatchBtn(cmdui);
		return;

	case CCommand::eMessagesReply:
	case CCommand::eToolbarMessageReplyBtn:
	case CCommand::eMessagesReplyToSender:
	case CCommand::eMessagesReplyToFrom:
	case CCommand::eMessagesReplyToAll:
	case CCommand::eToolbarMessageReplyAllBtn:
	case CCommand::eMessagesForward:
	case CCommand::eToolbarMessageForwardBtn:
	case CCommand::eMessagesBounce:
	case CCommand::eToolbarMessageBounceBtn:
	case CCommand::eMessagesCreateDigest:
	case CCommand::eToolbarMessageDigestBtn:
		OnUpdateSelectionUndeleted(cmdui);
		return;

	case CCommand::eMessagesReject:
	case CCommand::eToolbarMessageRejectBtn:
		OnUpdateMessageReject(cmdui);
		return;

	case CCommand::eMessagesSendAgain:
	case CCommand::eToolbarMessageSendAgainBtn:
		OnUpdateMessageSendAgain(cmdui);
		return;

	case CCommand::eMessagesFlags:
		OnUpdateFlags(cmdui);
		return;

	case CCommand::eToolbarMessageFlagsBtn:
		if (cmdui->mMenu != NULL)
		{
			switch(cmdui->mMenuIndex)
			{
			case 1:
				OnUpdateFlagsSeen(cmdui);
				break;
			case 2:
				OnUpdateFlagsImportant(cmdui);
				break;
			case 3:
				OnUpdateFlagsAnswered(cmdui);
				break;
			case 4:
				OnUpdateFlagsDeleted(cmdui);
				break;
			case 5:
				OnUpdateFlagsDraft(cmdui);
				break;
			default:
				OnUpdateFlags(cmdui, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (cmdui->mMenuIndex - 6)));
				cmdui->SetText(CPreferences::sPrefs->mLabels.GetValue()[cmdui->mMenuIndex - 6]->name);
				break;
			}
		}
		else
			OnUpdateFlags(cmdui);
		break;

	case CCommand::eFlagsSeen:
		OnUpdateFlagsSeen(cmdui);
		return;

	case CCommand::eFlagsImportant:
		OnUpdateFlagsImportant(cmdui);
		return;

	case CCommand::eFlagsAnswered:
		OnUpdateFlagsAnswered(cmdui);
		return;

	case CCommand::eFlagsDeleted:
		OnUpdateFlagsDeleted(cmdui);
		return;

	case CCommand::eFlagsDraft:
		OnUpdateFlagsDraft(cmdui);
		return;

	case CCommand::eFlagsLabel1:
	case CCommand::eFlagsLabel2:
	case CCommand::eFlagsLabel3:
	case CCommand::eFlagsLabel4:
	case CCommand::eFlagsLabel5:
	case CCommand::eFlagsLabel6:
	case CCommand::eFlagsLabel7:
	case CCommand::eFlagsLabel8:
		OnUpdateFlagsLabel(cmdui);
		break;

	// Route these through the preview
	case CCommand::eMessagesReadPrev:
	case CCommand::eToolbarMessagePreviousBtn:
	case CCommand::eMessagesReadNext:
	case CCommand::eToolbarMessageNextBtn:
	case CCommand::eMessagesDelAndReadNext:
	case CCommand::eToolbarMessageDeleteNextBtn:
	case CCommand::eMessagesCopyAndReadNext:
		if (mInfoTableView->GetUsePreview())
		{
			mInfoTableView->GetPreview()->UpdateCommand(cmd, cmdui);
			return;
		}

	default:;
	}

	if (cmdui->mMenu)
	{
		// Apply Rules sub-menu
		if (static_cast<CWindow*>(mInfoTableView->GetOwningWindow())->GetMainMenuID(cmdui->mMenu) == CMainMenu::eApplyRules)
		{
			OnUpdateMailboxApplyRules(cmdui);
			return;
		}
	}

	CMailboxTable::UpdateCommand(cmd, cmdui);
}

bool CMailboxInfoTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnFileNewDraft();
		return true;

	case CCommand::eFileClose:
		OnFileClose();
		return true;

	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		OnMailboxProperties();
		return true;

	case CCommand::eMailboxMarkContentsSeen:
		OnMailboxPunt();
		return true;

	case CCommand::eMailboxSynchronise:
		OnMailboxSynchronise();
		return true;

	case CCommand::eMailboxClearDisconnected:
		OnMailboxClearDisconnect();
		return true;

	case CCommand::eMailboxSubscribe:
		OnMailboxSubscribe();
		return true;

	case CCommand::eMailboxUnsubscribe:
		OnMailboxUnsubscribe();
		return true;

	case CCommand::eMailboxCheckMail:
	case CCommand::eToolbarCheckMailboxBtn:
		OnMailboxCheck();
		return true;

	case CCommand::eMailboxGotoMessage:
		OnMailboxGotoMessage();
		return true;

	case CCommand::eMailboxLoadAllMessages:
		OnMailboxCacheAll();
		return true;

	case CCommand::eMailboxUpdateMatch:
		OnMailboxUpdateMatch();
		return true;

	case CCommand::eMailboxSearch:
	case CCommand::eToolbarSearchMailboxBtn:
		OnSearchMailbox();
		return true;

	case CCommand::eMailboxSearchAgain:
		OnSearchAgainMailbox();
		return true;

	case CCommand::eMailboxNextSearch:
		OnNextSearchMailbox();
		return true;

	case CCommand::eMailboxMakeRule:
		OnMakeRuleMailbox();
		return true;

	case CCommand::eToolbarMailboxMatchBtn:
		OnMatch();
		return true;

	case CCommand::eToolbarMailboxMatchOptionBtn:
		OnMatchButton(true);
		return true;

	case CCommand::eToolbarMailboxSelectPopup:
		OnSelectPopup(menu->mIndex);
		return true;

	case CCommand::eToolbarMailboxApplyRules:
		OnApplyRuleMailboxToolbar(reinterpret_cast<const char*>(menu));
		break;

	case CCommand::eMessagesReply:
		OnMessageReply();
		return true;

	case CCommand::eToolbarMessageReplyBtn:
		// Route this through the preview if its available and displaying a message and this table is not the target
		// NB This will only happen for toolbar clicks as menu commands will be routed
		// to the preview when its focussed.
		if (!IsTarget() && mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview() && mInfoTableView->GetPreview()->GetMessage())
			mInfoTableView->GetPreview()->ObeyCommand(cmd, menu);
		else
			OnMessageReply();
		return true;

	case CCommand::eMessagesReplyToSender:
		OnMessageReplySender();
		return true;

	case CCommand::eMessagesReplyToFrom:
		OnMessageReplyFrom();
		return true;

	case CCommand::eMessagesReplyToAll:
	case CCommand::eToolbarMessageReplyAllBtn:
		OnMessageReplyAll();
		return true;

	case CCommand::eMessagesForward:
		OnMessageForward();
		return true;

	case CCommand::eToolbarMessageForwardBtn:
		// Route this through the preview if its available and displaying a message
		// NB This will only happen for toolbar clicks as menu commands will be routed
		// to the preview when its focussed.
		if (mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview() && mInfoTableView->GetPreview()->GetMessage())
			mInfoTableView->GetPreview()->ObeyCommand(cmd, menu);
		else
			OnMessageForward();
		return true;

	case CCommand::eMessagesBounce:
	case CCommand::eToolbarMessageBounceBtn:
		OnMessageBounce();
		return true;

	case CCommand::eMessagesReject:
	case CCommand::eToolbarMessageRejectBtn:
		OnMessageReject();
		return true;

	case CCommand::eMessagesSendAgain:
	case CCommand::eToolbarMessageSendAgainBtn:
		OnMessageSendAgain();
		return true;

	case CCommand::eMessagesCreateDigest:
	case CCommand::eToolbarMessageDigestBtn:
		OnMessageCreateDigest();
		return true;

	case CCommand::eFlagsSeen:
		DoFlagMailMessage(NMessage::eSeen);
		return true;

	case CCommand::eFlagsImportant:
		DoFlagMailMessage(NMessage::eFlagged);
		return true;

	case CCommand::eFlagsAnswered:
		DoFlagMailMessage(NMessage::eAnswered);
		return true;

	case CCommand::eFlagsDeleted:
		DoFlagMailMessage(NMessage::eDeleted);
		return true;

	case CCommand::eFlagsDraft:
		DoFlagMailMessage(NMessage::eDraft);
		return true;

	case CCommand::eFlagsLabel1:
	case CCommand::eFlagsLabel2:
	case CCommand::eFlagsLabel3:
	case CCommand::eFlagsLabel4:
	case CCommand::eFlagsLabel5:
	case CCommand::eFlagsLabel6:
	case CCommand::eFlagsLabel7:
	case CCommand::eFlagsLabel8:
		DoFlagMailMessage(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (cmd - CCommand::eFlagsLabel1)));
		return true;

	case CCommand::eToolbarMessageFlagsBtn:
		switch(menu->mIndex)
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
			DoFlagMailMessage(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (menu->mIndex - 7)));
			break;
		}
		break;

	// Special mode commands
	case CCommand::eCommandMailboxThreadAll:
		OnThreadSelectAll();
		break;
	case CCommand::eCommandMailboxThreadMarkSeen:
		OnThreadMarkSeen();
		break;
	case CCommand::eCommandMailboxThreadMarkImportant:
		OnThreadMarkImportant();
		break;
	case CCommand::eCommandMailboxThreadMarkDeleted:
		OnThreadMarkDeleted();
		break;
	case CCommand::eCommandMailboxThreadSkip:
		OnThreadSkip();
		break;

	case CCommand::eCommandMailboxMatchUnseen:
		OnMatchUnseen();
		break;
	case CCommand::eCommandMailboxMatchImportant:
		OnMatchImportant();
		break;
	case CCommand::eCommandMailboxMatchDeleted:
		OnMatchDeleted();
		break;
	case CCommand::eCommandMailboxMatchSentToday:
		OnMatchSentToday();
		break;
	case CCommand::eCommandMailboxMatchFrom:
		OnMatchFrom();
		break;
	case CCommand::eCommandMailboxMatchSubject:
		OnMatchSubject();
		break;
	case CCommand::eCommandMailboxMatchDate:
		OnMatchDate();
		break;

	// Route these through the preview
	case CCommand::eMessagesReadPrev:
	case CCommand::eToolbarMessagePreviousBtn:
	case CCommand::eMessagesReadNext:
	case CCommand::eToolbarMessageNextBtn:
	case CCommand::eMessagesDelAndReadNext:
	case CCommand::eToolbarMessageDeleteNextBtn:
	case CCommand::eMessagesCopyAndReadNext:
		if (mInfoTableView->GetUsePreview())
			return mInfoTableView->GetPreview()->ObeyCommand(cmd, menu);
			
	default:;
	}

	if (menu)
	{
		// Do Apply rules
		if (menu->mMenu == static_cast<CWindow*>(mInfoTableView->GetOwningWindow())->GetMenus()[CMainMenu::eApplyRules])
		{
			OnApplyRuleMailbox(menu->mIndex - 1);
			return true;
		}
	}

	return CMailboxTable::ObeyCommand(cmd, menu);
}

// Reset button & caption state as well
void CMailboxInfoTable::Refresh() const
{
	// Update buttons then do inherited
	const_cast<CMailboxInfoTable*>(this)->UpdateItems();
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
void CMailboxInfoTable::RefreshRow(TableIndexT aRow)
{
	// Update buttons then do inherited
	UpdateItems();
	CMailboxTable::RefreshRow(aRow);
}

void CMailboxInfoTable::DoSelectionChanged()
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
void CMailboxInfoTable::UpdateItems()
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
void CMailboxInfoTable::UpdateCaptions()
{
	char aStr[32];
	if (GetMbox())
		::snprintf((char*) aStr, 32, "%ld", GetMbox()->GetNumberFound());
	else
		::snprintf((char*) aStr, 32, "--");
	mInfoTableView->mTotal->SetText(aStr);

	if (GetMbox())
		::snprintf((char*) aStr, 32, "%ld", GetMbox()->GetNumberUnseen());
	else
		::snprintf((char*) aStr, 32, "--");
	mInfoTableView->mUnseen->SetText(aStr);

	if (GetMbox())
		::snprintf((char*) aStr, 32, "%ld", GetMbox()->GetNumberDeleted());
	else
		::snprintf((char*) aStr, 32, "--");
	mInfoTableView->mDeleted->SetText(aStr);

	if (GetMbox())
		::snprintf((char*) aStr, 32, "%ld", GetMbox()->GetNumberMatch());
	else
		::snprintf((char*) aStr, 32, "--");
	mInfoTableView->mMatched->SetText(aStr);
}

void CMailboxInfoTable::OnUpdateFileClose(CCmdUI* pCmdUI)
{
	// Must have mailbox
	pCmdUI->Enable(CanFileClose());
}

void CMailboxInfoTable::OnUpdateMailboxSubscribe(CCmdUI* pCmdUI)
{
	// Must not be subscribed
	pCmdUI->Enable(GetMbox() && !GetMbox()->IsSubscribed());
}

void CMailboxInfoTable::OnUpdateMailboxUnsubscribe(CCmdUI* pCmdUI)
{
	// Must be subscribed
	pCmdUI->Enable(GetMbox() && GetMbox()->IsSubscribed());
}

void CMailboxInfoTable::OnUpdateMailboxSynchronise(CCmdUI* pCmdUI)
{
	// Must be some deleted
	pCmdUI->Enable(GetMbox() && GetMbox()->GetProtocol()->CanDisconnect() && !GetMbox()->GetProtocol()->IsDisconnected());
}

void CMailboxInfoTable::OnUpdateMailboxClearDisocnnect(CCmdUI* pCmdUI)
{
	// Must be some deleted
	pCmdUI->Enable(GetMbox() && GetMbox()->GetProtocol()->CanDisconnect() && mIsSelectionValid);
}

void CMailboxInfoTable::OnUpdateMailboxPartialCache(CCmdUI* pCmdUI)
{
	// Must be some deleted
	pCmdUI->Enable(GetMbox() && !GetMbox()->FullyCached());
}

void CMailboxInfoTable::OnUpdateMailboxUpdateMatch(CCmdUI* pCmdUI)
{
	// In match state but not search results match
	pCmdUI->Enable(GetMbox() && (GetMbox()->GetViewMode() != NMbox::eViewMode_All) && !mSearchResults);
}

void CMailboxInfoTable::OnUpdateMailboxApplyRules(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
	
	// Force reset of mailbox menus - only done if required
	CApplyRulesMenu::ResetMenuList(pCmdUI->mMenu);
}

void CMailboxInfoTable::OnUpdateMailboxApplyRulesBtn(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
}

void CMailboxInfoTable::OnUpdateMailboxMakeRule(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
}

void CMailboxInfoTable::OnUpdateMessageReject(CCmdUI* pCmdUI)
{
	if (CAdminLock::sAdminLock.mAllowRejectCommand)
		OnUpdateSelectionUndeleted(pCmdUI);
	else
		pCmdUI->Enable(false);
}

void CMailboxInfoTable::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Must be one undeleted and smart
	pCmdUI->Enable((!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue()) &&
						TestSelectionIgnoreOr(&CMailboxInfoTable::TestSelectionSmart));
}

void CMailboxInfoTable::OnUpdateFlags(CCmdUI* pCmdUI)
{
	bool enabled = mIsSelectionValid && GetMbox() &&
					GetMbox()->HasAllowedFlag(static_cast<NMessage::EFlags>(NMessage::eDeleted | NMessage::eSeen |
																			NMessage::eAnswered | NMessage::eFlagged |
																			NMessage::eDraft));
	pCmdUI->Enable(enabled);
}

void CMailboxInfoTable::OnUpdateFlagsDeleted(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, NMessage::eDeleted);
}

void CMailboxInfoTable::OnUpdateFlagsSeen(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, NMessage::eSeen);
}

void CMailboxInfoTable::OnUpdateFlagsAnswered(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, NMessage::eAnswered);
}

void CMailboxInfoTable::OnUpdateFlagsImportant(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, NMessage::eFlagged);
}

void CMailboxInfoTable::OnUpdateFlagsDraft(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, NMessage::eDraft);
}

void CMailboxInfoTable::OnUpdateFlagsLabel(CCmdUI* pCmdUI)
{
	OnUpdateFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->mCmd - CCommand::eFlagsLabel1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->mCmd - CCommand::eFlagsLabel1]->name);
}

void CMailboxInfoTable::OnUpdateFlags(CCmdUI* pCmdUI, NMessage::EFlags flag)
{
	bool enabled = mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(flag);
	pCmdUI->Enable(enabled);
	if (enabled)
	{
		// Use mark if at least one deleted
		if (TestSelectionIgnore1Or(&CMailboxInfoTable::TestSelectionFlag, flag))
		{
			// Use check mark if all deleted
			pCmdUI->SetCheck(TestSelectionIgnore1And(&CMailboxInfoTable::TestSelectionFlag, flag) ? 1 : 2);
		}
		else
			pCmdUI->SetCheck(0);
	}
}

void CMailboxInfoTable::OnUpdateMatchBtn(CCmdUI* pCmdUI)
{
	// Button on or off
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
	pCmdUI->SetCheck(GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch));
}

void CMailboxInfoTable::OnFileNewDraft(void)
{
	DoNewLetter(GetDisplay()->GetLatestKeyModifiers().control());
}

void CMailboxInfoTable::OnApplyRuleMailbox(JIndex index)
{
	// Only if not locked out
	if (CAdminLock::sAdminLock.mNoAttachments)
		return;

	DoApplyRuleMailbox((index == CApplyRulesMenu::eApply_All) ? -1L : (long) index - CApplyRulesMenu::eApply_First);
}

void CMailboxInfoTable::OnMessageReply()
{
	DoMessageReply(replyReplyTo, GetDisplay()->GetLatestKeyModifiers().control());
}

void CMailboxInfoTable::OnMessageReplySender()
{
	DoMessageReply(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMailboxInfoTable::OnMessageReplyFrom()
{
	DoMessageReply(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMailboxInfoTable::OnMessageReplyAll()
{
	DoMessageReply(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMailboxInfoTable::OnMessageForward()
{
	DoMessageForward(GetDisplay()->GetLatestKeyModifiers().control());
}

void CMailboxInfoTable::OnMatch()
{
	// Toggle match mode
	OnMatchButton(GetDisplay()->GetLatestKeyModifiers().control());
}

// Check whether drag item is acceptable
bool CMailboxInfoTable::ItemIsAcceptable(const JArray<Atom>& typeList)
{
	// Look for a format
	std::auto_ptr<JOrderedSetIterator<Atom> > ti(typeList.NewIterator());
	Atom i;
	while (ti->Next(&i))
	{
		// Some formats cannot be dragged outside the source window
		if ((i == CMulberryApp::sFlavorMsgList) &&
			GetMbox() && GetMbox()->IsReadOnly())
			return false;
	}

	return CMailboxTable::ItemIsAcceptable(typeList);
}

void CMailboxInfoTable::ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Only if context set - not all widgets that have this
	// class mixed in need context menus
	if (mContextMenu)
	{
		// Insert copy to menu here and force update
		JXTextMenu* copy_to = NULL;
		if (!mContextMenu->GetSubmenu(7, (const JXMenu**) &copy_to))
		{
			copy_to = new CCopyToMenu::CCopyToMain(true, mContextMenu, 7, const_cast<JXContainer*>(mContextMenu->GetEnclosure()));
			ListenTo(copy_to);
		}

		CMailControl::BlockBusy(true);
		try
		{
			// Force reset of mailbox menus - only done if required
			CCopyToMenu::ResetMenuList(copy_to, NULL);

			// All items are valid
			copy_to->EnableAll();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			CMailControl::BlockBusy(false);
			CLOG_LOGRETHROW;
			throw;
		}
		CMailControl::BlockBusy(false);

		// Do popup
		mContextMenu->PopUp(mOwner, pt, buttonStates, modifiers);
	}
}
