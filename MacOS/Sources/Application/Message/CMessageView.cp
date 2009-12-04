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


// Source for CMessageView class

#include "CMessageView.h"

#include "CActionManager.h"
#include "CAddress.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CBodyTable.h"
#include "CCaptionParser.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CFormattedTextDisplay.h"
#include "CHeadAndFoot.h"
#include "CLetterWindow.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CMulberryApp.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxWindow.h"
#include "CNetworkException.h"
#include "COptionClick.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CRFC822.h"
#include "CSpeechSynthesis.h"
#include "CSpaceBarAttachment.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CWaitCursor.h"
#include "CWebKitControl.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include "MyCFString.h"

#include <LDisclosureTriangle.h>
#include <LBevelButton.h>
#include <LIconControl.h>
#include <LPopupButton.h>

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

const int cHeaderHeight = 32;
const int cPartsCollapsedHeight = 24;
const int cPartsTableMinimumHeight = 84;
const int cPartsMoveAdjust = (cHeaderHeight - cPartsCollapsedHeight) / 2;
const int cSplitterMin1 = cPartsTableMinimumHeight;
const int cSplitterMin2 = 32;

// Static members


cdmutexprotect<CMessageView::CMessageViewList> CMessageView::sMsgViews;
cdstring CMessageView::sLastCopyTo;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageView::CMessageView()
{
	InitMessageView();
}

// Constructor from stream
CMessageView::CMessageView(LStream *inStream)
		: CBaseView(inStream)
{
	InitMessageView();
}

// Default destructor
CMessageView::~CMessageView()
{
	// Set status
	SetClosing();

	// Remove from list
	{
		cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
		CMessageViewList::iterator found = std::find(sMsgViews->begin(), sMsgViews->end(), this);
		if (found != sMsgViews->end())
			sMsgViews->erase(found);
	}

	// Set status
	SetClosed();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Default constructor
void CMessageView::InitMessageView()
{
	mItsMsg = NULL;
	mMailboxView = NULL;
	mWasUnseen = false;
	mItsMsgError = false;
	mShowText = NULL;
	mCurrentPart = NULL;
	mPrintSpec = NULL;
	mShowHeader = CPreferences::sPrefs->showMessageHeader.GetValue();
	mShowAddressCaption = true;
	mShowParts = true;
	mShowSecure = true;
	mSecureMulti = true;
	mParsing = eViewFormatted;
	mFontScale = 0;
	mQuoteDepth = -1;

	mRedisplayBlock = false;
	mResetTextBlock = false;
	mAllowDeleted = false;
	mDidExpandParts = false;

	mSeenTriggerTime = 0;

	// Add to list
	{
		cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
		sMsgViews->push_back(this);
	}
}

// Get details of sub-panes
void CMessageView::FinishCreateSelf()
{
	// Do inherited
	CBaseView::FinishCreateSelf();

	// Get window that owns this one
	LView* super = GetSuperView();
	while(super && !dynamic_cast<LWindow*>(super))
		super = super->GetSuperView();
	mOwnerWindow = dynamic_cast<LWindow*>(super);

	// Treat as 3-pane if in 3-pane window or is a preview in mailbox window
	CMailboxWindow* mbox_preview = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
	mIs3Pane = mbox_preview || (mOwnerWindow == C3PaneWindow::s3PaneWindow);

	mSplitter = (CSplitterView*) FindPaneByID(paneid_MessageViewSplitter);

	mHeader = (LView*) FindPaneByID(paneid_MessageViewTop);

	mCaption1 = (CTextDisplay*) FindPaneByID(paneid_MessageViewCaption1);
	mCaption1->SetDefaultBackground(UGAColorRamp::GetColor(colorRamp_Gray1));
	mCaption1->SetWrap(1000);
	mCaption1->SetHardWrap(true);

	mCaption2 = (CTextDisplay*) FindPaneByID(paneid_MessageViewCaption2);
	mCaption2->SetDefaultBackground(UGAColorRamp::GetColor(colorRamp_Gray1));
	mCaption2->SetWrap(1000);
	mCaption2->SetHardWrap(true);

	mZoomBtn = (LBevelButton*) FindPaneByID(paneid_MessageViewZoom);

	mParts = (LView*) FindPaneByID(paneid_MessageViewParts);

	mSeparator = (LPane*) FindPaneByID(paneid_MessageViewSeparator);

	mAttachHide = (LView*) FindPaneByID(paneid_MessageAttachHide);

	mAttachNumberField = (CStaticText*) FindPaneByID(paneid_MessageAttachNumber);
	mFlatHierarchyBtn = (LBevelButton*) FindPaneByID(paneid_MessageFlatHierarchy);
	mFlatHierarchyBtn->Hide();

	mAttachTwister = (LDisclosureTriangle*) FindPaneByID(paneid_MessageAttachTwister);
	mAttachmentsIcon = (LIconControl*) FindPaneByID(paneid_MessageAttachmentsIcon);
	mAlternativeIcon = (LIconControl*) FindPaneByID(paneid_MessageAlternativeIcon);

	mAttachTable = (CBodyTable*) FindPaneByID(paneid_MessageAttachTable);
	CContextMenuAttachment::AddUniqueContext(mAttachTable, 2004, mAttachTable);
	mAttachHide->Hide();
	mAttachHide->Disable();

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

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CMessageBtns);

	// Bottom spliiter items
	mBottomPane = (LView*) FindPaneByID(paneid_MessageBottom);
	mSecurePane = (LView*) FindPaneByID(paneid_MessageSecure);
	mSecureInfo = (CTextDisplay*) FindPaneByID(paneid_MessageSecureInfo);
	mSecureInfo->SetDefaultBackground(UGAColorRamp::GetColor(colorRamp_Gray1));

	mTextPane = (LView*) FindPaneByID(paneid_MessageScroller);

	// Make text edit the target
	mText = (CFormattedTextDisplay*) FindPaneByID(paneid_MessageText);
	SetLatentSub(mText);
	mText->SetTextTraits(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetFindAllowed(true);
	mText->Add_Listener(this);

	mHTML = (CWebKitControl*) FindPaneByID(paneid_MessageHTML);
#ifdef USE_HTML
	mHTML->Hide();
	mHTML->Disable();
#else
	delete mHTML;
#endif

	// Allow extended keyboard actions
	mText->AddAttachment(new CSpaceBarAttachment(this, mText));
	CContextMenuAttachment::AddUniqueContext(mText, 2002, mText, false);

	ShowSecretPane(false);

	// Install splitter views
	LView* msg_focus = (LView*) FindPaneByID(paneid_MessageViewTextFocus);
	mSplitter->InstallViews(mAttachHide, msg_focus ? msg_focus : mBottomPane, true);
	mSplitter->ShowView(false, true);
	mSplitter->SetMinima(cSplitterMin1, cSplitterMin2);
	mSplitter->SetPixelSplitPos(cSplitterMin1);
	mSplitter->SetLockResize(true);

	// Reset to default size
	ResetState();

	// Reset based on current view options
	ResetOptions();

	// Set status
	Disable();
	SetOpen();

	CContextMenuAttachment::AddUniqueContext(this, new CCopyToContextMenuAttachment(2001, this));
}

// Handle key presses
Boolean CMessageView::HandleKeyPress(const EventRecord	&inKeyEvent)
{
	// Get state of command modifiers (not shift)
	bool mods = ((inKeyEvent.modifiers & (optionKey | cmdKey | controlKey)) != 0);

	CommandT cmd = 0;
	switch (inKeyEvent.message & charCodeMask)
	{
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

	case 'z':
	case 'Z':
		cmd = cmd_ToolbarZoomPreview;
		break;

	default:;
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

	// Do default key press processing
	return CBaseView::HandleKeyPress(inKeyEvent);
}

//	Respond to commands
Boolean CMessageView::ObeyCommand(CommandT inCommand, void *ioParam)
{
	ResIDT	menuID;
	SInt16	menuItem;

	bool	cmdHandled = true;

	if (IsSyntheticCommand(inCommand, menuID, menuItem)) {

		// Show then select the requested window
		if (CCopyToMenu::IsCopyToMenu(menuID))
		{
			CMbox* mbox = NULL;
			if (CCopyToMenu::GetMbox(menuID, menuItem, mbox))
				CopyThisMessage(mbox, COptionMenu::sOptionKey);
			cmdHandled = true;
		}
		else
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
	}
	else {
		switch (inCommand)
		{
		case cmd_NewLetter:
			DoNewLetter(COptionMenu::sOptionKey);
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
			if (GetCopyBtn() && GetCopyBtn()->GetSelectedMbox(mbox, true))
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
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
		}
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMessageView::FindCommandStatus(
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
			outEnabled = true; //!mItsMsg->IsSubMessage();
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

		// These are not enabled for sub-messages
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
				outEnabled = true;
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
				LStr255 txt1(STRx_Standards, (mItsMsg && mItsMsg->IsDeleted()) ? str_Undelete : str_Delete);
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

		case cmd_ShowParts:
			{
				// Always enabled but text may change
				outEnabled = true;
				LStr255 txt2(STRx_Standards, !mAttachTwister->GetValue() ? str_ShowParts : str_HideParts);
				::PLstrcpy(outName, txt2);
			}
			break;

		case cmd_SpeakSelection:
			if (!CSpeechSynthesis::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName))
			{
				outEnabled = true;
				::PLstrcpy(outName, LStr255(STRx_Speech, str_SpeakMessage));
			}
			break;

		default:
			LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		}
	}
}

void CMessageView::FindFlagCommandStatus(
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
void CMessageView::ListenToMessage(MessageT inMessage,void *ioParam)
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

	case msg_MTwistAttach:
		DoPartsTwist();
		break;

	case msg_MFlatHierarchy:
		DoPartsFlat(*(long*) ioParam);
		break;

	case msg_View:
		OnTextFormatPopup(*(long*) ioParam);
		break;

	case msg_QuoteDepth:
		OnQuoteDepth((EQuoteDepth) *((SInt32*) ioParam));
		break;
	}
}

void CMessageView::ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh)
{
	// Do default
	LView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	// Captions occupy half the width each, adjusted for zoom button
	SDimension16 psize;
	GetFrameSize(psize);
	SInt16 cwidth = (psize.width - (mZoomBtn ? 18 : 0) - 10) / 2;
	
	SDimension16 csize1;
	mCaption1->GetFrameSize(csize1);
	
	if (csize1.width != cwidth)
	{
		mCaption1->ResizeFrameBy(cwidth - csize1.width, 0, true);
		mCaption2->MoveBy(cwidth - csize1.width, 0, false);
		mCaption2->ResizeFrameBy(cwidth - csize1.width, 0, true);
	}
}

#pragma mark ____________________________Timer

// Called during idle
void CMessageView::SpendTime(const EventRecord &inMacEvent)
{
	// See if we are equal or greater than trigger
	if (::time(NULL) >= mSeenTriggerTime)
	{
		// Change the seen flag and stop the timer
		if (mItsMsg && mItsMsg->IsUnseen())
		{
			mItsMsg->ChangeFlags(NMessage::eSeen, true);

			// Only do if message still exists
			if (!mItsMsg)
				return;

			// If seen state has changed do actions associated with that
			if (mWasUnseen)
				CActionManager::MessageSeenChange(mItsMsg);
		}

		StopSeenTimer();
	}
}

// Start timer to trigger seen flag
void CMessageView::StartSeenTimer(unsigned long secs)
{
	// Cache the time at which we trigger
	mSeenTriggerTime = ::time(NULL) + secs;

	// Start idle time processing
	StartRepeating();
}

// Stop timer to trigger seen flag
void CMessageView::StopSeenTimer()
{
	// Just stop idle time processing
	StopRepeating();
}

#pragma mark ____________________________Other Bits

void CMessageView::MakeToolbars(CToolbarView* parent)
{
	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(mText);
	}
	else
	{
		CMailboxWindow* mbox_window = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
		CMailboxView* mbox_view = mbox_window->GetMailboxView();

		Add_Listener(mbox_view->GetToolbar());
		mbox_view->GetToolbar()->AddCommander(mText);
	}
}

CMailboxToolbarPopup* CMessageView::GetCopyBtn() const
{
	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
	{
		// Copy btn from toolbar
		return C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->GetCopyBtn();
	}
	else
	{
		// Copy btn from toolbar
		CMailboxWindow* mbox_window = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
		CMailboxView* mbox_view = mbox_window->GetMailboxView();
		return static_cast<CMailboxInfoToolbar*>(mbox_view->GetToolbar())->GetCopyBtn();
	}
}

void CMessageView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTextDisplay::eBroadcast_Activate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewActivate, this);
		break;
	case CTextDisplay::eBroadcast_Deactivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewDeactivate, this);
		break;
	case CTextDisplay::eBroadcast_SelectionChanged:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewSelectionChanged, this);
		break;
	}
}

bool CMessageView::HasFocus() const
{
	// Switch target to the text (will focus)
	return LCommander::GetTarget() == mText;
}

void CMessageView::Focus()
{
	if (IsVisible())
		// Switch target to the text (will focus)
		LCommander::SwitchTarget(mText);
	else
		// Make text the target when this is activated
		GetOwningWindow()->SetLatentSub(mText);
}

// Set caption texts
void CMessageView::SetMessage(CMessage* theMsg, bool restore)
{
	// Only if different and no error
	if ((mItsMsg == theMsg) && !mItsMsgError)
		return;

	// If its not cached, we cannot open it
	if (theMsg && !theMsg->IsFullyCached())
		theMsg = NULL;

	// Always stop any seen timer
	StopSeenTimer();

	// Reset message and current part
	mItsMsg = theMsg;
	mWasUnseen = mItsMsg ? mItsMsg->IsUnseen() : false;
	mCurrentPart = NULL;

	// Reset the error flag
	mItsMsgError = false;

	// Reset any previous cached raw body
	mRawUTF16Text.reset(NULL);

	// Fill out address/subject/date fields
	ResetCaption();

	// Allow deleted messages that appear as message is read in
	mAllowDeleted = true;

	// Check for previous verify/decrypt status first (to avoid doing it again)
	// If there was a bad passphrase error, skip the cached data so user has a chance to enter
	// the correct passphrase next time
	if (mItsMsg && (mItsMsg->GetCryptoInfo() != NULL) && !mItsMsg->GetCryptoInfo()->GetBadPassphrase())
	{
		// Show the secure info pane
		SetSecretPane(*mItsMsg->GetCryptoInfo());
		ShowSecretPane(true);
	}

	// Now check for auto verify/decrypt - but not if restoring
	else if (mItsMsg && !restore &&
		(CPreferences::sPrefs->mAutoVerify.GetValue() && mItsMsg->GetBody()->IsVerifiable() ||
		 CPreferences::sPrefs->mAutoDecrypt.GetValue() && mItsMsg->GetBody()->IsDecryptable()))
	{
		// NULL out current part as its used in VerifyDecrypt
		mCurrentPart = NULL;
		OnMessageVerifyDecrypt();
	}

	// No verify/decrypt
	else
		// Turn of secure info pane
		ShowSecretPane(false);

	// Reset attachment list
	UpdatePartsList();

	// Reset text to first text part and reset table display
	if (mItsMsg)
	{
		CAttachment* attach = mItsMsg->FirstDisplayPart();
		ShowPart(attach, restore);
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
	}
	else
	{
		StStopRedraw noDraw(mText);

		// Wipe out the text
		mText->WipeText(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
		mText->Refresh();
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
	PostSetMessage(restore);

	// Force command updating since this can happen at idle time via a task
	// and commands for the mailbox (previewing this) need updating
	SetUpdateCommandStatus(true);
}

// Restore message text and state
void CMessageView::RestoreMessage(CMessage* theMsg, const SMessageViewState& state)
{
	// Don't allow drawing until complete
	StNoRedraw _noredraw(this);

	// Block changes to text whilst we change the state to avoid multple
	// calls to ResetText which is expensive if styled text parsing required
	{
		StValueChanger<bool> _block(mResetTextBlock, true);

		if (state.mShowHeader ^ mShowHeader)
			mHeaderBtn->SetValue(state.mShowHeader);
		if (state.mPartsExpanded ^ mAttachTwister->GetValue())
			mAttachTwister->SetValue(!mAttachTwister->GetValue());
		mDidExpandParts = state.mDidExpandParts;
		if (state.mParsing ^ mParsing)
			OnTextFormatPopup(state.mParsing);
		if (state.mFontScale != mFontScale)
		{
			mFontScale = state.mFontScale;
			ResetFontScale();
		}
	}

	// Now reset the message itself (force the current to NULL to force a proper reset)
	mItsMsg = NULL;
	SetMessage(theMsg, true);

	// Restore scroll and selection state
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
}

// Processing after message set and displayed
void CMessageView::PostSetMessage(bool restore)
{
	// Only do this if visible and not restoring
	if (!restore && IsVisible())
	{
		// Do parts expand if no visible part or multiparts and the parts area is visible
		if (mShowParts)
		{
			if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
			{
				// Do auto expansion or expand if no parts
				if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
					(mAttachTwister->GetValue() != 1))
				{
					mAttachTwister->SetValue(1);
					mDidExpandParts = true;

					// Must switch focus back to text as expanding parts sets it to the parts table
					// Only do this when in 1-pane mode
					if (!Is3Pane())
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

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CMessageView::ClearMessage()
{
	// Remove the reference to it here
	mItsMsg = NULL;
	mItsMsgError = true;

	// Remove the reference to it in the attachment table
	mAttachTable->ClearBody();
}

// Someone else changed this message
void CMessageView::MessageChanged()
{
	// Do not process redisplay if blocked
	if (mRedisplayBlock)
		return;

	// Disable parts area controls if no suitable message
	if (mItsMsg && !mItsMsg->IsFake())
	{
		Enable();
	}
	else
	{
		Disable();

		// If currently active, force focus to owner view
		if (IsOnDuty())
			mMailboxView->Focus();
		else
		{
			// Find the top/off-duty commander in the mailbox view chain and set the table latent
			LCommander* top = mMailboxView;
			while((top->GetOnDutyState() != triState_Off) && top->GetSuperCommander())
				top = top->GetSuperCommander();
			top->SetLatentSub(mMailboxView->GetTable());
		}
	}

	// Clear if message deleted
	if (!mAllowDeleted && mItsMsg && mItsMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
		SetMessage(NULL);
}

// Show sub-message window
void CMessageView::ShowSubMessage(CAttachment* attach, Rect zoom_from)
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
void CMessageView::SetSecretPane(const CMessageCryptoInfo& info)
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
			mSecureInfo->SetFontStyle(bold);
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
		const SInt16 moveby = 15;

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
void CMessageView::SetPrintSpec(LPrintSpec* printSpec)
{
	// Don't delete it if we're setting it to NULL
	if (mPrintSpec && printSpec)
		delete mPrintSpec;
	mPrintSpec = printSpec;
}

// Reset message text
void CMessageView::ResetText()
{
	// Only do if message exists and not blocked
	if (!mItsMsg || mResetTextBlock) return;

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

		//Reset the parser's data
		mText->Reset(true, mFontScale);

		// Copy in header if required
		if (mShowHeader)
			mText->ParseHeader(mItsMsg->GetHeader(), actual_view);
		else if (GetViewOptions().GetShowSummary())
		{
			std::ostrstream hdr;
			mItsMsg->GetEnvelope()->GetSummary(hdr);
			hdr << std::ends;

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

	// Make it active in 1-pane mode (and not printing)
	if (!Is3Pane())
		SwitchTarget(mText);
}

// Reset font scale text
void CMessageView::ResetFontScale()
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

// Reset message text
void CMessageView::ResetCaption()
{
	mCaption1->FocusDraw();
	mCaption2->FocusDraw();

	{
		StStopRedraw noDraw1(mCaption1);
		StStopRedraw noDraw2(mCaption2);

		// Determine actual view type and content
		TextTraitsRecord txtr;
		UTextTraits::LoadTextTraits(261, txtr);

		// Delete any previous text
		mCaption1->WipeText(txtr);
		mCaption2->WipeText(txtr);

		CEnvelope* env = mItsMsg && mItsMsg->GetEnvelope() ? mItsMsg->GetEnvelope() : NULL;

		// From
		if (env && env->GetFrom()->size())
		{
			mCaption1->SetFontStyle(bold);
			mCaption1->InsertUTF8(cHDR_FROM, 6);
			mCaption1->SetFontStyle(normal);
			cdstring addr = env->GetFrom()->front()->GetFullAddress();
			mCaption1->InsertUTF8(addr);
		}

		// To
		if (env && env->GetTo()->size())
		{
			mCaption2->SetFontStyle(bold);
			mCaption2->InsertUTF8(cHDR_TO, 4);
			mCaption2->SetFontStyle(normal);
			cdstring addr = env->GetTo()->front()->GetFullAddress();
			if (env->GetTo()->size() > 1)
				addr += ", …";
			mCaption2->InsertUTF8(addr);
		}

		// CC
		if (env && env->GetCC()->size())
		{
			cdstring space = "    ";
			mCaption2->InsertUTF8(space);

			mCaption2->SetFontStyle(bold);
			mCaption2->InsertUTF8(cHDR_CC, 4);
			mCaption2->SetFontStyle(normal);
			cdstring addr = env->GetCC()->front()->GetFullAddress();
			if (env->GetCC()->size() > 1)
				addr += ", …";
			mCaption2->InsertUTF8(addr);
		}

		// Insert a 14pt space to crudely adjust line spacing
		mCaption1->SetFontSize(::Long2Fix(13));
		mCaption1->InsertUTF8(" ");
		mCaption1->SetFontSize(::Long2Fix(10));
		mCaption2->SetFontSize(::Long2Fix(13));
		mCaption2->InsertUTF8(" ");
		mCaption2->SetFontSize(::Long2Fix(10));

		// Next line
		mCaption1->InsertUTF8(os_endl, os_endl_len);
		mCaption2->InsertUTF8(os_endl, os_endl_len);

		// Subject
		if (env)
		{
			mCaption1->SetFontStyle(bold);
			mCaption1->InsertUTF8(cHDR_SUBJECT, 9);
			mCaption1->SetFontStyle(normal);
			cdstring subj = env->GetSubject();
			mCaption1->InsertUTF8(subj);
		}

		// Date
		if (env)
		{
			mCaption2->SetFontStyle(bold);
			mCaption2->InsertUTF8(cHDR_DATE, 6);
			mCaption2->SetFontStyle(normal);
			cdstring date = env->GetTextDate(true, false);
			mCaption2->InsertUTF8(date);
		}

		// Insert a 14pt space to crudely adjust line spacing
		mCaption1->SetFontSize(::Long2Fix(13));
		mCaption1->InsertUTF8(" ");
		mCaption1->SetFontSize(::Long2Fix(10));
		mCaption2->SetFontSize(::Long2Fix(13));
		mCaption2->InsertUTF8(" ");
		mCaption2->SetFontSize(::Long2Fix(10));

		mCaption1->SetSelection(0, 0);
		mCaption2->SetSelection(0, 0);
	}

	// Force redraw
	mCaption1->Refresh();
	mCaption2->Refresh();
}

// Mail view options changed
void CMessageView::ResetOptions()
{
	// Reset any message text to ensure summary info is shown/hidden
	ResetText();

	// Show hide the address caption
	if (mShowAddressCaption ^ GetViewOptions().GetShowAddressPane())
	{
		mShowAddressCaption = GetViewOptions().GetShowAddressPane();
		ShowCaption(mShowAddressCaption);
	}

	// Show/hide the parts
	if (mShowParts ^ GetViewOptions().GetShowParts())
	{
		mShowParts = GetViewOptions().GetShowParts();
		ShowParts(mShowParts);
	}

	// Check zoom & close visible state
	if (mShowAddressCaption || mShowParts)
	{
		if (mZoomBtn)
			mZoomBtn->Show();
	}
	else
	{
		if (mZoomBtn)
			mZoomBtn->Hide();
	}
}

#pragma mark ____________________________Commands

// Save message to file
void CMessageView::SaveThisMessage()
{
	CMessage*			saveMsg = mItsMsg;

	// Only do if message exists
	if (!saveMsg)
		return;

	// Get current subject as name - filter colons
	cdstring saveAsName(saveMsg->GetEnvelope()->GetSubject());
	::strreplace(saveAsName.c_str_mod(), ":", '_');

	// Do standard save as dialog
	MyCFString cfstr(saveAsName, kCFStringEncodingUTF8);
	PPx::FSObject fspec;
	bool replacing;
	if (PP_StandardDialogs::AskSaveFile(cfstr, kMessageFileType, fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))

		// Now save file
		DoSave(fspec, kMessageFileType);

}

// Save message
void CMessageView::DoSave(PPx::FSObject &inFileSpec,
								OSType inFileType)
{
	// Only do if message still exists
	if (!mItsMsg)
		return;

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

		// Write header if required (always write it in raw mode
		if (CPreferences::sPrefs->saveMessageHeader.GetValue() || (mParsing == eViewAsRaw))
			saveFile->WriteBlock(mItsMsg->GetHeader(), ::strlen(mItsMsg->GetHeader()));

		// Write text to file stream
		const unichar_t* txt = GetSaveText();
		if (txt != NULL)
			saveFile->WriteBlock(txt, ::unistrlen(txt) * sizeof(unichar_t));

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
const unichar_t* CMessageView::GetSaveText()
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
void CMessageView::DoPrint()
{
	// Only do if message still exists
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
								(!mShowHeader && !GetViewOptions().GetShowSummary());

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
		PMResolution res;
		::PMGetResolution(GetPrintSpec()->GetPageFormat(), &res);
		iHRes = res.hRes;
		iVRes = res.vRes;

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
bool CMessageView::TryCopyMessage(bool option_key)
{
	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (GetCopyBtn() && GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			return CopyThisMessage(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}

	return false;
}

// Handle twist of attachments
void CMessageView::DoPartsTwist()
{
	bool expanding = (mAttachTwister->GetValue() == 1);

	SDimension16 parts_size;
	mAttachHide->GetFrameSize(parts_size);
	SInt16 moveby = parts_size.height;

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts before collapsing
		SBooleanRect binding;
		mAttachHide->GetFrameBinding(binding);
		binding.bottom = false;
		mAttachHide->SetFrameBinding(binding);

		// Collapsing splitter
		mSplitter->ShowView(false, true);

		// Show/hide items
		mAttachHide->Hide();
		mAttachHide->Disable();
		mFlatHierarchyBtn->Hide();
		mAttachNumberField->Show();

		// Only do this when in 1-pane mode or when parts is the current target
		if (!Is3Pane() || mAttachTable->IsTarget())
			SwitchTarget(mText);
	}
	else
	{
		// Expanding splitter
		mSplitter->ShowView(true, true);

		// Show/hide items
		mAttachHide->Enable();
		mAttachHide->Show();
		mAttachNumberField->Hide();
		mFlatHierarchyBtn->Show();

		// Turn on resize of parts before expanding
		SBooleanRect binding;
		mAttachHide->GetFrameBinding(binding);
		binding.bottom = true;
		mAttachHide->SetFrameBinding(binding);

		// Only do this when in 1-pane mode
		if (!Is3Pane())
			SwitchTarget(mAttachTable);
	}

	// Update caption
	UpdatePartsCaption();

	Refresh();
}

// Update attachments caption
void CMessageView::UpdatePartsCaption()
{
	if (mAttachTwister->GetValue() == 0)
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
			count = (mItsMsg ? mItsMsg->GetBody()->CountParts() : 0);

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
void CMessageView::UpdatePartsList()
{
	mAttachTable->SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
	UpdatePartsCaption();
}

// Flatten parts table
void CMessageView::DoPartsFlat(bool hierarchic)
{
	mAttachTable->SetFlat(!hierarchic);
	UpdatePartsCaption();
}

void CMessageView::OnTextFormatPopup(long nID)
{
	EView old_view = mParsing;
	EView new_view = (EView) nID;
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

// Scroll the text if possible
bool CMessageView::SpacebarScroll(bool shift_key)
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

// Get current state of the view
void CMessageView::GetViewState(SMessageViewState& state) const
{
	mText->GetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->GetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	state.mShowHeader = mShowHeader;
	state.mPartsExpanded = mAttachTwister->GetValue();
	state.mDidExpandParts = mDidExpandParts;
	state.mParsing = mParsing;
	state.mFontScale = mFontScale;
}

// Change view state to one saved earlier
void CMessageView::SetViewState(const SMessageViewState& state)
{
	bool changed = false;
	if (state.mShowHeader ^ mShowHeader)
	{
		mHeaderBtn->SetValue(!mShowHeader);
		changed = true;
	}
	if (state.mPartsExpanded ^ mAttachTwister->GetValue())
	{
		mAttachTwister->SetValue(!mAttachTwister->GetValue());
		changed = true;
	}
	mDidExpandParts = state.mDidExpandParts;
	if (state.mParsing ^ mParsing)
	{
		OnTextFormatPopup(state.mParsing);
		changed = true;
	}
	if (state.mFontScale != mFontScale)
	{
		mFontScale = state.mFontScale;
		ResetFontScale();
		changed = true;
	}

	// Nowe reset text if changed
	if (changed)
		ResetText();

	// Do these after all other changes
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
}

// Show/hide address caption area
void CMessageView::ShowCaption(bool show)
{
	SDimension16 move_size;
	mCaption1->GetFrameSize(move_size);
	SInt16 moveby = move_size.height;

	if (show)
	{
		// Shrink/move splitter
		mSplitter->ResizeFrameBy(0, -moveby, false);
		mSplitter->MoveBy(0, moveby, false);

		// Increase header height
		mHeader->ResizeFrameBy(0, moveby, false);

		// Show caption after all other changes
		mCaption1->Show();
		mCaption2->Show();
		mSeparator->Show();
	}
	else
	{
		// Hide caption before other changes
		mCaption1->Hide();
		mCaption2->Hide();
		mSeparator->Hide();

		// Decrease header height
		mHeader->ResizeFrameBy(0, -moveby, false);

		// Expand/move splitter
		mSplitter->ResizeFrameBy(0, moveby, false);
		mSplitter->MoveBy(0, -moveby, false);
	}

	Refresh();
}

// Show/hide parts area
void CMessageView::ShowParts(bool show)
{
	// Collapse parts if needed
	if (!show && (mAttachTwister->GetValue() == 1))
		mAttachTwister->SetValue(0);

	SDimension16 move_size;
	mParts->GetFrameSize(move_size);
	SInt16 moveby = move_size.height;

	if (show)
	{
		// Shrink/move splitter
		mSplitter->ResizeFrameBy(0, -moveby, false);
		mSplitter->MoveBy(0, moveby, false);

		// Increase header height
		mHeader->ResizeFrameBy(0, moveby, false);

		// Show parts after all other changes
		mParts->Show();
	}
	else
	{
		// Hide parts before other changes
		mParts->Hide();

		// Decrease header height
		mHeader->ResizeFrameBy(0, -moveby, false);

		// Expand/move splitter
		mSplitter->ResizeFrameBy(0, moveby, false);
		mSplitter->MoveBy(0, -moveby, false);
	}
}

// Show/hide parts area
void CMessageView::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	SDimension16 move_size;
	mSecurePane->GetFrameSize(move_size);
	SInt16 moveby = move_size.height;

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

// Reset text traits from prefs
void CMessageView::ResetTextTraits(const TextTraitsRecord& list_traits,
										const TextTraitsRecord& display_traits)
{
	mAttachTable->SetTextTraits(list_traits);
	mText->SetTextTraits(display_traits);
	ResetText();
}

// Reset state from prefs
void CMessageView::ResetState(bool force)
{
	CMessageWindowState& state = (mOwnerWindow == C3PaneWindow::s3PaneWindow) ?
										CPreferences::sPrefs->mMessageView3Pane.Value() :
										CPreferences::sPrefs->mMessageView1Pane.Value();

	// Force twist down if required
	mAttachTwister->SetValue(state.GetPartsTwisted());
	mSplitter->SetPixelSplitPos(state.GetSplitChange());
	mFlatHierarchyBtn->SetValue(!state.GetFlat());
}

// Save current state in prefs
void CMessageView::SaveState()
{
	SaveDefaultState();
}

// Save current state in prefs
void CMessageView::SaveDefaultState()
{
	// Get bounds
	Rect bounds = {0, 0, 0, 0};

	// Add info to prefs
	CMessageWindowState state(NULL, &bounds, eWindowStateNormal,
								mSplitter->GetPixelSplitPos(), (mAttachTwister->GetValue()==1), false, mAttachTable->GetFlat(), false);

	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
	{
		if (CPreferences::sPrefs->mMessageView3Pane.Value().Merge(state))
			CPreferences::sPrefs->mMessageView3Pane.SetDirty();
	}
	else
	{
		if (CPreferences::sPrefs->mMessageView1Pane.Value().Merge(state))
			CPreferences::sPrefs->mMessageView1Pane.SetDirty();
	}
}
