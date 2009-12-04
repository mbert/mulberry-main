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


// Source for CMessageWindow class

#include "CMessageWindow.h"

#include "CActionManager.h"
#include "CAddress.h"
#include "CAddressList.h"
#include "CAddressText.h"
#include "CAdminLock.h"
#include "CBetterScrollerX.h"
#include "CBodyTable.h"
#include "CCaptionParser.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCopyToMenu.h"
#include "CDragTasks.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CFormattedTextDisplay.h"
#include "CHeadAndFoot.h"
#include "CLetterWindow.h"
#include "CMailboxToolbarPopup.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageSimpleView.h"
#include "CMessageToolbar.h"
#include "CMulberryCommon.h"
#include "CMulberryApp.h"
#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CNetworkException.h"
#include "COptionClick.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CResources.h"
#include "CRFC822.h"
#include "CSpeechSynthesis.h"
#include "CSpaceBarAttachment.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CTextFieldX.h"
#include "CToolbarView.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"
#include "CWebKitControl.h"
#include "CWindowsMenu.h"

#include <LDisclosureTriangle.h>
#include <LBevelButton.h>
#include <LIconControl.h>
#include <LPopupButton.h>
#include "MyCFString.h"

#include <UGAColorRamp.h>
#include <UStandardDialogs.h>
#include <UGraphicUtils.h>

#include <stdio.h>
#include <string.h>
#include <strstream>

//#define USE_HTML

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E W I N D O W
// __________________________________________________________________________________________________

// Static members


cdmutexprotect<CMessageWindow::CMessageWindowList> CMessageWindow::sMsgWindows;
cdstring CMessageWindow::sNumberString;
cdstring CMessageWindow::sNumberStringBig;
cdstring CMessageWindow::sSubMessageString;
cdstring CMessageWindow::sDigestString;
cdstring CMessageWindow::sLastCopyTo;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageWindow::CMessageWindow()
#if PP_Target_Carbon
		: LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
		: LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitMessageWindow();
}

// Constructor from stream
CMessageWindow::CMessageWindow(LStream *inStream)
		: LWindow(inStream),
#if PP_Target_Carbon
		  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
		  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitMessageWindow();
}

// Default destructor
CMessageWindow::~CMessageWindow()
{
	// Set status
	SetClosing();

	// Remove from list
	{
		cdmutexprotect<CMessageWindowList>::lock _lock(sMsgWindows);
		CMessageWindowList::iterator found = std::find(sMsgWindows->begin(), sMsgWindows->end(), this);
		if (found != sMsgWindows->end())
			sMsgWindows->erase(found);
	}
	CWindowsMenu::RemoveWindow(this);

	// Delete multi-op list
	delete mMsgs;
	mMsgs = NULL;

	// Set status
	SetClosed();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Default constructor
void CMessageWindow::InitMessageWindow()
{
	mItsMsg = NULL;
	mWasUnseen = false;
	mMsgs = NULL;
	mItsMsgError = false;
	mShowText = NULL;
	mCurrentPart = NULL;
	mPrintSpec = NULL;
	mShowHeader = CPreferences::sPrefs->showMessageHeader.GetValue();
	mShowSecure = true;
	mSecureMulti = true;
	mParsing = eViewFormatted;
	mFontScale = 0;
	mQuoteDepth = -1;

	mHeaderState.mExpanded = true;
	mHeaderState.mFromVisible = true;
	mHeaderState.mFromExpanded = false;
	mHeaderState.mToVisible = true;
	mHeaderState.mToExpanded = false;
	mHeaderState.mCcVisible = true;
	mHeaderState.mCcExpanded = false;
	mHeaderState.mSubjectVisible = true;

	mRedisplayBlock = false;
	mAllowDeleted = false;
	mDidExpandParts = false;
	mDeleteBtnActive = false;

	// Add to list
	{
		cdmutexprotect<CMessageWindowList>::lock _lock(sMsgWindows);
		sMsgWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this);
}

// Get details of sub-panes
void CMessageWindow::FinishCreateSelf(void)
{

	// Do inherited
	LWindow::FinishCreateSelf();

	// Get static test
	if (sNumberString.empty()) {
		sNumberString.FromResource("UI::Message::MessageNumber");
	}
	if (sNumberStringBig.empty()) {
		sNumberStringBig.FromResource("UI::Message::MessageNumberBig");
	}
	if (sSubMessageString.empty())
	{
		sSubMessageString.FromResource("UI::Message::SubMessage");
	}
	if (sDigestString.empty())
	{
		sDigestString.FromResource("UI::Message::DigestMessage");
	}

	// Gray header
	mHeaderMove = (LView*) FindPaneByID(paneid_MessageHeader);

	// Get toolbar and main view
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_MessageToolbarView);
	mView = (CMessageSimpleView*) FindPaneByID(paneid_MessageView);

	// Create toolbars for a view we own
	mToolbarView->SetSibling(mView);
	mView->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set Help
	mFromMove = (LView*) FindPaneByID(paneid_MessageFromMove);

	mFromTwister = (LDisclosureTriangle*) FindPaneByID(paneid_MessageFromTwister);

	mFromEdit = (CAddressText*) FindPaneByID(paneid_MessageFromX);
	mFromEdit->SetHardWrap(true);
	//mFromEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mFromEdit->GetSuperView())->ShowVerticalScrollBar(false);

	mToMove = (LView*) FindPaneByID(paneid_MessageToMove);

	mToTwister = (LDisclosureTriangle*) FindPaneByID(paneid_MessageToTwister);

	mToEdit = (CAddressText*) FindPaneByID(paneid_MessageToX);
	mToEdit->SetHardWrap(true);
	//mToEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mToEdit->GetSuperView())->ShowVerticalScrollBar(false);

	mCCMove = (LView*) FindPaneByID(paneid_MessageCCMove);

	mCCTwister = (LDisclosureTriangle*) FindPaneByID(paneid_MessageCCTwister);

	mCCEdit = (CAddressText*) FindPaneByID(paneid_MessageCCX);
	mCCEdit->SetHardWrap(true);
	//mCCEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mCCEdit->GetSuperView())->ShowVerticalScrollBar(false);

	mSubjectMove = (LView*) FindPaneByID(paneid_MessageSubjectMove);

	mSubjectField = (CTextFieldX*) FindPaneByID(paneid_MessageSubject);

	mDateField = (CTextFieldX*) FindPaneByID(paneid_MessageDate);

	mNumberField = (CStaticText*) FindPaneByID(paneid_MessageNumber);

	mAttachMove = (LView*) FindPaneByID(paneid_MessageAttachMove);

	mAttachNumberField = (CStaticText*) FindPaneByID(paneid_MessageAttachNumber);
	mFlatHierarchyBtn = (LBevelButton*) FindPaneByID(paneid_MessageFlatHierarchy);
	mFlatHierarchyBtn->Hide();

	mAttachTwister = (LDisclosureTriangle*) FindPaneByID(paneid_MessageAttachTwister);
	mAttachmentsIcon = (LIconControl*) FindPaneByID(paneid_MessageAttachmentsIcon);
	mAlternativeIcon = (LIconControl*) FindPaneByID(paneid_MessageAlternativeIcon);

	// Header button
	mHeaderBtn = (LBevelButton*) FindPaneByID(paneid_MessageBtnHeader);
	mHeaderBtn->SetValue(mShowHeader);

	mParseLevel = (LPopupButton*) FindPaneByID(paneid_MessageParseLevel);
	mParseLevel->SetValue(1);
	mParsing = eViewFormatted;

	mFontIncreaseBtn = (LBevelButton*) FindPaneByID(paneid_MessageFontIncrease);
	mFontDecreaseBtn = (LBevelButton*) FindPaneByID(paneid_MessageFontDecrease);
	mFontScaleField = (CStaticText*) FindPaneByID(paneid_MessageFontScale);

	mQuoteDepthPopup = (LPopupButton*) FindPaneByID(paneid_MessageQuoteDepth);
	mQuoteDepthPopup->SetValue(1);

	mAttachArea = (LView*) FindPaneByID(paneid_MessageAttachHide);
	mAttachTable = (CBodyTable*) FindPaneByID(paneid_MessageAttachTable);
	CContextMenuAttachment::AddUniqueContext(mAttachTable, 2004, mAttachTable);

	// Split pane
	mSplitter = (CSplitterView*) FindPaneByID(paneid_MessageSplit);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CMessageBtns);

	// Bottom spliiter items
	mBottomPane = (LView*) FindPaneByID(paneid_MessageBottom);
	mSecurePane = (LView*) FindPaneByID(paneid_MessageSecure);
	mSecureInfo = (CTextDisplay*) FindPaneByID(paneid_MessageSecureInfo);
	mSecureInfo->SetDefaultBackground(UGAColorRamp::GetColor(colorRamp_Gray1));

	mTextPane = (LView*) FindPaneByID(paneid_MessageScroller);

	// Set Drag & Drop pane to scroller
	mPane = mTextPane;

	// Make text edit the target
	mText = (CFormattedTextDisplay*) FindPaneByID(paneid_MessageText);
	SetLatentSub(mText);
	mText->SetTextTraits(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetFindAllowed(true);

	mHTML = (CWebKitControl*) FindPaneByID(paneid_MessageHTML);
#ifdef USE_HTML
	mHTML->Hide();
	mHTML->Disable();
#else
	delete mHTML;
#endif
	
	// Allow extended keyboard actions
	mToEdit->AddAttachment(new CSpaceBarAttachment(this, mText));
	mFromEdit->AddAttachment(new CSpaceBarAttachment(this, mText));
	mCCEdit->AddAttachment(new CSpaceBarAttachment(this, mText));
	mSubjectField->AddAttachment(new CSpaceBarAttachment(this, mText));
	mText->AddAttachment(new CSpaceBarAttachment(this, mText));
	CContextMenuAttachment::AddUniqueContext(mText, 2002, mText, false);

	ShowSecretPane(false);

	// Install the splitter items and initialise the splitter
	mSplitter->InstallViews(mAttachArea, mBottomPane, true);
	mSplitter->SetMinima(84, 64);
	mSplitter->ShowView(false, true);
	mSplitter->SetLockResize(true);

	// Reset to default size
	ResetState();

	// Set status
	SetOpen();

	CContextMenuAttachment::AddUniqueContext(this, new CCopyToContextMenuAttachment(2001, this));
}

// Handle key presses
Boolean CMessageWindow::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Get state of command modifiers (not shift)
	CKeyModifiers mods(inKeyEvent.modifiers);

	CommandT cmd = 0;
	switch (inKeyEvent.message & charCodeMask)
	{
	// Handle spacebar scroll here
	case char_Space:
	case kNonBreakingSpaceCharCode:
		if (!SpacebarScroll(mods.Get(CKeyModifiers::eShift)))
		{
			// Look for option key down
			if (mods.Get(CKeyModifiers::eAlt))
			{
				// Delete and go to next message
				OnMessageDeleteRead();
			}
			else if (mods.Get(CKeyModifiers::eShift))
			{
				// Go to previous message
				OnMessageReadPrev();
			}
			else
			{
				// Go to next message
				OnMessageReadNext();
			}
		}
		return true;

	// Delete and next
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if ((inKeyEvent.message & keyCodeMask) != vkey_Escape)
		{
			OnMessageDeleteRead();
			return true;
		}
		break;

	// Key shortcuts
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
		cmd = cmd_ReadNextMessage;
		break;

	case 'p':
	case 'P':
		cmd = cmd_ReadPrevMessage;
		break;

	case 'h':
	case 'H':
		cmd = cmd_HeaderToggleMessage;
		break;

	case 'v':
	case 'V':
		cmd = cmd_ShowParts;
		break;

	default:;
	}

	// Check for command execute
	if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() &&
			!mods.Get(CKeyModifiers::eAlt) && !mods.Get(CKeyModifiers::eCmd) && !mods.Get(CKeyModifiers::eControl) && (cmd != 0))
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

	// Do default key press processing
	return LWindow::HandleKeyPress(inKeyEvent);
}

//	Respond to commands
Boolean CMessageWindow::ObeyCommand(CommandT inCommand, void *ioParam)
{
	ResIDT	menuID;
	SInt16	menuItem;

	bool	cmdHandled = true;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Show then select the requested window
		if (CCopyToMenu::IsCopyToMenu(menuID))
		{
			CMbox* mbox = NULL;
			if (CCopyToMenu::GetMbox(menuID, menuItem, mbox))
				CopyThisMessage(mbox, COptionMenu::sOptionKey);
			cmdHandled = true;
		}
		else
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
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

		case cmd_Save:
		case cmd_ToolbarFileSaveBtn:
			SaveThisMessage();
			break;

		case cmd_PageSetup:
		{
			StPrintSession			session(*GetPrintSpec());
			StDesktopDeactivator	deactivator;
			UPrinting::AskPageSetup(*GetPrintSpec());
		}
		break;

		case cmd_Print:
		case cmd_ToolbarMessagePrintBtn:
			//  Hide status window as Print Manager gets name of top window and does not know about floats
			{
				StPrintSession	session(*GetPrintSpec());
				StStatusWindowHide hide;
				bool do_print = false;
				{
					StDesktopDeactivator	deactivator;
					do_print = UPrinting::AskPrintJob(*GetPrintSpec());
				}
				if (do_print)
					DoPrint();
			}

			// Print job status window might mess up window order
			UDesktop::NormalizeWindowOrder();
			break;

		case cmd_SpellCheck:
			OnEditSpellCheck();
			break;

		case cmd_ReadPrevMessage:
		case cmd_ToolbarMessagePreviousBtn:
			OnMessageReadPrev();
			break;

		case cmd_ReadNextMessage:
		case cmd_ToolbarMessageNextBtn:
			OnMessageReadNext();
			break;

		case cmd_DeleteReadNextMessage:
		case cmd_ToolbarMessageDeleteNextBtn:
			OnMessageDeleteRead();
			break;

		case cmd_CopyReadNextMessage:
			CopyReadNextMessage(COptionMenu::sOptionKey);
			break;

		case cmd_ReplyMessage:
			ReplyToThisMessage(replyReplyTo, COptionMenu::sOptionKey);
			break;
		case cmd_ToolbarMessageReplyBtn:
			ReplyToThisMessage(replyReplyTo, false);
			break;
		case cmd_ToolbarMessageReplyOptionBtn:
			ReplyToThisMessage(replyReplyTo, true);
			break;

		case cmd_ReplyToSenderMessage:
			ReplyToThisMessage(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ReplyToFromMessage:
			ReplyToThisMessage(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ReplyToAllMessage:
		case cmd_ToolbarMessageReplyAllBtn:
			ReplyToThisMessage(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
			break;

		case cmd_ForwardMessage:
			ForwardThisMessage(COptionMenu::sOptionKey);
			break;
		case cmd_ToolbarMessageForwardBtn:
			ForwardThisMessage(false);
			break;
		case cmd_ToolbarMessageForwardOptionBtn:
			ForwardThisMessage(true);
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

		case cmd_ToolbarMessageCopyPopup:
		case cmd_ToolbarMessageCopyOptionPopup:
		{
			CMbox* mbox = NULL;
			if (mView->GetToolbar()->GetCopyBtn() && mView->GetToolbar()->GetCopyBtn()->GetSelectedMbox(mbox, true))
				CopyThisMessage(mbox, inCommand == cmd_ToolbarMessageCopyOptionPopup);
			break;
		}

		case cmd_CopyNowMessage:
			TryCopyMessage(COptionMenu::sOptionKey);
			break;

		case cmd_ToolbarMessageCopyBtn:
			TryCopyMessage(false);
			break;

		case cmd_ToolbarMessageCopyOptionBtn:
			TryCopyMessage(true);
			break;

		case cmd_MessageCopyExplicit:
			TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue());
			break;

		case cmd_MessageMoveExplicit:
			TryCopyMessage(!CPreferences::sPrefs->deleteAfterCopy.GetValue());
			break;

		case cmd_DeleteMessage:
		case cmd_ToolbarMessageDeleteBtn:
		case cmd_FlagsDeleted:
			OnMessageDelete();
			break;

		case cmd_HeaderToggleMessage:
			// Fake hit in button
			mHeaderBtn->SetValue(!mShowHeader);
			break;

		case cmd_FlagsSeen:
			// Toggle state
			OnMessageFlags(NMessage::eSeen);
			break;

		case cmd_FlagsImportant:
			// Toggle state
			OnMessageFlags(NMessage::eFlagged);
			break;

		case cmd_FlagsAnswered:
			// Toggle state
			OnMessageFlags(NMessage::eAnswered);
			break;

		case cmd_FlagsDraft:
			// Toggle state
			OnMessageFlags(NMessage::eDraft);
			break;

		case cmd_FlagsLabel1:
		case cmd_FlagsLabel2:
		case cmd_FlagsLabel3:
		case cmd_FlagsLabel4:
		case cmd_FlagsLabel5:
		case cmd_FlagsLabel6:
		case cmd_FlagsLabel7:
		case cmd_FlagsLabel8:
			OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (inCommand - cmd_FlagsLabel1)));
			break;

		case cmd_ToolbarMessageFlagsBtn:
			switch(*((SInt32*) ioParam))
			{
			case 1:
				OnMessageFlags(NMessage::eSeen);
				break;
			case 2:
				OnMessageFlags(NMessage::eFlagged);
				break;
			case 3:
				OnMessageFlags(NMessage::eAnswered);
				break;
			case 4:
				OnMessageDelete();
				break;
			case 5:
				OnMessageFlags(NMessage::eDraft);
				break;
			default:
				OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (*((SInt32*) ioParam) - 7)));
				break;
			}
			break;

		case cmd_ViewPartsMessage:
			if (mText->IsTarget())
			{
				// View current part
				OnMessageViewCurrent();
			}
			else
			{
				mAttachTable->DoExtractParts(true);
			}
			break;

		case cmd_ExtractPartsMessage:
			mAttachTable->DoExtractParts(false);
			break;

		case cmd_DecodeBinHexMessage:
			OnMessageDecodeBinHex();
			break;

		case cmd_DecodeUUMessage:
			OnMessageDecodeUU();
			break;

		case cmd_VerifyDecrypt:
			OnMessageVerifyDecrypt();
			break;

		case cmd_SetDefaultSize:
			SaveDefaultState();
			break;

		case cmd_ResetDefaultWindow:
			ResetState();
			break;

		case cmd_ExpandWindow:
		case cmd_ToolbarCollapseHeader:
			OnHeaderExpand();
			break;

		case cmd_ShowParts:
			// Fake hit in button
			mAttachTwister->SetValue(!mAttachTwister->GetValue());
			break;

		case cmd_SpeakSelection:
			// Only do something if speech class does not handle it
			if (!CSpeechSynthesis::ObeyCommand(inCommand, ioParam))
				SpeakMessage();
			break;

		default:
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
			break;
		}
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMessageWindow::FindCommandStatus(
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
		// Check for copy to
		if (CCopyToMenu::IsCopyToMenu(menuID))
			outEnabled = true;
		else
			LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
	}
	else
	{
		switch (inCommand)
		{
		case cmd_Open:
			// Always Disabled
			outEnabled = false;
			break;

		// These are always enabled
		case cmd_Save:
		case cmd_PageSetup:
		case cmd_Print:
		case cmd_SelectAll:
		case cmd_ReplyMessage:
		case cmd_ReplyToSenderMessage:
		case cmd_ReplyToFromMessage:
		case cmd_ReplyToAllMessage:
		case cmd_ForwardMessage:
		case cmd_BounceMessage:
		case cmd_SetDefaultSize:
		case cmd_ResetDefaultWindow:
		case cmd_ToolbarFileSaveBtn:
		case cmd_ToolbarMessagePrintBtn:
		case cmd_ToolbarMessageReplyBtn:
		case cmd_ToolbarMessageReplyOptionBtn:
		case cmd_ToolbarMessageReplyAllBtn:
		case cmd_ToolbarMessageForwardBtn:
		case cmd_ToolbarMessageForwardOptionBtn:
		case cmd_ToolbarMessageBounceBtn:
			outEnabled = true;
			break;

		case cmd_RejectMessage:
		case cmd_ToolbarMessageRejectBtn:
			outEnabled = CAdminLock::sAdminLock.mAllowRejectCommand;
			break;

		case cmd_SpellCheck:
			// Must have spell checker
			outEnabled = CPluginManager::sPluginManager.HasSpelling();
			break;

		// This is only enabled if originally sent by user
		case cmd_SendAgainMessage:
		case cmd_ToolbarMessageSendAgainBtn:
			outEnabled = mItsMsg && mItsMsg->IsSmartFrom();
			break;

		// Copy to options
		case cmd_CopyToMessage:
		case cmd_CopyNowMessage:
		case cmd_ToolbarMessageCopyBtn:
		case cmd_ToolbarMessageCopyOptionBtn:
		case cmd_MessageCopyExplicit:
		case cmd_MessageMoveExplicit:
			outEnabled = (mItsMsg != NULL);
			switch(inCommand)
			{
			case cmd_CopyToMessage:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveToMailbox : str_CopyToMailbox);
				::PLstrcpy(outName, txt1);
				break;
			}
			case cmd_CopyNowMessage:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveNow : str_CopyNow);
				::PLstrcpy(outName, txt1);
				break;
			}
			case cmd_ToolbarMessageCopyBtn:
			case cmd_ToolbarMessageCopyOptionBtn:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveTo : str_CopyTo);
				::PLstrcpy(outName, txt1);
				break;
			}
			default:;
			}
			break;

		case cmd_ReadPrevMessage:
		case cmd_ToolbarMessagePreviousBtn:
			if (mItsMsg &&
				(!mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true) ||
				 mItsMsg->IsSubMessage() && mItsMsg->GetPrevDigest()))
				outEnabled = true;
			else
				outEnabled = false;
			break;

		case cmd_ReadNextMessage:
		case cmd_ToolbarMessageNextBtn:
			if (mItsMsg &&
				(!mItsMsg->IsSubMessage() ||
				 mItsMsg->IsSubMessage() && mItsMsg->GetNextDigest()))
			{
				outEnabled = true;

				// Update icon as well
				if (inCommand == cmd_ToolbarMessageNextBtn)
				{
					ResIDT icon = 0;
					if (mItsMsg->IsSubMessage())
					{
						// Reset navigation buttons
						icon = mItsMsg->GetNextDigest() ? ICNx_NextMsg : ICNx_LastNextMsg;
					}
					else
					{
						// Reset navigation buttons
						icon = mItsMsg->GetMbox()->GetNextMessage(mItsMsg, true) ? ICNx_NextMsg : ICNx_LastNextMsg;
					}
					if (mView->GetToolbar()->GetReadNextBtn())
					{
						ControlButtonContentInfo cinfo;
						cinfo.contentType = kControlContentIconSuiteRes;
						cinfo.u.resID = icon;
						mView->GetToolbar()->GetReadNextBtn()->SetDataTag(0, kControlBevelButtonContentTag, sizeof(ControlButtonContentInfo), &cinfo);
					}
				}
			}
			else
				outEnabled = false;
			break;

		case cmd_DeleteReadNextMessage:
		case cmd_ToolbarMessageDeleteNextBtn:
			outEnabled = mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eDeleted);
			break;

		case cmd_FlagsMessage:
		case cmd_ToolbarMessageFlagsBtn:
			outEnabled = mItsMsg && !mItsMsg->IsSubMessage();
			break;

		case cmd_CopyReadNextMessage:
			outEnabled = (mItsMsg != NULL);
			break;

		case cmd_ViewPartsMessage:
		case cmd_ExtractPartsMessage:
			if (mText->IsTarget())
			{
				// Must have a visible part
				outEnabled = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
			}
			else
			{
				// Is list not empty?
				STableCell aCell(0, 0);
				outEnabled = mAttachTable->GetNextSelectedCell(aCell);
			}
			break;

		case cmd_DecodeEmbeddedMessage:
		case cmd_DecodeBinHexMessage:
		case cmd_DecodeUUMessage:
			// Only if text available
			outEnabled = mItsMsg && mItsMsg->HasText();
			break;

		case cmd_VerifyDecrypt:
			// Only if text available
			outEnabled = CPluginManager::sPluginManager.HasSecurity();
			break;

		case cmd_DeleteMessage:
		case cmd_ToolbarMessageDeleteBtn:
			{
				// Only if not sub-message and not read only
				outEnabled = mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eDeleted);
				LStr255 txt1(STRx_Standards, mItsMsg && mItsMsg->IsDeleted() ? str_Undelete : str_Delete);
				::PLstrcpy(outName, txt1);
				if (outEnabled && (inCommand == cmd_ToolbarMessageDeleteBtn))
				{
					outUsesMark = true;
					outMark = mItsMsg->IsDeleted() ? (UInt16)checkMark : (UInt16)noMark;
				}
			}
			break;

		case cmd_HeaderToggleMessage:
			{
				// Always enabled but text may change
				outEnabled = true;
				LStr255 txt2(STRx_Standards, !mShowHeader ? str_ShowHeader : str_HideHeader);
				::PLstrcpy(outName, txt2);
			}
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

		case cmd_ExpandWindow:
		case cmd_ToolbarCollapseHeader:
			{
				// Always enabled but text may change
				outEnabled = true;
				LStr255 txt2(STRx_Standards, mHeaderState.mExpanded ? str_CollapseHeader : str_ExpandHeader);
				::PLstrcpy(outName, txt2);
			}
			break;

		case cmd_ShowParts:
			{
				// Always enabled but text may change
				outEnabled = true;
				LStr255 txt2(STRx_Standards, !mAttachTwister->GetValue() ? str_ShowParts : str_HideParts);
				::PLstrcpy(outName, txt2);
			}
			break;

		case cmd_Toolbar:
			// Always enabled
			outEnabled = true;
			break;

		case cmd_SpeakSelection:
			if (!CSpeechSynthesis::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName))
			{
				outEnabled = true;
				::PLstrcpy(outName, LStr255(STRx_Speech, str_SpeakMessage));
			}
			break;

		default:
			LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
		}
	}
}

void CMessageWindow::FindFlagCommandStatus(
	Boolean				&outEnabled,
	Boolean				&outUsesMark,
	UInt16				&outMark,
	NMessage::EFlags	flag)
{
	outEnabled = mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(flag);
	outUsesMark = outEnabled;
	if (mItsMsg && mItsMsg->HasFlag(flag))
		outMark = checkMark;
	else
		outMark = noMark;
}

// Respond to clicks in the icon buttons
void CMessageWindow::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_HeaderMessage:
		// Toggle header display
		mShowHeader = !mShowHeader;

		// Reset this message
		ResetText();
		break;

	case msg_FontUp:
		OnIncreaseFont();
		break;

	case msg_FontDown:
		OnDecreaseFont();
		break;

	case msg_MTwistFrom:
		OnFromTwist();
		break;

	case msg_MTwistTo:
		OnToTwist();
		break;

	case msg_MTwistCC:
		OnCcTwist();
		break;

	case msg_MTwistAttach:
		DoPartsTwist();
		break;

	case msg_MFlatHierarchy:
		DoPartsFlat(*(long*) ioParam);
		break;

	case msg_View:
		{
			EView old_view = mParsing;
			EView new_view = (EView) *((SInt32*) ioParam);
			OnViewAs(new_view);

			// Check for failure to change and reset popup to old value
			if (mParsing == old_view)
				mParseLevel->SetValue(mParsing);

			// Turn off quote depth if not formatted mode
			if (mParsing != eViewFormatted)
				mQuoteDepthPopup->Disable();
			else
				mQuoteDepthPopup->Enable();
		}
		break;

	case msg_QuoteDepth:
		OnQuoteDepth((EQuoteDepth) *((SInt32*) ioParam));
		break;
	}
}

// Make sure toolbar is activated toos
void CMessageWindow::Activate()
{
	LWindow::Activate();

	mView->RefreshToolbar();
}

// Add address list to caption/field:
void CMessageWindow::SetTwistList(CAddressText* aEdit,
									LDisclosureTriangle* aTwister,
									CAddressList* list)
{

	// Policy: always fit address text into full horizontal width with wrapping
	// If there is more than one line of text enable the twist widget
	// If there is more than one address expand the field if prefs allow

	cdstring to_addrs;

	// Add all in list
	bool first = true;
	bool got_two = false;
	for(CAddressList::iterator iter = list->begin(); iter != list->end(); iter++)
	{
		cdstring txt = (*iter)->GetFullAddress();
		if (!first)
			to_addrs += os_endl;
		to_addrs += txt;
		if (first)
		{
			first = false;
		}
		else
			got_two = true;
	}

	// Update field - will null if no addresses
	aEdit->SetText(to_addrs);

	// Disable twist if only one line
	if (aEdit->GetLineCount() < 2)
		aTwister->Disable();
	else
		aTwister->Enable();

	// Force twist down if more than one address and preference set
	if (CPreferences::sPrefs->mExpandHeader.GetValue())
		aTwister->SetValue(got_two);
}

// Set caption texts
void CMessageWindow::SetMessage(CMessage* theMsg)
{
	ThrowIfNil_(theMsg);

	// Reset message and current part
	mItsMsg = theMsg;
	mWasUnseen = mItsMsg ? mItsMsg->IsUnseen() : false;
	mCurrentPart = NULL;

	// Reset the error flag
	mItsMsgError = false;

	// Reset any previous cached raw body
	mRawUTF16Text.reset(NULL);

	// Get new envelope
	CEnvelope*	theEnv = mItsMsg ? mItsMsg->GetEnvelope() : NULL;
	ThrowIfNil_(theEnv);
	cdstring	theTitle;

	// Set To: text
	SetTwistList(mToEdit, mToTwister, theEnv->GetTo());

	// Set From: text
	SetTwistList(mFromEdit, mFromTwister, theEnv->GetFrom());

	// Set CC: text
	SetTwistList(mCCEdit, mCCTwister, theEnv->GetCC());

	// Set Subject: text
	mSubjectField->SetText(theEnv->GetSubject());

	// Set Date: text
	mDateField->SetText(theEnv->GetTextDate(true, true));

	// Set window title to subject (if there), else from (if there) else unknown
	if (!theEnv->GetSubject().empty())
	{
		short subj_len = theEnv->GetSubject().length();
		if (subj_len < kMaxTitleLength)
			theTitle= theEnv->GetSubject();
		else
			theTitle = cdstring(theEnv->GetSubject(), 0, kMaxTitleLength - 1);
	}
	else if (theEnv->GetFrom()->size())
	{
		theTitle = theEnv->GetFrom()->front()->GetNamedAddress();
		theTitle.erase(kMaxTitleLength);
	}
	else
		theTitle.FromResource("UI::Message::NoSubject");

	MyCFString cftext(theTitle.c_str(), kCFStringEncodingUTF8);
	SetCFDescriptor(cftext);

	// Make sure entry in Window menu is updated with new title
	CWindowsMenu::RenamedWindow();

	// Allow deleted messages that appear as message is read in
	mAllowDeleted = true;

	// Check for previous verify/decrypt status first (to avoid doing it again)
	// If there was a bad passphrase error, skip the cached data so user has a chance to enter
	// the correct passphrase next time
	if ((mItsMsg->GetCryptoInfo() != NULL) && !mItsMsg->GetCryptoInfo()->GetBadPassphrase())
	{
		// Show the secure info pane
		SetSecretPane(*mItsMsg->GetCryptoInfo());
		ShowSecretPane(true);
	}

	// Now check for auto verify/decrypt
	else if (CPreferences::sPrefs->mAutoVerify.GetValue() &&
		mItsMsg->GetBody()->IsVerifiable() ||
		CPreferences::sPrefs->mAutoDecrypt.GetValue() &&
		mItsMsg->GetBody()->IsDecryptable())
	{
		// NULL out current part as its used in VerifyDecrypt
		mCurrentPart = NULL;
		OnMessageVerifyDecrypt();
	}
	
	// Not verify/decrypt
	else
		// Turn of secure info pane
		ShowSecretPane(false);

	// Reset attachment list
	UpdatePartsList();

	// Reset text to first text part and reset table display
	CAttachment* attach = mItsMsg->FirstDisplayPart();
	ShowPart(attach);
	mAttachTable->SetRowShow(attach);

	// If raw mode read in raw body
	if (mParsing == eViewAsRaw)
	{
		// Handle cancel or failure of raw mode
		if (!ShowRawBody())
			mParsing = eViewFormatted;
		else
			ResetText();
	}

	// Do attachments icon
	if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1))
	{
		if (mItsMsg->GetBody()->HasUniqueTextPart())
		{
			mAttachmentsIcon->Hide();
			mAlternativeIcon->Show();
		}
		else
		{
			mAttachmentsIcon->Show();
			mAlternativeIcon->Hide();
		}
	}
	else
	{
		mAttachmentsIcon->Hide();
		mAlternativeIcon->Hide();
	}

	// Update window features
	MessageChanged();

	// No longer allow deleted
	mAllowDeleted = false;

	// Do final set message processing
	PostSetMessage();
}

// Processing after message set and displayed
void CMessageWindow::PostSetMessage()
{
	// Only do this if visible
	if (IsVisible())
	{
		// Do parts expand if no visible part or multiparts
		if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
		{
			// Do auto expansion or expand if no parts
			if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
				(mAttachTwister->GetValue() != 1))
			{
				mAttachTwister->SetValue(1);
				mDidExpandParts = true;

				// Must switch focus back to text as expanding parts sets it to the pasrt table
				SwitchTarget(mText);
			}
		}
		else
		{
			// Do auto collapse
			if (CPreferences::sPrefs->mExpandParts.GetValue() &&
				(mAttachTwister->GetValue() == 1) && mDidExpandParts)
			{
				mAttachTwister->SetValue(0);
				mDidExpandParts = false;
			}

		}

		// Set seen flag if no attachments
		if (!mCurrentPart && mItsMsg && mItsMsg->IsUnseen())
			mItsMsg->ChangeFlags(NMessage::eSeen, true);

		// Do attachments announcement
		if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniqueTextPart())
		{
			// Do attachment notification
			const CNotification& notify = CPreferences::sPrefs->mAttachmentNotification.GetValue();
			if (notify.DoPlaySound())
				::PlayNamedSound(notify.GetSoundID());
			if (notify.DoSpeakText())
				CSpeechSynthesis::SpeakString(notify.GetTextToSpeak());
			if (notify.DoShowAlert())
				CErrorHandler::PutNoteAlertRsrc("Alerts::Message::MessageHasAttachments", !notify.DoPlaySound());
		}

		// Look for top-level multipart/digest
		if (CPreferences::sPrefs->mAutoDigest.GetValue() && mItsMsg)
		{
			CAttachment* digest = mItsMsg->GetBody()->GetFirstDigest();
			if (digest)
			{
				Rect dummy = {0, 0, 0, 0};
				ShowSubMessage(digest, dummy);
			}
		}

		// If seen state has changed do actions associated with that
		if (mItsMsg && mWasUnseen && !mItsMsg->IsUnseen())
			CActionManager::MessageSeenChange(mItsMsg);
	}
}

void CMessageWindow::ClearMessage()
{
	// Remove the reference to it here
	mItsMsg = NULL;
	mItsMsgError = true;

	// Remove the reference to it in the attachment table
	mAttachTable->ClearBody();
}

// Someone else changed this message
void CMessageWindow::MessageChanged(void)
{
	// Do not process redisplay if blocked
	if (mRedisplayBlock)
		return;

	// Close window if message deleted
	if (!mAllowDeleted && mItsMsg && mItsMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
	{
		// Close window via queued task
		CCloseMessageWindowTask* task = new CCloseMessageWindowTask(this);
		task->Go();
		return;
	}

	// Set message number text or sub-message information
	cdstring newTxt;
	if (mItsMsg)
	{
		if (mItsMsg->IsSubMessage())
		{
			char temp[64];
			if (mItsMsg->CountDigest())
				::snprintf(temp, 64, sDigestString, mItsMsg->GetDigestNumber(), mItsMsg->CountDigest());
			else
				::snprintf(temp, 64, sSubMessageString);
			newTxt = temp;
		}
		else
		{
			char temp[64];
			::snprintf(temp, 64, mItsMsg->GetMbox()->GetNumberFound() < 10000 ? sNumberString : sNumberStringBig,
						mItsMsg->GetMessageNumber(), mItsMsg->GetMbox()->GetNumberFound());
			newTxt = temp;
		}
	}
	if (newTxt != mNumberField->GetText())
		mNumberField->SetText(newTxt);

	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Show sub-message window
void CMessageWindow::ShowSubMessage(CAttachment* attach, Rect zoom_from)
{
	// Check that its a message part
	if (!attach->IsMessage())
		return;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Create the message window
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessage(attach->GetMessage());

		// Stagger relative to this
		Rect frame;
		CalcLocalFrameRect(frame);
		LocalToPortPoint(topLeft(frame));
		PortToGlobalPoint(topLeft(frame));
		::OffsetRect(&frame, 20, 20);
		newWindow->DoSetPosition(topLeft(frame));

		if (!::EmptyRect(&zoom_from))
		{
			Rect zoom_to;
			newWindow->CalcLocalFrameRect(zoom_to);
			newWindow->LocalToPortPoint(topLeft(zoom_to));
			newWindow->LocalToPortPoint(botRight(zoom_to));
			newWindow->PortToGlobalPoint(topLeft(zoom_to));
			newWindow->PortToGlobalPoint(botRight(zoom_to));

			::ZoomRects(&zoom_from, &zoom_to, 8, kZoomAccelerate);
		}

		newWindow->Show();

		attach->SetSeen(true);
	}
	catch (CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Must throw out if disconnected and window close
		if (ex.disconnected() || ex.reconnected())
		{
			CLOG_LOGRETHROW;
			throw;
		}

		// Window failed to be created
		if (newWindow)
			FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Window failed to be created
		if (newWindow)
			FRAMEWORK_DELETE_WINDOW(newWindow)
	}
}

// Set details for decrypt/verify
void CMessageWindow::SetSecretPane(const CMessageCryptoInfo& info)
{
	mSecureInfo->FocusDraw();
	bool multi_line = false;

	{
		StStopRedraw noDraw(mSecureInfo);

		// Determine actual view type and content
		TextTraitsRecord txtr;
		UTextTraits::LoadTextTraits(261, txtr);

		// Delete any previous text
		mSecureInfo->WipeText(txtr);

		if (info.GetSuccess())
		{
			if (info.GetDidSignature())
			{
				cdstring txt;
				if (info.GetSignatureOK())
				{
					txt += "Signature: OK";
					RGBColor green = {0x0000, 0xDDDD, 0x0000};
					mSecureInfo->SetFontColor(green);
				}
				else
				{
					txt += "Signature: Bad";
					RGBColor red = {0xDDDD, 0x0000, 0x0000};
					mSecureInfo->SetFontColor(red);
				}
				mSecureInfo->SetFontStyle(bold);
				mSecureInfo->InsertUTF8(txt);

				RGBColor black = {0x0000, 0x0000, 0x0000};
				mSecureInfo->SetFontColor(black);
				txt = "    Signed By: ";
				mSecureInfo->InsertUTF8(txt);

				mSecureInfo->SetFontStyle(normal);

				cdstring addr;
				bool matched_from = false;
				for(cdstrvect::const_iterator iter = info.GetSignedBy().begin(); iter != info.GetSignedBy().end(); iter++)
				{
					// Add text
					if (iter != info.GetSignedBy().begin())
						addr += ", ";
					addr += *iter;
					
					// Determine whether item matches from address
					if (mItsMsg && mItsMsg->GetEnvelope() &&
						(mItsMsg->GetEnvelope()->GetFrom()->size() > 0) &&
						mItsMsg->GetEnvelope()->GetFrom()->front()->StrictCompareEmail(CAddress(*iter)))
					{
						// Only show the address of the one that matches - ignore others
						matched_from = true;
						addr = *iter;
						break;
					}
				}
				
				// Change colour if n matching from address
				if (!matched_from)
				{
					RGBColor red = {0xDDDD, 0x0000, 0x0000};
					mSecureInfo->SetFontColor(red);
					
					addr += " WARNING: Does not match From address";
				}

				// Insert address data
				mSecureInfo->InsertUTF8(addr);

				// Next line
				if (info.GetDidDecrypt())
				{
					mSecureInfo->InsertUTF8(os_endl, os_endl_len);
					multi_line = true;
				}
			}

			if (info.GetDidDecrypt())
			{
				mSecureInfo->SetFontStyle(bold);
				RGBColor green = {0x0000, 0xDDDD, 0x0000};
				mSecureInfo->SetFontColor(green);

				cdstring txt;
				txt += "Decrypted: OK";
				mSecureInfo->InsertUTF8(txt);

				RGBColor black = {0x0000, 0x0000, 0x0000};
				mSecureInfo->SetFontColor(black);
				txt = "    Encrypted To: ";
				mSecureInfo->InsertUTF8(txt);

				mSecureInfo->SetFontStyle(normal);

				cdstring addr;
				for(cdstrvect::const_iterator iter = info.GetEncryptedTo().begin(); iter != info.GetEncryptedTo().end(); iter++)
				{
					if (iter != info.GetEncryptedTo().begin())
						addr += ", ";
					addr += *iter;
				}
				mSecureInfo->InsertUTF8(addr);
			}
		}
		else
		{
			TextStyle ts;
			ts.tsFace = bold;
			RGBColor red = {0xDDDD,0x0000,0x0000};
			mSecureInfo->SetFontColor(red);

			cdstring txt;
			txt += "Failed to Verify/Decrypt";
			if (!info.GetError().empty())
			{
				txt+= ":   ";
				txt += info.GetError();
			}
			mSecureInfo->InsertUTF8(txt);
		}
	}

	// Finally force redraw with text scrolled to top-left
	mSecureInfo->SetSelection(0, 0);
	mSecureInfo->ScrollImageTo(0, 0, true);
	mSecureInfo->Refresh();

	// Check multi-line state
	if (mSecureMulti ^ multi_line)
	{
		const SInt16 moveby = 14;

		if (multi_line)
		{
			if (mShowSecure)
			{
				mTextPane->ResizeFrameBy(0, -moveby, false);
				mTextPane->MoveBy(0, moveby, true);
#ifdef USE_HTML
				mHTML->ResizeFrameBy(0, -moveby, false);
				mHTML->MoveBy(0, moveby, true);
#endif
			}
			mSecurePane->ResizeFrameBy(0, moveby, true);
		}
		else
		{
			mSecurePane->ResizeFrameBy(0, -moveby, true);
			if (mShowSecure)
			{
				mTextPane->ResizeFrameBy(0, moveby, false);
				mTextPane->MoveBy(0, -moveby, true);
#ifdef USE_HTML
				mHTML->ResizeFrameBy(0, moveby, false);
				mHTML->MoveBy(0, -moveby, true);
#endif
			}
		}

		mSecureMulti = multi_line;
	}
}

// Set a print record handle
void CMessageWindow::SetPrintSpec(LPrintSpec* printSpec)
{
	// Don't delete it if we're setting it to NULL
	if (mPrintSpec && printSpec)
		delete mPrintSpec;
	mPrintSpec = printSpec;
}

// Reset message text
void CMessageWindow::ResetText(void)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Start cursor for busy operation
	CWaitCursor wait;

	// Set message text
	{

		mText->FocusDraw();

		StStopRedraw noDraw(mText);

		// Determine actual view type and content
		bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
		EView actual_view = mParsing;
		bool actual_styles = CPreferences::sPrefs->mUseStyles.GetValue();
		EContentSubType actual_content = use_part ? mCurrentPart->GetContent().GetContentSubtype() : eContentSubPlain;
		i18n::ECharsetCode charset = use_part ? mCurrentPart->GetContent().GetCharset() : i18n::eUSASCII;
		const TextTraitsRecord* actual_traits = NULL;
		switch(mParsing)
		{
		case eViewFormatted:
			// Use plain text font when not using styles
			bool html = ((actual_content == eContentSubEnriched) || (actual_content == eContentSubHTML)) && actual_styles;
			actual_traits = html ? &CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits : &CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits;
			break;
		case eViewPlain:
		case eViewRaw:
		case eViewAsRaw:
			actual_traits = &CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits;
			break;
		case eViewAsHTML:
			actual_view = eViewFormatted;
			actual_styles = true;				// Force style rendering on
			actual_content = eContentSubHTML;
			actual_traits = &CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits;
			break;
		case eViewAsEnriched:
			actual_view = eViewFormatted;
			actual_styles = true;				// Force style rendering on
			actual_content = eContentSubEnriched;
			actual_traits = &CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits;
			break;
		case eViewAsFixedFont:
			actual_view = eViewFormatted;
			actual_traits = &CPreferences::sPrefs->mFixedTextTraits.GetValue().traits;
			break;
		default:;
		}

		// Adjust font size by scale factor
		TextTraitsRecord mod_traits = *actual_traits;
		if (mFontScale)
			mod_traits.size = std::max(1.0, pow(1.2, mFontScale) * mod_traits.size);

		// Delete any previous text
		mText->WipeText(mod_traits);

		// Reset the parser's data
		mText->Reset(true, mFontScale);

		// Copy in header if required
		if (mShowHeader)
			mText->ParseHeader(mItsMsg->GetHeader(), actual_view);
		else if (!mHeaderState.mExpanded)
		{
			// Get summary from envelope
			std::ostrstream hdr;
			mItsMsg->GetEnvelope()->GetSummary(hdr);
			hdr << std::ends;

			// Parse as header
			mText->ParseHeader(hdr.str(), actual_view);
			hdr.freeze(false);
		}

		// Only do if message still exists
		if (!mItsMsg)
			return;

		// Copy in text
		if ((mParsing == eViewAsRaw) && mRawUTF16Text.get())
			mText->ParseBody(mRawUTF16Text.get(), actual_content, actual_view, mQuoteDepth);
		else if (mCurrentPart && mUTF16Text.get())
			mText->ParseBody(mUTF16Text.get(), actual_content, actual_view, mQuoteDepth, actual_styles);
		else if (!mCurrentPart)
		{
			// Add special text if there are no displayable parts
			cdstring no_displayable;
			no_displayable.FromResource("UI::Message::NoDisplayablePart");
			cdustring no_displayable_utf16(no_displayable);
			mText->ParseBody(no_displayable_utf16, eContentSubPlain, actual_view, mQuoteDepth);
		}

		mText->InsertFormatted(actual_view);

		// Set selection at start
		mText->SetSelectionRange(0, 0);

#ifdef USE_HTML
		if ((actual_content == eContentSubHTML) && (actual_view == eViewFormatted))
		{
			mHTML->Show();
			mHTML->Enable();
			mTextPane->Hide();
			cdustring data(mUTF16Text.get());
			mHTML->SetData(data.ToUTF8());
		}
		else
		{
			mHTML->Hide();
			mHTML->Disable();
			mTextPane->Show();
		}
#endif
	}

	// Finally force redraw with text scrolled to top
	mText->ScrollImageTo(0, 0, true);
	mText->Refresh();

	// Make it active (not printing)
	SwitchTarget(mText);
}

// Reset font scale text
void CMessageWindow::ResetFontScale()
{
	// Set text
	cdstring txt;
	txt.reserve(32);
	if (mFontScale != 0)
		::snprintf(txt.c_str_mod(), 32, "%+d", mFontScale);
	mFontScaleField->SetText(txt);

	// Enable/disable controls
	if (mFontScale < 7)
		mFontIncreaseBtn->Enable();
	else
		mFontIncreaseBtn->Disable();
	if (mFontScale > -5)
		mFontDecreaseBtn->Enable();
	else
		mFontDecreaseBtn->Disable();
}

#pragma mark ____________________________Commands

// Save message to file
void CMessageWindow::SaveThisMessage()
{
	CMessage*			saveMsg = (mMsgs ? mMsgs->front() : mItsMsg);

	// Only do if message exists
	if (!saveMsg)
		return;

	// Get current subject as name - filter colons
	cdstring saveAsName(saveMsg->GetEnvelope()->GetSubject());
	::strreplace(saveAsName.c_str_mod(), "/", '_');

	// Do standard save as dialog
	MyCFString cfstr(saveAsName, kCFStringEncodingUTF8);
	PPx::FSObject fspec;
	bool replacing;
	if (PP_StandardDialogs::AskSaveFile(cfstr, kMessageFileType, fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))
	{
		// Fix for window manager bug after a replace operation
		Activate();

		// Now save file
		DoSave(fspec, kMessageFileType);
	}
}

// Save message
void CMessageWindow::DoSave(PPx::FSObject &inFileSpec,
								OSType inFileType)
{
	LFileStream* saveFile = NULL;

	try
	{

		// Delete any existing file
		if (inFileSpec.Exists())
			inFileSpec.Delete();

		// Create file object and specify it
		saveFile = new LFileStream(inFileSpec);

		// Create a new file of the required type & creator
		saveFile->CreateNewDataFile(*reinterpret_cast<const OSType*>(CPreferences::sPrefs->mSaveCreator.GetValue().c_str()), inFileType, smCurrentScript);
		inFileSpec.Update();

		// Open data fork for write only
		saveFile->OpenDataFork(fsWrPerm);

		// Look for multiple operations
		if (mMsgs)
		{
			// Iterate over each message and add to file
			bool first = true;
			for(CMessageList::const_iterator iter = mMsgs->begin(); iter != mMsgs->end(); iter++)
			{
				if (first)
					first = false;
				else
					saveFile->WriteBlock(CR, 1);

				// Write header if required
				if (CPreferences::sPrefs->saveMessageHeader.GetValue())
					saveFile->WriteBlock((*iter)->GetHeader(), ::strlen((*iter)->GetHeader()));

				// Write text to file stream
				bool has_data = (*iter)->HasData(0);
				mShowText = (*iter)->ReadPart();

				if (mShowText)
					saveFile->WriteBlock(mShowText, ::strlen(mShowText));

				if (!has_data)
					(*iter)->ClearMessage();
			}
		}
		else if (mItsMsg)
		{
			// Write header if required (always write it in raw mode
			if (CPreferences::sPrefs->saveMessageHeader.GetValue() || (mParsing == eViewAsRaw))
				saveFile->WriteBlock(mItsMsg->GetHeader(), ::strlen(mItsMsg->GetHeader()));

			// Write text to file stream
			const unichar_t* txt = GetSaveText();
			if (txt)
				saveFile->WriteBlock(txt, ::unistrlen(txt) * sizeof(unichar_t));
		}

		// Close it if open and delete object
		delete saveFile;
		saveFile = NULL;
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Message::NoSave", ex.GetErrorCode());

		// Delete any saved file on disk
		if (saveFile)
		{
			// Clean up - will close file
			delete saveFile;
			saveFile = NULL;

			// Then delete
			inFileSpec.Delete();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Delete any saved file on disk
		if (saveFile)
		{
			// Clean up - will close file
			delete saveFile;
			saveFile = NULL;

			// Then delete
			inFileSpec.Delete();
		}
	}
}

// Get text to save from open message window
const unichar_t* CMessageWindow::GetSaveText()
{
	// Reset the formatter
	mText->Reset(true);

	// Get appropriate content typt to interpret, checking for raw view
	bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
	EContentSubType actual_content = use_part ? mCurrentPart->GetContent().GetContentSubtype() : eContentSubPlain;

	// Parse data via formatter and return
	if ((mParsing == eViewAsRaw) && mRawUTF16Text.get())
		return mText->ParseBody(mRawUTF16Text.get(), actual_content, mParsing, mQuoteDepth);
	else if (mCurrentPart && mUTF16Text.get())
		return mText->ParseBody(mUTF16Text.get(), actual_content, mParsing, mQuoteDepth);

	return NULL;
}

// Print message
void CMessageWindow::DoPrint()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	LPrintout* 		thePrintout = NULL;
	CHeadAndFoot*	header = NULL;
	CHeadAndFoot*	footer = NULL;

	// Prevent any drawing while printing
	StStopRedraw noDraw(mText);

	// Save selection and remove
	CTextDisplay::StPreserveSelection _selection(mText);
	mText->SetSelectionRange(0, 0);

	// See if header insertion required
	cdstring header_insert;
	bool do_header_insert = CPreferences::sPrefs->mPrintSummary.GetValue() &&
								(!mShowHeader && mHeaderState.mExpanded);

	// See if printer font change required
	bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
	bool change_font = use_part &&
						(mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain);
	i18n::ECharsetCode charset = use_part ? mCurrentPart->GetContent().GetCharset() : i18n::eUSASCII;

	try
	{

		// Create printout
		thePrintout = LPrintout::CreatePrintout(paneid_MessagePrintout);
		ThrowIfNil_(thePrintout);
		thePrintout->SetPrintSpec(*GetPrintSpec());

		// Get placeholder
		LPlaceHolder* headPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintHead);
		LPlaceHolder* textPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintArea);
		LPlaceHolder* footPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintFooter);

		// Make it size of page with a 1/2" margin all round (less header & footer size)
		Rect pageRect;
		GetPrintSpec()->GetPageRect(pageRect);
		Rect paperRect;
		GetPrintSpec()->GetPaperRect(paperRect);
		short iHRes;
		short iVRes;
#if PP_Target_Carbon
		PMResolution res;
		::PMGetResolution(GetPrintSpec()->GetPageFormat(), &res);
		iHRes = res.hRes;
		iVRes = res.vRes;
#else
		iHRes = (**(THPrint) GetPrintSpec()->GetPrintRecord()).prInfo.iHRes;
		iVRes = (**(THPrint) GetPrintSpec()->GetPrintRecord()).prInfo.iVRes;
#endif

		short page_hSize = UGraphicUtils::RectWidth(pageRect) - iHRes;
		short page_vSize = UGraphicUtils::RectHeight(pageRect) - iVRes;

		// Do this here to prevent superview of text object being affected by printing
		textPlace->InstallOccupant(mText, atNone);

		// If plain text force change in text traits
		if (change_font)
			mText->SetFont(CPreferences::sPrefs->mPrintTextTraits.GetValue().traits);

		// Check for summary headers
		if (do_header_insert)
		{
			// Get summary from envelope
			std::ostrstream hdr;
			mItsMsg->GetEnvelope()->GetSummary(hdr);
			hdr << std::ends;
			header_insert.steal(hdr.str());

			// Parse as header
			mText->InsertFormattedHeader(header_insert.c_str());
		}

		// Create header edit text and set text traits and set to full width of page
		header = new CHeadAndFoot(CPreferences::sPrefs->mHeaderBox.GetValue());
		UTextTraits::SetTETextTraits(&CPreferences::sPrefs->mCaptionTextTraits.GetValue().traits, header->GetMacTEH());
		header->ResizeFrameTo(page_hSize, page_vSize, false);

		cdstring headTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mHeaderCaption.GetValue(), mItsMsg, false);
		c2pstr(headTxt.c_str_mod());
		header->SetDescriptor(reinterpret_cast<unsigned char*>(headTxt.c_str_mod()));

		// Create footer edit text and set text traits and set to full width of page
		footer = new CHeadAndFoot(CPreferences::sPrefs->mFooterBox.GetValue());
		UTextTraits::SetTETextTraits(&CPreferences::sPrefs->mCaptionTextTraits.GetValue().traits, footer->GetMacTEH());
		footer->ResizeFrameTo(page_hSize, page_vSize, false);

		cdstring footTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mFooterCaption.GetValue(), mItsMsg, false);
		c2pstr(footTxt.c_str_mod());
		footer->SetDescriptor(reinterpret_cast<unsigned char*>(footTxt.c_str_mod()));

		// Now do resizing of place holders
		long caption_line_height = ::TEGetHeight(1, 1, header->GetMacTEH());
		long text_line_height = mText->GetLineHeight(0);

		// Resize header
		long header_height = headTxt.length() ? caption_line_height * (**header->GetMacTEH()).nLines : 0;
		if (header_height)
		{
			if (CPreferences::sPrefs->mHeaderBox.GetValue())
				header_height += cHeadAndFootInset;
			headPlace->ResizeFrameTo(page_hSize, header_height, false);
		}
		else
			headPlace->Hide();

		// Resize footer
		long footer_height = headTxt.length() ? caption_line_height * (**footer->GetMacTEH()).nLines : 0;
		if (footer_height)
		{
			if (CPreferences::sPrefs->mFooterBox.GetValue())
				footer_height += cHeadAndFootInset;
			footPlace->ResizeFrameTo(page_hSize, footer_height, false);
		}
		else
			footPlace->Hide();

		// Resize text
		long text_height = page_vSize - (header_height + footer_height + (header_height ? caption_line_height : 0) + (footer_height ? caption_line_height : 0));
		text_height -= (text_height % text_line_height);
		textPlace->ResizeFrameTo(page_hSize, text_height, false);

		// Set origin 1/2" in from corner
		short hOrigin = -paperRect.left + iHRes/2;
		short vOrigin = -paperRect.top + iVRes/2;
		headPlace->PlaceInSuperFrameAt(hOrigin, vOrigin, false);
		textPlace->PlaceInSuperFrameAt(hOrigin, vOrigin + header_height + (header_height ? caption_line_height : 0), false);
		footPlace->PlaceInSuperFrameAt(hOrigin, vOrigin + header_height + (header_height ? caption_line_height : 0) +
															text_height + (footer_height ? caption_line_height : 0), false);

		headPlace->InstallOccupant(header, atNone);
		//textPlace->InstallOccupant(mText, atNone);
		footPlace->InstallOccupant(footer, atNone);

		// Do print job
		thePrintout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Remove summary headers
	if (do_header_insert)
	{
		cdustring temp(header_insert);
		mText->SetSelectionRange(0, temp.length());
		mText->InsertUTF8(cdstring::null_str);
	}

	// If plain text force change in text traits
	if (change_font)
	{
		mText->FocusDraw();
		if (mFontScale != 0)
		{
			ResetText();
		}
		else
			mText->SetFont(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	}

	// Kill footer & printout/placeholder
	delete thePrintout;
	delete header;			// <- by some odd accident this also results in footer being deleted!!??
	//delete footer;

	// Force text refresh
	mText->FocusDraw();
	//mText->AdjustImageToText();
	mText->Refresh();
}

// Try to copy a message
bool CMessageWindow::TryCopyMessage(bool option_key)
{
	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (mView->GetToolbar()->GetCopyBtn() && mView->GetToolbar()->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			return CopyThisMessage(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}

	return false;
}

// Handle twist of attachments
void CMessageWindow::DoPartsTwist(void)
{
	const SInt16	cBodyTwistMove = 84;

	// Determine motion size and hide/show
	if (mAttachTwister->GetValue()==0)
	{
		// Hide the parts table pane in the splitter
		mSplitter->ShowView(false, true);

		mAttachNumberField->Show();
		mFlatHierarchyBtn->Hide();
		SwitchTarget(mText);
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitter->ShowView(true, true);

		mAttachNumberField->Hide();
		mFlatHierarchyBtn->Show();
		SwitchTarget(mAttachTable);
	}

	// Update caption
	UpdatePartsCaption();

	Refresh();
}

// Update attachments caption
void CMessageWindow::UpdatePartsCaption(void)
{
	if (mAttachTwister->GetValue()==0)
	{
		cdstring caption;
		long count = 0;
		if (mAttachTable->GetFlat())
		{
			TableIndexT rows;
			TableIndexT cols;
			mAttachTable->GetTableSize(rows, cols);
			count = rows;
		}
		else
			count = mItsMsg ? mItsMsg->GetBody()->CountParts() : 0;

		if ((count >= 1) || !mItsMsg || !mItsMsg->GetBody()->CanDisplay())
		{
			caption = count;
		}
		else
			caption.FromResource("Alerts::Letter::NoAttachments");

		mAttachNumberField->SetText(caption);
	}
}

// Update list of attachments, show/hide
void CMessageWindow::UpdatePartsList(void)
{
	mAttachTable->SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
	UpdatePartsCaption();
}

// Flatten parts table
void CMessageWindow::DoPartsFlat(bool hierarchic)
{
	mAttachTable->SetFlat(!hierarchic);
	UpdatePartsCaption();
}

// Expand to full size header
void CMessageWindow::OnHeaderExpand()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;
	LayoutHeader();

	// Reset text as message header summary display will also change
	ResetText();
}

// From twister clicked
void CMessageWindow::OnFromTwist()
{
	// Toggle state
	mHeaderState.mFromExpanded = (mFromTwister->GetValue() == 1);
	DoTwist(mFromMove, mFromEdit, mFromTwister);
}

// To twister clicked
void CMessageWindow::OnToTwist()
{
	// Toggle state
	mHeaderState.mToExpanded = (mToTwister->GetValue() == 1);
	DoTwist(mToMove, mToEdit, mToTwister);
}

// Cc twister clicked
void CMessageWindow::OnCcTwist()
{
	// Toggle state
	mHeaderState.mCcExpanded = (mCCTwister->GetValue() == 1);
	DoTwist(mCCMove, mCCEdit, mCCTwister);
}

// To twister clicked
void CMessageWindow::DoTwist(LView* mover, CAddressText* aEdit, LDisclosureTriangle* aTwister)
{
	if (aTwister->GetValue()==0)
	{
		SInt16 moveby = -cTwistMove;
		aEdit->ResizeFrameBy(10, 0, false);
		aEdit->SetHardWrap(true);
		((CBetterScrollerX*) aEdit->GetSuperView())->ShowVerticalScrollBar(false);
		mover->ResizeFrameBy(0, moveby, false);
	}
	else
	{
		SInt16 moveby = cTwistMove;
		aEdit->ResizeFrameBy(-10, 0, false);
		aEdit->SetHardWrap(false);
		((CBetterScrollerX*) aEdit->GetSuperView())->ShowVerticalScrollBar(true);
		mover->ResizeFrameBy(0, moveby, false);
	}

	// Do header layout after change
	LayoutHeader();
}

// Layout header component based on state
void CMessageWindow::LayoutHeader()
{
	// Start with some initial offset
	SInt16 top = 2;

	SPoint32 header_loc;
	mHeaderMove->GetFrameLocation(header_loc);

	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Check for From field
		if (mHeaderState.mFromVisible)
		{
			// Get current position
			SPoint32 pos;
			mFromMove->GetFrameLocation(pos);

			// Get current size
			SDimension16 size;
			mFromMove->GetFrameSize(size);

			// Move to current top and increment top by height
			mFromMove->MoveBy(0, top - (pos.v - header_loc.v), false);
			top += size.height;

			// Always show it
			mFromMove->Show();
		}
		else
			// Always hide it
			mFromMove->Hide();

		// Check for To field
		if (mHeaderState.mToVisible)
		{
			// Get current position
			SPoint32 pos;
			mToMove->GetFrameLocation(pos);

			// Get current size
			SDimension16 size;
			mToMove->GetFrameSize(size);

			// Move to current top and increment top by height
			mToMove->MoveBy(0, top - (pos.v - header_loc.v), false);
			top += size.height;

			// Always show it
			mToMove->Show();
		}
		else
			// Always hide it
			mToMove->Hide();

		// Check for Cc field
		if (mHeaderState.mCcVisible)
		{
			// Get current position
			SPoint32 pos;
			mCCMove->GetFrameLocation(pos);

			// Get current size
			SDimension16 size;
			mCCMove->GetFrameSize(size);

			// Move to current top and increment top by height
			mCCMove->MoveBy(0, top - (pos.v - header_loc.v), false);
			top += size.height;

			// Always show it
			mCCMove->Show();
		}
		else
			// Always hide it
			mCCMove->Hide();

		// Check for Subject field
		if (mHeaderState.mSubjectVisible)
		{
			// Get current position
			SPoint32 pos;
			mSubjectMove->GetFrameLocation(pos);

			// Get current size
			SDimension16 size;
			mSubjectMove->GetFrameSize(size);

			// Move to current top and increment top by height
			mSubjectMove->MoveBy(0, top - (pos.v - header_loc.v), false);
			top += size.height;

			// Always show it
			mSubjectMove->Show();
		}
		else
			// Always hide it
			mSubjectMove->Hide();
	}
	else
	{
		// Hide all addresses and subject
		mFromMove->Hide();
		mToMove->Hide();
		mCCMove->Hide();
		mSubjectMove->Hide();

		// Make sure we have something to show the message count
		top = 20;
	}

	// Get current position
	SPoint32 pos;
	mAttachMove->GetFrameLocation(pos);

	// Get current size
	SDimension16 size;
	mAttachMove->GetFrameSize(size);

	// Move to current top and increment top by height
	mAttachMove->MoveBy(0, top - (pos.v - header_loc.v), false);
	top += size.height;

	// Get current size of header area
	mHeaderMove->GetFrameSize(size);

	// Resize header area
	mHeaderMove->ResizeFrameBy(0, top - size.height, false);

	// Determine how much to resize view
	SInt16 moveby = top - size.height;

	// Move parts and resize/move text
	mSplitter->MoveBy(0, moveby, false);
	mSplitter->ResizeFrameBy(0, -moveby, false);

	Refresh();
}

// Show/hide parts area
void CMessageWindow::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	SDimension16 move_size;
	mSecurePane->GetFrameSize(move_size);
	SInt16 moveby = move_size.height - 2;

	if (show)
	{
		// Shrink/move text pane
		mTextPane->ResizeFrameBy(0, -moveby, false);
		mTextPane->MoveBy(0, moveby, false);
#ifdef USE_HTML
		mHTML->ResizeFrameBy(0, -moveby, false);
		mHTML->MoveBy(0, moveby, false);
#endif

		// Show parts after all other changes
		mSecurePane->Show();
	}
	else
	{
		// Hide parts before other changes
		mSecurePane->Hide();

		// Expand/move splitter
		mTextPane->ResizeFrameBy(0, moveby, false);
		mTextPane->MoveBy(0, -moveby, false);
#ifdef USE_HTML
		mHTML->ResizeFrameBy(0, moveby, false);
		mHTML->MoveBy(0, -moveby, false);
#endif
	}

	mShowSecure = show;

	Refresh();
}

// Scroll the text if possible
bool CMessageWindow::SpacebarScroll(bool shift_key)
{
	// Scroll down by height of Frame, but not past bottom of Image
	SPoint32		frameLoc;
	SPoint32		imageLoc;
	SDimension16	frameSize;
	SDimension32	imageSize;
	mText->GetFrameLocation(frameLoc);
	mText->GetImageLocation(imageLoc);
	mText->GetFrameSize(frameSize);
	mText->GetImageSize(imageSize);

	// Check for up vs down
	if (shift_key)
	{
		// Check the amount that can be scrolled up
		SInt32	upMax = (frameLoc.v - imageLoc.v);
		if (upMax > 0)
		{
			SPoint32		scrollUnit;
			mText->GetScrollUnit(scrollUnit);

			SInt32	up = (frameSize.height - 1) / scrollUnit.v;
			if (up <= 0) {
				up = 1;
			}
			up *= scrollUnit.v;
			if (up > upMax)
				up = upMax;
			mText->FocusDraw();
			mText->ScrollImageBy(0, -up, true);

			return true;
		}
		else
			return false;
	}
	else
	{
		// Check the amount that can be scrolled down
		SInt32	downMax = imageSize.height - frameSize.height - (frameLoc.v - imageLoc.v);
		if (downMax > 0)
		{
			SPoint32		scrollUnit;
			mText->GetScrollUnit(scrollUnit);

			SInt32	down = (frameSize.height - 1) / scrollUnit.v;
			if (down <= 0) {
				down = 1;
			}
			down *= scrollUnit.v;
			if (down > downMax)
				down = downMax;
			mText->FocusDraw();
			mText->ScrollImageBy(0, down, true);

			return true;
		}
		else
			return false;
	}
}

#pragma mark ____________________________Window State

// Reset text traits from prefs
void CMessageWindow::ResetTextTraits(const TextTraitsRecord& list_traits,
										const TextTraitsRecord& display_traits)
{
	mAttachTable->SetTextTraits(list_traits);
	mText->SetTextTraits(display_traits);
	ResetText();
}

// Reset state from prefs
void CMessageWindow::ResetState(void)
{
	CMessageWindowState& state = CPreferences::sPrefs->mMessageWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state.GetBestRect(state);
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// Force twist down if required
	mAttachTwister->SetValue(state.GetPartsTwisted());
	mSplitter->SetPixelSplitPos(state.GetSplitChange());
	mHeaderState.mExpanded = !state.GetCollapsed();
	mFlatHierarchyBtn->SetValue(!state.GetFlat());
	
	LayoutHeader();

	// Set zoom state
	if (state.GetState() == eWindowStateMax)
		DoSetZoom(true);
}

// Save current state in prefs
void CMessageWindow::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CMessageWindowState state(NULL, &mUserBounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mSplitter->GetPixelSplitPos(), (mAttachTwister->GetValue()==1), !mHeaderState.mExpanded, mAttachTable->GetFlat(), false);
	if (CPreferences::sPrefs->mMessageWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMessageWindowDefault.SetDirty();

} // CMessageWindow::SaveDefaultState

#pragma mark ____________________________Drag & Drop

// Check to see whether drop from another window
Boolean CMessageWindow::CheckIfViewIsAlsoSender(DragReference inDragRef)
{
	// Get drag attributes
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);

	// Compare with relevant flag
	return (theDragAttributes & kDragInsideSenderWindow);
}

// Check its a message
Boolean CMessageWindow::ItemIsAcceptable(
	DragReference	inDragRef,
	ItemReference	inItemRef)
{
	// Check for cDragMsgType
	FlavorFlags		theFlags;
	if (::GetFlavorFlags(inDragRef, inItemRef, cDragMsgType, &theFlags) == noErr)
		return true;

	return false;
}

// Receive a message
void CMessageWindow::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&inItemBounds)
{
	// Do nothing if moved to same mbox
	if (CheckIfViewIsAlsoSender(inDragRef))
		return;

	InvalPortRect(&inItemBounds);

	//
	// Information about the drag contents we'll be needing.
	//
	FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
	Size			theDataSize;	// How much data there is for us.
	CMessage*		theFlavorData;	// Where we will put that data.

	// Check to make sure the drag contains a cDragMsgType item.
	if (::GetFlavorFlags(inDragRef, inItemRef, cDragMsgType, &theFlags) == noErr) {

		// Must save/restore target
		LCommander* old_target = GetTarget();

		// Get size of data
		::GetFlavorDataSize(inDragRef, inItemRef, cDragMsgType, &theDataSize);
		if (theDataSize) {
			ThrowIf_(theDataSize != sizeof(CMessage*));	// sanity check

			// Get the data about the message we are receiving.
			::GetFlavorData(inDragRef, inItemRef, cDragMsgType, &theFlavorData, &theDataSize, 0L);

			CDragMessageToWindow* task = dynamic_cast<CDragMessageToWindow*>(CDragTask::GetCurrentDragTask());
			if (!task)
				task = new CDragMessageToWindow(this, theFlavorData);

			// Must save/restore target
			SwitchTarget(old_target);
		}
	}
}

// Hilite/unhilite region two pixels wide round border
void CMessageWindow::HiliteDropArea(
	DragReference	inDragRef)
{
	Rect	dropRect;

	// Get frame less scroll bars
	mPane->CalcLocalFrameRect(dropRect);
	::InsetRect(&dropRect, 1, 1);
	dropRect.right -= 15;
	dropRect.bottom -= 15;

	// Create drag region
	RgnHandle	dropRgn = ::NewRgn();
	::RectRgn(dropRgn, &dropRect);

	// Do hilite
	::ShowDragHilite(inDragRef, dropRgn, true);

	// Dispose drag region
	::DisposeRgn(dropRgn);
}
