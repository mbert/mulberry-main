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

#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CCmdUIChecker.h"
#include "CMailboxInfoFrame.h"
#include "CMailboxInfoWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxInfoTable

IMPLEMENT_DYNCREATE(CMailboxInfoTable, CTableDragAndDrop)

BEGIN_MESSAGE_MAP(CMailboxInfoTable, CMailboxTable)
	ON_UPDATE_COMMAND_UI(IDM_FILE_NEW_DRAFT, OnUpdateAlways)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnFileNewDraft)

	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PROPERTIES, OnUpdateAlways)
	ON_COMMAND(IDM_EDIT_PROPERTIES, OnMailboxProperties)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_PUNT, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_PUNT, OnMailboxPunt)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SUBSCRIBE, OnUpdateMailboxSubscribe)
	ON_COMMAND(IDM_MAILBOX_SUBSCRIBE, OnMailboxSubscribe)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_UNSUBSCRIBE, OnUpdateMailboxUnsubscribe)
	ON_COMMAND(IDM_MAILBOX_UNSUBSCRIBE, OnMailboxUnsubscribe)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SYNCHRONISE, OnUpdateMailboxSynchronise)
	ON_COMMAND(IDM_MAILBOX_SYNCHRONISE, OnMailboxSynchronise)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CLEARDISCONNECT, OnUpdateMailboxClearDisconnect)
	ON_COMMAND(IDM_MAILBOX_CLEARDISCONNECT, OnMailboxClearDisconnect)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CHECK, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_CHECK, OnMailboxCheck)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_GOTO, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_GOTO, OnMailboxGotoMessage)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CACHEALL, OnUpdateMailboxPartialCache)
	ON_COMMAND(IDM_MAILBOX_CACHEALL, OnMailboxCacheAll)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_UPDATEMATCH, OnUpdateMailboxUpdateMatch)
	ON_COMMAND(IDM_MAILBOX_UPDATEMATCH, OnMailboxUpdateMatch)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SEARCH, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_SEARCH, OnSearchMailbox)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SEARCH_AGAIN, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_SEARCH_AGAIN, OnSearchAgainMailbox)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_NEXT_SEARCH, OnUpdateAlways)
	ON_COMMAND(IDM_MAILBOX_NEXT_SEARCH, OnNextSearchMailbox)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_APPLY_RULES_ALL, IDM_APPLY_RULESEnd, OnUpdateMailboxApplyRules)
	ON_COMMAND_RANGE(IDM_APPLY_RULES_ALL, IDM_APPLY_RULESEnd, OnApplyRuleMailbox)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_MAKE_RULE, OnUpdateMailboxMakeRule)
	ON_COMMAND(IDM_MAILBOX_MAKE_RULE, OnMakeRuleMailbox)

	ON_COMMAND(IDC_MAILBOX_THREADSELECTALL, OnThreadSelectAll)
	ON_COMMAND(IDC_MAILBOX_THREADMARKSEEN, OnThreadMarkSeen)
	ON_COMMAND(IDC_MAILBOX_THREADMARKIMPORTANT, OnThreadMarkImportant)
	ON_COMMAND(IDC_MAILBOX_THREADMARKDELETED, OnThreadMarkDeleted)
	ON_COMMAND(IDC_MAILBOX_THREADSKIP, OnThreadSkip)

	ON_COMMAND(IDC_MAILBOX_MATCHUNSEEN, OnMatchUnseen)
	ON_COMMAND(IDC_MAILBOX_MATCHIMPORTANT, OnMatchImportant)
	ON_COMMAND(IDC_MAILBOX_MATCHDELETED, OnMatchDeleted)
	ON_COMMAND(IDC_MAILBOX_MATCHSENTTODAY, OnMatchSentToday)
	ON_COMMAND(IDC_MAILBOX_MATCHFROM, OnMatchFrom)
	ON_COMMAND(IDC_MAILBOX_MATCHSUBJECT, OnMatchSubject)
	ON_COMMAND(IDC_MAILBOX_MATCHDATE, OnMatchDate)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_REPLY, OnMessageReply)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_SENDER, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_REPLY_SENDER, OnMessageReplySender)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_FROM, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_REPLY_FROM, OnMessageReplyFrom)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_ALL, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_REPLY_ALL, OnMessageReplyAll)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_FORWARD, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_FORWARD, OnMessageForward)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_BOUNCE, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_BOUNCE, OnMessageBounce)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REJECT, OnUpdateMessageReject)
	ON_COMMAND(IDM_MESSAGES_REJECT, OnMessageReject)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_SEND_AGAIN, OnUpdateMessageSendAgain)
	ON_COMMAND(IDM_MESSAGES_SEND_AGAIN, OnMessageSendAgain)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_CREATE_DIGEST, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDM_MESSAGES_CREATE_DIGEST, OnMessageCreateDigest)

	ON_UPDATE_COMMAND_UI(IDM_FLAGS_DELETED, OnUpdateFlagsDeleted)
	ON_COMMAND(IDM_FLAGS_DELETED, OnMessageDelete)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_SEEN, OnUpdateFlagsSeen)
	ON_COMMAND(IDM_FLAGS_SEEN, OnFlagsSeen)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_ANSWERED, OnUpdateFlagsAnswered)
	ON_COMMAND(IDM_FLAGS_ANSWERED, OnFlagsAnswered)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_IMPORTANT, OnUpdateFlagsImportant)
	ON_COMMAND(IDM_FLAGS_IMPORTANT, OnFlagsImportant)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_DRAFT, OnUpdateFlagsDraft)
	ON_COMMAND(IDM_FLAGS_DRAFT, OnFlagsDraft)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FLAGS_LABEL1, IDM_FLAGS_LABEL8, OnUpdateFlagsLabel)
	ON_COMMAND_RANGE(IDM_FLAGS_LABEL1, IDM_FLAGS_LABEL8, OnFlagsLabel)

	ON_WM_CREATE()
	
	// Toolbar
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnFileNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnFileNewDraft)

	ON_COMMAND(IDC_TOOLBARDETAILSBTN, OnMailboxProperties)

	ON_COMMAND(IDC_TOOLBARCHECKMAILBOXBTN, OnMailboxCheck)

	ON_COMMAND(IDC_TOOLBARSEARCHMAILBOXBTN, OnSearchMailbox)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMAILBOXMATCHBTN, OnUpdateMatchButton)
	ON_COMMAND(IDC_TOOLBARMAILBOXMATCHBTN, OnMatch)
	ON_COMMAND(IDC_TOOLBARMAILBOXMATCHOPTIONBTN, OnMatchOption)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMAILBOXSELECTPOPUP, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_SELECT_SEEN, IDM_SELECT_NONMATCH, OnSelectPopup)
	ON_COMMAND_RANGE(IDM_SELECT_LABEL1, IDM_SELECT_LABEL8, OnSelectPopup)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMAILBOXAPPLYRULES, OnUpdateMailboxApplyRules)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEREPLYBTN, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYBTN, OnMessageReply)
	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYOPTIONBTN, OnMessageReply)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEREPLYALLBTN, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYALLBTN, OnMessageReplyAll)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEFORWARDBTN, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDC_TOOLBARMESSAGEFORWARDBTN, OnMessageForward)
	ON_COMMAND(IDC_TOOLBARMESSAGEFORWARDOPTIONBTN, OnMessageForward)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEBOUNCEBTN, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDC_TOOLBARMESSAGEBOUNCEBTN, OnMessageBounce)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEREJECTBTN, OnUpdateMessageReject)
	ON_COMMAND(IDC_TOOLBARMESSAGEREJECTBTN, OnMessageReject)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGESENDAGAINBTN, OnUpdateMessageSendAgain)
	ON_COMMAND(IDC_TOOLBARMESSAGESENDAGAINBTN, OnMessageSendAgain)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEDIGESTBTN, OnUpdateSelectionNotDeleted)
	ON_COMMAND(IDC_TOOLBARMESSAGEDIGESTBTN, OnMessageCreateDigest)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEFLAGSBTN, OnUpdateSelection)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoTable::CMailboxInfoTable()
{
	mSearchResults = false;
	mLastTyping = 0;
	mLastChar = 0;
}

// Default destructor
CMailboxInfoTable::~CMailboxInfoTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMailboxInfoTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxTable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sFlavorMsgList);
	AddDragFlavor(CF_UNICODETEXT);

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorMboxList);
	AddDropFlavor(CMulberryApp::sFlavorMboxRefList);

	SetReadOnly(false);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);

	mInfoTableView = dynamic_cast<CMailboxInfoView*>(mTableView);

	// Tooltips are active
	EnableToolTips();

	return 0;
}

#pragma mark ____________________________Display Updating

// Update captions & buttons
BOOL CMailboxInfoTable::RedrawWindow(LPCRECT lpRectUpdate, CRgn* prgnUpdate, UINT flags)
{
	// Update buttons then do inherited
	UpdateItems();

	return CWnd::RedrawWindow(lpRectUpdate, prgnUpdate, flags);
}

// Reset button & caption state as well
void CMailboxInfoTable::RefreshSelection()
{
	// Update buttons then do inherited
	UpdateItems();

	CMailboxTable::RefreshSelection();

} // CMailboxInfoTable::RefreshSelection

// Reset button & caption state as well
void CMailboxInfoTable::RefreshRow(TableIndexT row)
{
	// Update buttons then do inherited
	UpdateItems();

	CMailboxTable::RefreshRow(row);
}

void CMailboxInfoTable::DoSelectionChanged()
{
	// Determine whether preview is triggered
	const CUserAction& preview = mInfoTableView->GetPreviewAction();
	if (preview.GetSelection())
		PreviewMessage();

	// Determine whether full view is triggered
	const CUserAction& fullview = mInfoTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFileOpenMessage(false);

	// Do selection changed after changing preview as the selection change
	// status update needs to have the correct state for the preview
	CMailboxTable::DoSelectionChanged();
}

// Update after possible change
void CMailboxInfoTable::UpdateItems()
{
	// Don't update items until fully open
	if (mInfoTableView->IsOpening())
		return;

	// Update buttons then do inherited
	if (GetMbox() && !mUpdating)
		UpdateState();
	if (!mUpdating)
		UpdateCaptions();
}

// Update buttons
void CMailboxInfoTable::UpdateState()
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
	cdstring	aStr;
	if (GetMbox())
		aStr = GetMbox()->GetNumberFound();
	else
		aStr = "--";
	CUnicodeUtils::SetWindowTextUTF8(&mInfoTableView->mTotalText, aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberUnseen();
	else
		aStr = "--";
	CUnicodeUtils::SetWindowTextUTF8(&mInfoTableView->mUnseenText, aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberDeleted();
	else
		aStr = "--";
	CUnicodeUtils::SetWindowTextUTF8(&mInfoTableView->mDeletedText, aStr);

	if (GetMbox())
		aStr = GetMbox()->GetNumberMatch();
	else
		aStr = "--";
	CUnicodeUtils::SetWindowTextUTF8(&mInfoTableView->mMatchedText, aStr);
}

#pragma mark ____________________________________Keyboard/Mouse

// Handle key down
bool CMailboxInfoTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_TAB:
		// Look for control key
		if (::GetKeyState(VK_CONTROL) < 0)
			DoRollover(false);
		else
			SelectNextNew(::GetKeyState(VK_SHIFT) < 0);
		return true;
	default:;
	}

	// Did not handle key
	return CMailboxTable::HandleKeyDown(nChar, nRepCnt, nFlags);
}

// Handle key down
bool CMailboxInfoTable::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get state of command modifiers (not shift)
	bool mods = ((::GetKeyState(VK_CONTROL) < 0) || (::GetKeyState(VK_SHIFT) < 0) || (::GetKeyState(VK_MENU) < 0));

	// Look for mode switch
	bool do_mode = false;
	if (mLastTyping != 0)
	{
		unsigned long new_time = (1000 * clock())/CLOCKS_PER_SEC;
		do_mode = (new_time - mLastTyping < 1000);
	}

	UINT cmd = 0;
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
				switch (nChar)
				{
				case 'a':
				case 'A':
					cmd = IDC_MAILBOX_THREADSELECTALL;
					break;
				case 's':
				case 'S':
					cmd = IDC_MAILBOX_THREADMARKSEEN;
					break;
				case 'i':
				case 'I':
					cmd = IDC_MAILBOX_THREADMARKIMPORTANT;
					break;
				case 'd':
				case 'D':
					cmd = IDC_MAILBOX_THREADMARKDELETED;
					break;
				case 'k':
				case 'K':
					cmd = IDC_MAILBOX_THREADSKIP;
					break;
				default:
					// Beep for illegal mode character
					::MessageBeep(-1);
					break;
				}
			}
			else
				// Beep for illegal mode
				::MessageBeep(-1);
			break;
		// Match aggregate mode
		case 'u':
			switch (nChar)
			{
			case 's':
			case 'S':
				cmd = IDC_MAILBOX_MATCHUNSEEN;
				break;
			case 'i':
			case 'I':
				cmd = IDC_MAILBOX_MATCHIMPORTANT;
				break;
			case 'd':
			case 'D':
				cmd = IDC_MAILBOX_MATCHDELETED;
				break;
			case 't':
			case 'T':
				cmd = IDC_MAILBOX_MATCHSENTTODAY;
				break;
			case 'f':
			case 'F':
				cmd = IDC_MAILBOX_MATCHFROM;
				break;
			case 'j':
			case 'J':
				cmd = IDC_MAILBOX_MATCHSUBJECT;
				break;
			case 'a':
			case 'A':
				cmd = IDC_MAILBOX_MATCHDATE;
				break;
			case 'u':
			case 'U':
				// Toggle toolbar button state to turn off match
				if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch))
					cmd = IDC_TOOLBARMAILBOXMATCHBTN;
				else
					cmd = IDC_TOOLBARMAILBOXMATCHOPTIONBTN;
				break;
			default:
				// Beep for illegal mode character
				::MessageBeep(-1);
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

		switch(nChar)
		{
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
			cmd = ID_EDIT_SELECT_ALL;
			break;

		case 'd':
		case 'D':
			cmd = IDM_MESSAGES_DELETE;
			break;

		case 's':
		case 'S':
			cmd = IDM_FLAGS_SEEN;
			break;

		case 'i':
		case 'I':
			cmd = IDM_FLAGS_IMPORTANT;
			break;

		case 'e':
		case 'E':
			cmd = IDM_MAILBOX_EXPUNGE;
			break;

		case 'l':
		case 'L':
			cmd = IDM_MAILBOX_CHECK;
			break;

		case 'y':
		case 'Y':
			cmd = IDM_MAILBOX_SEARCH;
			break;

		case 'r':
		case 'R':
			cmd = IDM_MESSAGES_REPLY;
			break;

		case 'f':
		case 'F':
			cmd = IDM_MESSAGES_FORWARD;
			break;

		case 'b':
		case 'B':
			cmd = IDM_MESSAGES_BOUNCE;
			break;

		case 'c':
		case 'C':
			cmd = IDC_MAILBOXCOPYCMD;
			break;

		case 'm':
		case 'M':
			cmd = IDC_MAILBOXMOVECMD;
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
				cmd = ID_FILE_CLOSE;
			break;

		case 'z':
		case 'Z':
			cmd = IDC_3PANETOOLBAR_ZOOMITEMS;
			break;

		case 't':
		case 'T':
			// Thread mode switch
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				mLastTyping = (1000 * clock())/CLOCKS_PER_SEC;
				mLastChar = 't';
				return true;
			}
			break;

		case 'u':
		case 'U':
			if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
			{
				// Match mode switch
				mLastTyping = (1000 * clock())/CLOCKS_PER_SEC;
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
		CCmdUIChecker cmdui;
		cmdui.m_nID = cmd;
		
		// Only if commander available
		cmdui.DoUpdate(this, true);
		
		// Execute if enabled
		if (cmdui.GetEnabled())
			SendMessage(WM_COMMAND, cmd);

		return true;
	}

	// Pass up to super commander
	return CMailboxTable::HandleChar(nChar, nRepCnt, nFlags);
}

#pragma mark ____________________________________Command Updaters

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

void CMailboxInfoTable::OnUpdateMailboxClearDisconnect(CCmdUI* pCmdUI)
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
	CApplyRulesMenu::ResetMenuList();
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
	// Not if locked by admin
	if (CAdminLock::sAdminLock.mAllowRejectCommand)
		OnUpdateSelectionNotDeleted(pCmdUI);
	else
		pCmdUI->Enable(false);
}

void CMailboxInfoTable::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Must be one undeleted and smart
	pCmdUI->Enable((!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue()) &&
						TestSelectionIgnoreOr(&CMailboxInfoTable::TestSelectionSmart));
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
	OnUpdateFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->m_nID - IDM_FLAGS_LABEL1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->m_nID - IDM_FLAGS_LABEL1]->name.win_str());
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

void CMailboxInfoTable::OnUpdateMatchButton(CCmdUI* pCmdUI)
{
	// Button on or off
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
	pCmdUI->SetCheck(GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch));
}

#pragma mark ____________________________________Command Handlers

BOOL CMailboxInfoTable::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route some messages to the preview
	switch(nID)
	{
	case IDM_MESSAGES_READ_PREV:
	case IDC_TOOLBARMESSAGEPREVIOUSBTN:
	case IDM_MESSAGES_READ_NEXT:
	case IDC_TOOLBARMESSAGENEXTBTN:
	case IDM_MESSAGES_DELETE_READ:
	case IDC_TOOLBARMESSAGEDELETENEXTBTN:
	case IDM_MESSAGES_COPY_READ:
		if (mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview())
			return mInfoTableView->GetPreview()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		break;
	case IDM_MESSAGES_REPLY:
	case IDC_TOOLBARMESSAGEREPLYBTN:
	case IDC_TOOLBARMESSAGEREPLYOPTIONBTN:
	case IDM_MESSAGES_FORWARD:
	case IDC_TOOLBARMESSAGEFORWARDBTN:
	case IDC_TOOLBARMESSAGEFORWARDOPTIONBTN:
		// Route this through the preview if its available and displaying a message, and this table is not currently focussed
		// NB This will only happen for toolbar clicks as menu commands will be routed
		// to the preview when its focussed.
		if (!IsTarget() && mInfoTableView->GetUsePreview() && mInfoTableView->GetPreview() && mInfoTableView->GetPreview()->GetMessage())
			return mInfoTableView->GetPreview()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		break;
	
	case IDC_TOOLBARMAILBOXAPPLYRULES:
		// Check for execution of Apply Rules button
		if (nCode == CN_COMMAND)
		{
			OnApplyRuleMailboxToolbar(static_cast<const char*>(pExtra));
			return true;
		}
		break;
	default:;
	}

	// Do inherited
	return CMailboxTable::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMailboxInfoTable::OnFileNewDraft()
{
	DoNewLetter(::GetKeyState(VK_MENU) < 0);
}

#pragma mark ____________________________Commands

void CMailboxInfoTable::OnApplyRuleMailbox(UINT index)
{
	// Only if not locked out
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	DoApplyRuleMailbox(index == IDM_APPLY_RULES_ALL ? -1 : index - IDM_APPLY_RULESStart);
}

void CMailboxInfoTable::OnMessageReply()
{
	DoMessageReply(replyReplyTo, (::GetKeyState(VK_MENU) < 0));
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
	DoMessageForward(::GetKeyState(VK_MENU) < 0);
}

void CMailboxInfoTable::OnFlagsSeen()
{
	DoFlagMailMessage(NMessage::eSeen);
}

void CMailboxInfoTable::OnFlagsAnswered()
{
	DoFlagMailMessage(NMessage::eAnswered);
}

void CMailboxInfoTable::OnFlagsImportant()
{
	DoFlagMailMessage(NMessage::eFlagged);
}

void CMailboxInfoTable::OnFlagsDraft()
{
	DoFlagMailMessage(NMessage::eDraft);
}

void CMailboxInfoTable::OnFlagsLabel(UINT nID)
{
	DoFlagMailMessage(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (nID - IDM_FLAGS_LABEL1)));
}

void CMailboxInfoTable::OnMatch()
{
	// Toggle match mode
	OnMatchButton(::GetKeyState(VK_MENU) < 0);
}

void CMailboxInfoTable::OnMatchOption()
{
	// Toggle match mode
	OnMatchButton(true);
}

// Check whether drag item is acceptable
bool CMailboxInfoTable::ItemIsAcceptable(COleDataObject* pDataObject)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);
	
	// Do not allow message drop on read-only mailbox
	if ((theFlavor == CMulberryApp::sFlavorMsgList) &&
		GetMbox() && GetMbox()->IsReadOnly())
		return false;

	return CMailboxTable::ItemIsAcceptable(pDataObject);
}

