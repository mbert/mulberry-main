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


// Source for CLetterWindow class

#include "CLetterWindow.h"

#include "CAddress.h"
#include "CAddressBookManager.h"
#include "CAddressDisplay.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CBetterScrollerX.h"
#include "CCaptionParser.h"
#include "CCharsetManager.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCopyToMenu.h"
#include "CDataAttachment.h"
#include "CEditFormattedTextDisplay.h"
#include "CStyleToolbar.h"
#include "CEnrichedTransformer.h"
#include "CEnvelope.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFileTable.h"
#include "CFormatElement.h"
#include "CFormatList.h"
#include "CHTMLTransformer.h"
#include "CIdentityPopup.h"
#include "CIMAPCommon.h"
#include "CLetterDoc.h"
#include "CLetterView.h"
#include "CMailboxPopup.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMIMESupport.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionClick.h"
#include "CParserEnriched.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CSimpleTitleTable.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStringResources.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextEngine.h"
#include "CTextFieldX.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"

#include "MyCFString.h"

#include <LCheckBox.h>
#include <LBevelButton.h>
#include <LDisclosureTriangle.h>
#include <LIconControl.h>
#include <LPictureControl.h>

#include <LCFString.h>

//#include <UProfiler.h>

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C L E T T E R W I N D O W
// __________________________________________________________________________________________________

// Static members

cdmutexprotect<CLetterWindow::CLetterWindowList> CLetterWindow::sLetterWindows;
short CLetterWindow::sLetterWindowCount = 0;
cdstring  CLetterWindow::sLastAppendTo;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CLetterWindow::CLetterWindow()
#if PP_Target_Carbon
		 : LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
		 : LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitLetterWindow();
}

// Constructor from stream
CLetterWindow::CLetterWindow(LStream *inStream)
		: LWindow(inStream),
#if PP_Target_Carbon
		  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
		  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitLetterWindow();
}

// Default destructor
CLetterWindow::~CLetterWindow()
{
	// Set status
	SetClosing();

	// Delete all parts
	delete mBody;
	mBody = NULL;
	if (mPartsTable != NULL)
		mPartsTable->SetBody(NULL);

	// Delete any stored bounce header
	delete mBounceHeader;
	mBounceHeader = NULL;

	// Delete any message lists
	delete mMsgs;
	mMsgs = NULL;

	// Remove from list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		CLetterWindowList::iterator found = std::find(sLetterWindows->begin(), sLetterWindows->end(), this);
		if (found != sLetterWindows->end())
			sLetterWindows->erase(found);
		if (sLetterWindows->size() <= 1)
			sLetterWindowCount = sLetterWindows->size();
	}
	CWindowsMenu::RemoveWindow(this);

	// Disable each of the address fields to prevent resolution
	if (mToEdit != NULL)
		mToEdit->Deactivate();
	if (mCCEdit != NULL)
		mCCEdit->Deactivate();
	if (mBCCEdit != NULL)
		mBCCEdit->Deactivate();

	// Set status
	SetClosed();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

CLetterWindow* CLetterWindow::ManualCreate()
{
	CLetterDoc* newDoc = NULL;
	CLetterWindow* newWindow = NULL;

	try
	{
		// Create the mailer window and give it the message
		newDoc = new CLetterDoc(CMulberryApp::sApp, NULL);
		newWindow = static_cast<CLetterWindow*>(newDoc->GetWindow());
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete newDoc;
		newDoc = NULL;
		newWindow = NULL;
	}
	
	return newWindow;
}

// Common init
void CLetterWindow::InitLetterWindow(void)
{
	// NULL out others
	mMsgs = NULL;
	mIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	mCustomIdentity = false;
	mDoSign = false;
	mDoEncrypt = false;
	mSignatureLength = 0;
	mCopyOriginalAllowed = false;
	mCopyOriginal = false;
	mCurrentPart = NULL;
	mBody = NULL;

	mPartsTable = NULL;
	mToEdit = NULL;
	mCCEdit = NULL;
	mBCCEdit = NULL;

	mHeaderState.mExpanded = true;
	mHeaderState.mToVisible = true;
	mHeaderState.mToExpanded = false;
	mHeaderState.mCcVisible = false;
	mHeaderState.mCcExpanded = false;
	mHeaderState.mBccVisible = false;
	mHeaderState.mBccExpanded = false;
	mHeaderState.mSubjectVisible = true;

	mSentOnce = false;
	mDirty = false;
	mReply = false;
	mForward = false;
	mBounce = false;
	mReject = false;
	mSendAgain = false;
	mOriginalEncrypted = false;
	mMarkSaved = false;
	mBounceHeader = NULL;

	// Add to list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		sLetterWindows->push_back(this);
		sLetterWindowCount++;
	}
	CWindowsMenu::AddWindow(this);
}

// Get details of sub-panes
void CLetterWindow::FinishCreateSelf(void)
{
	// Do inherited
	LWindow::FinishCreateSelf();
	AddAttachment(new LUndoer);

	// Gray header
	mHeaderMove = (LView*) FindPaneByID(paneid_LetterHeader);

	// Get toolbar and main view
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_LetterToolbarView);
	mView = (CLetterView*) FindPaneByID(paneid_LetterView);

	// Create toolbars for a view we own
	mToolbarView->SetSibling(mView);
	mView->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set backgrounds
	mFromMove = (LView*) FindPaneByID(paneid_LetterFromMove);
	mFromView = (LView*) FindPaneByID(paneid_LetterFromView);
	mIdentities = (CIdentityPopup*) FindPaneByID(paneid_LetterIdentities);
	mIdentityEditBtn = (LBevelButton*) FindPaneByID(paneid_LetterIdentityEdit);
	mIdentityEditBtn->Hide();
	mRecipientView = (LView*) FindPaneByID(paneid_LetterRecipientView);
	mRecipientView->Hide();
	mRecipientText = (CStaticText*) FindPaneByID(paneid_LetterRecipientText);
	mIdentities->Reset(CPreferences::sPrefs->mIdentities.GetValue(), true);	// Can have custom items
	mIdentities->SetValue(mIdentities->FirstIndex());

	mToMove = (LView*) FindPaneByID(paneid_LetterToMove);

	mToTwister = (LDisclosureTriangle*) FindPaneByID(paneid_LetterToTwister);

	mToEdit = (CAddressDisplay*) FindPaneByID(paneid_LetterToX);
	//mToEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mToEdit->GetSuperView())->ShowVerticalScrollBar(false);
	mToEdit->SetTwister(mToTwister);
	CContextMenuAttachment::AddUniqueContext(mToEdit, 3022, mToEdit, false);

	mCCMove = (LView*) FindPaneByID(paneid_LetterCCMove);

	mCCTwister = (LDisclosureTriangle*) FindPaneByID(paneid_LetterCCTwister);

	mCCEdit = (CAddressDisplay*) FindPaneByID(paneid_LetterCCX);
	//mCCEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mCCEdit->GetSuperView())->ShowVerticalScrollBar(false);
	mCCEdit->SetTwister(mCCTwister);
	CContextMenuAttachment::AddUniqueContext(mCCEdit, 3022, mCCEdit, false);

	mBCCMove = (LView*) FindPaneByID(paneid_LetterBCCMove);

	mBCCTwister = (LDisclosureTriangle*) FindPaneByID(paneid_LetterBCCTwister);

	mBCCEdit = (CAddressDisplay*) FindPaneByID(paneid_LetterBCCX);
	//mBCCEdit->SetOutlineHighlight(false);
	((CBetterScrollerX*) mBCCEdit->GetSuperView())->ShowVerticalScrollBar(false);
	mBCCEdit->SetTwister(mBCCTwister);
	CContextMenuAttachment::AddUniqueContext(mBCCEdit, 3022, mBCCEdit, false);

	mSubjectMove = (LView*) FindPaneByID(paneid_LetterSubjectMove);

	mSubjectField = (CTextFieldX*) FindPaneByID(paneid_LetterSubject);
	mSubjectField->AddListener(this);

	mSentIcon = (LIconControl*) FindPaneByID(paneid_LetterSentIcon);
	mSentIcon->Hide();

	mPartsMove = (LView*) FindPaneByID(paneid_LetterPartsMove);

	FindPaneByID(paneid_LetterPartsCaptionA)->SetVisible(false);
	mPartsNumber = (CTextFieldX*) FindPaneByID(paneid_LetterPartsNumber);

	mPartsTwister = (LDisclosureTriangle*) FindPaneByID(paneid_LetterPartsTwister);
	mPartsIcon = (LIconControl*) FindPaneByID(paneid_LetterPartsIcon);

	// Get menu
	mAppendList = (CMailboxPopup*) FindPaneByID(paneid_LetterAppendList);
	mAppendList->SetCopyTo(false);
	mAppendList->SetPickPopup(false);
	mAppendList->SetDefault();

	// Get menus
	mPartsArea = (LView*) FindPaneByID(paneid_LetterPartsHide);
	mPartsTable = (CFileTable*) FindPaneByID(paneid_LetterPartsTable);
	CContextMenuAttachment::AddUniqueContext(mPartsTable, 3019, mPartsTable);

	mPartsTitles = (CSimpleTitleTable*) FindPaneByID(paneid_LetterPartsTitles);
	mPartsTitles->SyncTable(mPartsTable);
	mPartsTable->SetTitles(mPartsTitles);
	mPartsTitles->SetOwner(mPartsTable);

	mPartsTitles->LoadTitles("UI::Titles::LetterParts", 6);
	mPartsTable->SetAttachmentsOnly(CPreferences::sPrefs->mDisplayAttachments.GetValue());

	// Create dummy text part
	mBody = (mCurrentPart = new CDataAttachment((char*) NULL));
	mCurrentPart->SetContent(eContentText, CPreferences::sPrefs->compose_as.GetValue());
	mPartsTable->SetBody(mBody);
	mPartsTable->SetRowShow(mCurrentPart);
	UpdatePartsCaption();

	// Split pane
	mSplitter = (CSplitterView*) FindPaneByID(paneid_LetterSplit);

	// Get text area
	mTextArea = (LView*) FindPaneByID(paneid_LetterTextArea);

	// Get text pane
	mText = (CEditFormattedTextDisplay*) FindPaneByID(paneid_LetterText);
	bool html = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML) || (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched);
	mText->SetTextTraits(html ? CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits : CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	CContextMenuAttachment::AddUniqueContext(mText, 3017, mText, false);

	// Make wrap safe
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();
	if (wrap == 0)
		wrap = 1000;

	// Force text to fixed width
#ifdef _TODO
	SDimension32 text_size;
	mText->GetImageSize(text_size);
	mText->ResizeImageTo(wrap * 6 + 1, text_size.height, false);
#endif

	// Add header & footer text to text field
	char ret2[]="\r\r";
	bool	changed = false;

	// Focus before fiddling with WE
	mText->FocusDraw();

	// Set spaces per tab in text entry and do not allow select all when tabbing
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetWrap(CPreferences::sPrefs->wrap_length.GetValue());
	mText->SetHardWrap(!CPreferences::sPrefs->mWindowWrap.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetFindAllowed(true);

	// Give default reply quotation to text display
	mText->SetQuotation(CPreferences::sPrefs->mReplyQuote.GetValue());

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CLetterBtns);

	// Set Drag & Drop pane to scroller
	mPane = FindPaneByID(paneid_LetterScroller);

	mToolbar = (CStyleToolbar *)FindPaneByID(paneid_LetterEnriched);
	mToolbar->Hide();

	mText->SetToolbar(mToolbar);
	HideToolbar(true);

	SetToolbar(CPreferences::sPrefs->compose_as.GetValue());

	// Install the splitter items and initialise the splitter
	mSplitter->InstallViews(mPartsArea, mTextArea, true);
	mSplitter->SetMinima(84, 64);
	mSplitter->ShowView(false, true);
	mSplitter->SetLockResize(true);

	ResetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits,
						CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);

	// Reset to default size
	ResetState();

	// Do CC/Bcc show if required
	if (CPreferences::sPrefs->mShowCCs.GetValue())
		OnAddCc();
	if (CPreferences::sPrefs->mShowBCCs.GetValue())
		OnAddBcc();

	// Set status
	SetOpen();

	CContextMenuAttachment::AddUniqueContext(this, 3016, this);
}

//Respond to commands
Boolean CLetterWindow::ObeyCommand(CommandT inCommand,void *ioParam)
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
				CopyNow(mbox, COptionMenu::sOptionKey);
			cmdHandled = true;
		}
		else
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
	}
	else
	{
		switch (inCommand)
		{
		case cmd_PasteQuote:
			IncludeScrap();
			break;

		case cmd_SpellCheck:
		case cmd_ToolbarLetterSpellBtn:
			DoSpellCheck(false);
			break;

		case cmd_IncludeOriginal:
			IncludeFromReply(false, false);
			break;

		case cmd_AddCC:
		case cmd_ToolbarLetterAddCCBtn:
			OnAddCc();
			break;

		case cmd_AddBcc:
		case cmd_ToolbarLetterAddBccBtn:
			OnAddBcc();
			break;

		case cmd_CopyOriginal:
		case cmd_ToolbarLetterCopyOriginalBtn:
			OnDraftCopyOriginal();
			break;

		case cmd_AttachFile:
		case cmd_ToolbarLetterAttachBtn:
			// Only if not locked out
			if (!CAdminLock::sAdminLock.mNoAttachments)
			{
				mPartsTable->DoAttachFile();
				UpdatePartsCaption();
			}
			break;

		//Justin
		case cmd_NewPlainTextPart:
			mPartsTable->DoNewTextPart(eContentSubPlain);
			break;

		case cmd_NewEnrichedTextPart:
			mPartsTable->DoNewTextPart(eContentSubEnriched);
			break;

		case cmd_NewHTMLTextPart:
			mPartsTable->DoNewTextPart(eContentSubHTML);
			break;
		case cmd_MultipartMixed:
			mPartsTable->DoMultipartMixed();
			break;

		case cmd_MultipartParallel:
			mPartsTable->DoMultipartParallel();
			break;

		case cmd_MultipartDigest:
			mPartsTable->DoMultipartDigest();
			break;

		case cmd_MultipartAlternative:
			mPartsTable->DoMultipartAlternative();
			break;

		case cmd_CurrentTextPlain:
		case cmd_CurrentTextPlainWrapped:
		case cmd_CurrentTextEnriched:
		case cmd_CurrentTextHTML:
			DoStyleChangePopup(inCommand - cmd_CurrentTextPlain + 1);
			break;

		case cmd_Send:
		case cmd_ToolbarLetterSendBtn:
			OnDraftSendMail();
			break;

		case cmd_AppendNow:
			CopyNow(COptionMenu::sOptionKey);
			break;

		case cmd_ToolbarLetterAppendNowBtn:
			CopyNow(false);
			break;

		case cmd_ToolbarLetterAppendNowOptionBtn:
			CopyNow(true);
			break;

		case cmd_InsertSignature:
		case cmd_ToolbarLetterSignatureBtn:
			InsertSignature(GetIdentity()->GetSignature(true));
			break;

		case cmd_SignDraft:
		case cmd_ToolbarLetterSignPopup:
			OnDraftSign();
			break;

		case cmd_EncryptDraft:
		case cmd_ToolbarLetterEncryptPopup:
			OnDraftEncrypt();
			break;

		case cmd_MDNRead:
		case cmd_ToolbarLetterReceiptBtn:
			OnDraftMDN();
			break;

		case cmd_DSNSuccess:
		case cmd_ToolbarLetterDSNBtn:
			OnDraftDSNSuccess();
			break;

		case cmd_DSNFailure:
			OnDraftDSNFailure();
			break;

		case cmd_DSNDelay:
			OnDraftDSNDelay();
			break;

		case cmd_DSNFull:
			OnDraftDSNFull();
			break;

		case cmd_SetDefaultSize:
			SaveDefaultState();
			break;

		case cmd_ResetDefaultWindow:
			ResetState();
			break;

		case cmd_ExpandWindow:
		case cmd_ToolbarCollapseHeader:
			// Fake hit in button
			OnHeaderExpand();
			break;

		case cmd_ShowParts:
			// Fake hit in button
			mPartsTwister->SetValue(!mPartsTwister->GetValue());
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
void CLetterWindow::FindCommandStatus(
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
		// Always enable windows menu
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
		// Check if TEXT is in the Scrap and typing into letter area and not reject
		case cmd_PasteQuote:
			outEnabled = (UScrap::HasData(kScrapFlavorTypeText) || UScrap::HasData(kScrapFlavorTypeUnicode)) && mText->IsOnDuty() && !mReject;
			break;

		case cmd_SpellCheck:
		case cmd_ToolbarLetterSpellBtn:
			// Must have spell checker and valid part and not reject
			outEnabled = mCurrentPart && CPluginManager::sPluginManager.HasSpelling() && !mReject;
			break;

		// Must have a reply to be executable
		case cmd_IncludeOriginal:
			outEnabled = (mMsgs && mMsgs->size() && mReply);
			break;

		case cmd_Send:
		case cmd_ToolbarLetterSendBtn:
			{
				const CIdentity* id = GetIdentity();
				outEnabled = CConnectionManager::sConnectionManager.IsConnected() ||
								CSMTPAccountManager::sSMTPAccountManager &&
								CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected(*id);
			}
			break;

		case cmd_AddCC:
		case cmd_ToolbarLetterAddCCBtn:
			outEnabled = !mHeaderState.mCcVisible;
			break;

		case cmd_AddBcc:
		case cmd_ToolbarLetterAddBccBtn:
			outEnabled = !mHeaderState.mBccVisible;
			break;

		case cmd_CopyOriginal:
		case cmd_ToolbarLetterCopyOriginalBtn:
			outEnabled = mCopyOriginalAllowed;
			outUsesMark = outEnabled;
			outMark = mCopyOriginal ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_AttachFile:
		case cmd_ToolbarLetterAttachBtn:
			// Only if not locked out and not reject
			outEnabled = !CAdminLock::sAdminLock.mNoAttachments && !mReject;
			break;

		case cmd_DraftNotifications:
		case cmd_NewTextPart:
		case cmd_NewPlainTextPart:
		case cmd_MultipartChoice:
		case cmd_MultipartMixed:
		case cmd_MultipartParallel:
		case cmd_MultipartDigest:
		case cmd_MultipartAlternative:
			// Only if not rejecting
			outEnabled = !mReject;
			break;

		case cmd_NewEnrichedTextPart:
		case cmd_NewHTMLTextPart:
		case cmd_CurrentTextPart:
			// Only if allowed by admin and not rejecting
			outEnabled = CAdminLock::sAdminLock.mAllowStyledComposition && !mReject;
			break;

		case cmd_CurrentTextPlain:
		case cmd_CurrentTextPlainWrapped:
		case cmd_CurrentTextEnriched:
		case cmd_CurrentTextHTML:
			// Only if allowed by admin and not rejecting
			outEnabled = CAdminLock::sAdminLock.mAllowStyledComposition && !mReject &&
							(mCurrentPart != NULL) && (mCurrentPart->GetContent().GetContentType() == eContentText);
			outUsesMark = false;
			if (outEnabled)
			{
				outUsesMark = true;
				bool marked = false;
				switch(inCommand)
				{
				case cmd_CurrentTextPlain:
					marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && !mText->GetHardWrap();
					break;
				case cmd_CurrentTextPlainWrapped:
					marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && mText->GetHardWrap();
					break;
				case cmd_CurrentTextEnriched:
					marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched);
					break;
				case cmd_CurrentTextHTML:
					marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML);
					break;
				}
				outMark = marked ? (UInt16)checkMark : (UInt16)noMark;
			}
			break;

		case cmd_SetDefaultSize:
		case cmd_ResetDefaultWindow:
			// Should check for valid recipient
			outEnabled = true;
			break;

		// Must have Copy To set
		case cmd_AppendToMailbox:
			outEnabled = !mReject;
			break;

		case cmd_AppendNow:
		case cmd_ToolbarLetterAppendNowBtn:
		case cmd_ToolbarLetterAppendNowOptionBtn:
		{
			CMbox* found = NULL;
			mAppendList->GetSelectedMbox(found, false);
			outEnabled = (found != (CMbox*) -1) && !mReject;
			break;
		}

		case cmd_InsertSignature:
		case cmd_ToolbarLetterSignatureBtn:
			outEnabled = !mReject;
			break;

		case cmd_SignDraft:
		case cmd_ToolbarLetterSignPopup:
			outEnabled = CPluginManager::sPluginManager.HasSecurity() && !mReject;
			outUsesMark = !mReject;
			outMark = mDoSign ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_EncryptDraft:
		case cmd_ToolbarLetterEncryptPopup:
			outEnabled = CPluginManager::sPluginManager.HasSecurity() && !mReject;
			outUsesMark = !mReject;
			outMark = mDoEncrypt ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_MDNRead:
		case cmd_ToolbarLetterReceiptBtn:
			outEnabled = !mReject;
			outUsesMark = true;
			outMark = mDSN.GetMDN() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_DSNSuccess:
		case cmd_ToolbarLetterDSNBtn:
			outEnabled = !mReject;
			outUsesMark = true;
			outMark = mDSN.GetSuccess() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_DSNFailure:
			outEnabled = !mReject;
			outUsesMark = true;
			outMark = mDSN.GetFailure() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_DSNDelay:
			outEnabled = !mReject;
			outUsesMark = true;
			outMark = mDSN.GetDelay() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_DSNFull:
			outEnabled = !mReject;
			outUsesMark = true;
			outMark = mDSN.GetFull() ? (UInt16)checkMark : (UInt16)noMark;
			break;

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
				LStr255 txt2(STRx_Standards, !mPartsTwister->GetValue() ? str_ShowParts : str_HideParts);
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

// Respond to clicks in the icon buttons
void CLetterWindow::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_IdentityEdit:
		OnEditCustom();
		break;

	case paneid_LetterSubject:
		ChangedSubject();
		break;

	case msg_AppendTo:
	{
		mAppendList->GetSelectedMboxName(sLastAppendTo, false);

		// Disable buttons
		CMbox* found = NULL;
		mAppendList->GetSelectedMbox(found, false);
		if (found == (CMbox*) -1)
		{
			mCopyOriginalAllowed = false;
		}
		else
		{
			// Stop appending replied to same mailbox
			if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce))
			{
				if ((mMsgs->front()->GetMbox() == found) || mMsgs->front()->IsSubMessage())
				{
					mCopyOriginalAllowed = false;
				}
				else
				{
					mCopyOriginalAllowed = true;
				}
			}
			else
			{
				mCopyOriginalAllowed = false;
			}
		}

		break;
	}

	case msg_TwistTo:
		OnToTwist();
		break;

	case msg_TwistCC:
		OnCcTwist();
		break;

	case msg_TwistBCC:
		OnBccTwist();
		break;

	case msg_TwistAttach:
		DoPartsTwist();
		break;

	case msg_Identities:
		DoIdentitiesPopup(*(long*) ioParam);
		break;
	}
}

// Make sure toolbar is activated too
void CLetterWindow::Activate()
{
	LWindow::Activate();
	
	mView->RefreshToolbar();

	static_cast<CLetterDoc*>(GetDocument())->StartRepeating();
}

// Do safety save on deactivate
void CLetterWindow::Deactivate()
{
	// Do temporary save at this time
	static_cast<CLetterDoc*>(GetDocument())->SaveTemporary();
	static_cast<CLetterDoc*>(GetDocument())->StopRepeating();

	LWindow::Deactivate();
}

// Return dirty state
void CLetterWindow::SetDirty(bool dirty)
{
	mToEdit->SetDirty(dirty);
	mCCEdit->SetDirty(dirty);
	mBCCEdit->SetDirty(dirty);
	mSubjectField->SetDirty(dirty);
	mText->SetDirty(dirty);
	mPartsTable->SetDirty(dirty);

	mDirty = dirty;
}

// Return dirty state
bool	CLetterWindow::IsDirty(void)
{
	mDirty = mToEdit->IsDirty() ||
				mCCEdit->IsDirty() ||
				mBCCEdit->IsDirty() ||
				mSubjectField->IsDirty() || mPartsTable->IsDirty() || mText->IsDirty();

	return mDirty;
}

#pragma mark ____________________________Initialise

// Add address list to caption/field:
void CLetterWindow::SetTwistList(CAddressDisplay* aEdit,
									LDisclosureTriangle* aTwister,
									const CAddressList* list)
{

	cdstring addrs;

	// Add all in list
	bool got_two = false;
	for(CAddressList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		if (!addrs.empty())
		{
			addrs += os_endl;
			got_two = true;
		}
		addrs += (*iter)->GetFullAddress();
	}

	// Update field - will null if no addresses
	aEdit->SetText(addrs);

	// Force twist down if more than one address
	aTwister->SetValue(got_two);

}

// Set address lists
void CLetterWindow::SetAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc)
{

	// Add all tos
	if (to)
		SetTwistList(mToEdit, mToTwister, to);

	// Add all Ccs
	if (cc)
	{
		SetTwistList(mCCEdit, mCCTwister, cc);
		
		// Make sure Cc is visible
		if (!mHeaderState.mCcVisible && cc->size())
			OnAddCc();
	}	

	// Add all Bccs
	if (bcc)
	{
		SetTwistList(mBCCEdit, mBCCTwister, bcc);
		
		// Make sure Bcc is visible
		if (!mHeaderState.mBccVisible && bcc->size())
			OnAddBcc();
	}	

	// Set target to text if some items inserted into list
	if (mHeaderState.mExpanded)
	{
		if (IsVisible())
			SwitchTarget((to || cc || bcc) ? static_cast<LCommander*>(mSubjectField) : static_cast<LCommander*>(mToEdit));
		else
			SetLatentSub((to || cc || bcc) ? static_cast<LCommander*>(mSubjectField) : static_cast<LCommander*>(mToEdit));
	}
}

// Get address lists
void CLetterWindow::GetAddressLists(CAddressList*& to,
									CAddressList*& cc,
									CAddressList*& bcc)
{
	// Get addresses from each field
	to = mToEdit->GetAddresses();
	cc = mCCEdit->GetAddresses();
	bcc = mBCCEdit->GetAddresses();
}

void CLetterWindow::SetSubject(const cdstring& theSubject)
{
	mSubjectField->SetText(theSubject);
	ChangedSubject();
}

void CLetterWindow::GetSubject(cdstring& theSubject) const
{
	theSubject = mSubjectField->GetText();
}

// Do common bits after setting a message
void CLetterWindow::PostSetMessage(const CIdentity* id)
{
	// Enable append replied if not sub-message (not for rejects)
	CMbox* found = NULL;
	if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce) &&
		mAppendList->GetSelectedMbox(found, false) &&
		(mMsgs->front()->GetMbox() != found) &&
			!mMsgs->front()->IsSubMessage())
	{
		mCopyOriginalAllowed = true;
	}

	// Must make sure window is listening to protocol logoffs
	if (mMsgs)
	{
		for(CMessageList::const_iterator iter = mMsgs->begin(); iter != mMsgs->end(); iter++)
			(*iter)->GetMbox()->GetMsgProtocol()->Add_Listener(this);
	}

	// Init the identity
	InitIdentity(id);

	// Need to reset focus
	if (mReply || mReject || mSendAgain || !mHeaderState.mExpanded)
		// Set target to text
		SwitchTarget(mText);
	else
		// Set target to to address
		SwitchTarget(mToEdit);

	// If message comes from INBOX and pref set check it as true
	// Do this after init'ing the identity
	// Don't do it for rejects
	if (mMsgs && mMsgs->size() && !mReject &&
		!::strcmpnocase(mMsgs->front()->GetMbox()->GetName(), cINBOX) &&
		CPreferences::sPrefs->inbox_append.GetValue())
	{
		mCopyOriginal = true;
	}

	// Disable certain controls for rejects
	if (mReject)
	{
		// Disable all edit fields
		mToEdit->Disable();
		mCCEdit->Disable();
		mBCCEdit->Disable();
		mSubjectField->Disable();

		// Disable copy to controls
		mAppendList->Disable();
		mCopyOriginalAllowed = false;
	}

	// Finally force redraw
	Refresh();
	SetDirty(false);
	
	// Force main toolbar to update
	mView->RefreshToolbar();

	// Make sure its visible
	Show();
	
	// Do address capture if needed
	if ((mReply || mForward) && mMsgs && CPreferences::sPrefs->mCaptureRespond.GetValue())
		CAddressBookManager::sAddressBookManager->CaptureAddress(*mMsgs);
}

#pragma mark ____________________________Sending

#pragma mark ____________________________Message actions

// Display this text
void CLetterWindow::SetText(const char *theText, bool plain)
{
	// Set message text
	{
		StStopRedraw noDraw(mText);

		// Delete any previous text
		mText->SetSelectionRange(0, LONG_MAX);
		mText->InsertUTF8(cdstring::null_str);

		if(mBody->GetContent().GetContentType() == eContentText)
		{
			mText->SetText(NULL);
			mText->FocusDraw();
			{
				StStopRedraw noDraw(mText);
				bool html = ((mBody->GetContent().GetContentSubtype() == eContentSubEnriched) ||
							 (mBody->GetContent().GetContentSubtype() == eContentSubHTML)) &&
								CAdminLock::sAdminLock.mAllowStyledComposition;
				mText->SetTextTraits(html ? CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits : CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
				mText->Reset(true);
				{
					//StProfileSection profile("\pMulberry Profile", 200, 20);
					cdustring text_utf16(theText);
					mText->ParseBody(text_utf16, plain ? eContentSubPlain : mBody->GetContent().GetContentSubtype(), eViewFormatted, -1);
				}
				mText->InsertFormatted(html ? eViewFormatted : eViewPlain);
			}
			mText->ScrollImageTo(0, 0, false);
			mText->Refresh();

			SetToolbar(mBody->GetContent());

			// Make it active
			SwitchTarget(mText);
		}
		// Copy in text
		//{
			//StHandleLocker lock(theText);
		//	mText->WEInsert(theText, length);
		//}

		// Set selection at start
		mText->SetSelectionRange(0, 0);

		// Have removed signature
		mSignatureLength = 0;
	}
}

//  Get tagged text
void CLetterWindow::GetTaggedText(cdstring& txt)
{
	// Might be no visible part
	if (!mCurrentPart)
		return;

	// Determine nature of part
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubEnriched:
		{
			CEnrichedTransformer enrich(mText);
			txt.steal(enrich.Transform());
		}
		break;
	case eContentSubHTML:
		{
			CHTMLTransformer html(mText);
			txt.steal(html.Transform());
		}
		break;
	default:
		mText->GetText(txt);
		break;
	}
}

// Get envelope
void CLetterWindow::SetEnvelope(Handle theText, long length)
{
	LHandleStream data(theText);
	long copy_length;

	// Get to text
	data.ReadBlock(&copy_length, sizeof(long));
	if (copy_length)
	{
		std::auto_ptr<char> txt(new char[copy_length]);
		data.ReadBlock(txt.get(), copy_length);
		mToEdit->SetText(txt.get(), copy_length);
	}

	// Get CC text
	data.ReadBlock(&copy_length, sizeof(long));
	if (copy_length)
	{
		std::auto_ptr<char> txt(new char[copy_length]);
		data.ReadBlock(txt.get(), copy_length);
		mCCEdit->SetText(txt.get(), copy_length);
		if (!mHeaderState.mCcVisible)
			OnAddCc();
	}

	// Get BCC text
	data.ReadBlock(&copy_length, sizeof(long));
	if (copy_length)
	{
		std::auto_ptr<char> txt(new char[copy_length]);
		data.GetBytes(txt.get(), copy_length);
		mBCCEdit->SetText(txt.get(), copy_length);
		if (!mHeaderState.mBccVisible)
			OnAddBcc();
	}

	// Get subject text
	data.ReadBlock(&copy_length, sizeof(long));
	if (copy_length)
	{
		std::auto_ptr<char> txt(new char[copy_length + 1]);
		data.ReadBlock(txt.get(), copy_length);
		txt.get()[copy_length] = '\0';
		mSubjectField->SetText(txt.get());
		ChangedSubject();
	}

	if(!(data.AtEnd()))
	{
		// Get the subpart
		data.ReadBlock(&copy_length, sizeof(long));
		if (copy_length)
		{
			std::auto_ptr<char> txt(new char[copy_length + 1]);
			data.ReadBlock(txt.get(), copy_length);
			txt.get()[copy_length] = '\0';
			mBody->SetContentSubtype(txt.get());
			SetToolbar(mBody->GetContent());
		}
	}

	if(!(data.AtEnd()))
	{
		// Get the subpart
		data.ReadBlock(&copy_length, sizeof(long));
		cdstring txt;
		txt.reserve(copy_length + 1);
		if (copy_length)
			data.ReadBlock(txt.c_str_mod(), copy_length);
		mAppendList->SetSelectedMbox(txt, false, copy_length == 0);
		mAppendList->Refresh();
	}

	// Detach handle
	data.DetachDataHandle();
}

// Get envelope
void CLetterWindow::GetEnvelope(Handle& theText, long& length)
{
	LHandleStream data;

	// Get to text
	cdstring txt;
	mToEdit->GetText(txt);
	long copy_length = txt.length();
	data.WriteBlock(&copy_length, sizeof(long));
	{
		data.WriteBlock(txt.c_str(), copy_length);
	}

	// Get CC text
	mCCEdit->GetText(txt);
	copy_length = txt.length();
	data.WriteBlock(&copy_length, sizeof(long));
	{
		data.WriteBlock(txt.c_str(), copy_length);
	}

	// Get BCC text
	mBCCEdit->GetText(txt);
	copy_length = txt.length();
	data.WriteBlock(&copy_length, sizeof(long));
	{
		data.WriteBlock(txt.c_str(), copy_length);
	}

	// Get subject text
	mSubjectField->GetText(txt);
	copy_length = txt.length();
	data.WriteBlock(&copy_length, sizeof(long));
	data.WriteBlock(txt.c_str(), copy_length);

	// Get subpart
	cdstring subpartText;
	if (mCurrentPart)
		subpartText = mCurrentPart->GetContentSubtypeText();
	copy_length = subpartText.length();
	data.WriteBlock(&copy_length, sizeof(long));
	data.WriteBlock(subpartText.c_str(), copy_length);

	// Get copy to
	cdstring copy_to;
	mAppendList->GetSelectedMboxName(copy_to, false);
	copy_length = copy_to.length();
	data.WriteBlock(&copy_length, sizeof(long));
	data.WriteBlock(copy_to.c_str(), copy_length);

	// Detach handle
	length = data.GetLength();
	theText = data.DetachDataHandle();
}

// Set display traits
void CLetterWindow::SetTraits(SLetterTraits& traits)
{
	// Now set position via AE
	DoSetPosition(topLeft(traits.bounds));

}

// Get display traits
void CLetterWindow::GetTraits(SLetterTraits& traits)
{
	// Get window bounds
	Rect theBounds;
	CalcPortFrameRect(theBounds);
	PortToGlobalPoint(topLeft(theBounds));
	PortToGlobalPoint(botRight(theBounds));

	// Set traits
	traits.bounds = theBounds;
}

void CLetterWindow::ChangedSubject()
{
	// If document unspecified then change title to match subject
	if (!GetDocument()->IsSpecified())
	{
		// Cache original doc title in case subject is deleted
		if (mOriginalTitle.empty())
		{
			MyCFString origText(CopyCFDescriptor());
			mOriginalTitle = origText.GetString();
		}

		cdstring subjectText = mSubjectField->GetText();
		
		// Check for empty subject and use original title
		if (subjectText.empty())
		{
			subjectText = mOriginalTitle;
		}

		MyCFString cftext(subjectText.c_str(), kCFStringEncodingUTF8);
		SetCFDescriptor(cftext);
		CWindowsMenu::RenamedWindow();
	}
}

void CLetterWindow::ShowSentIcon()
{
	mSentIcon->Show();
	mSentIcon->Refresh();
}

bool CLetterWindow::DoAppendReplied() const
{
	return mCopyOriginalAllowed && mCopyOriginal;
}

bool CLetterWindow::IsSigned() const
{
	return mDoSign;
}

bool CLetterWindow::IsEncrypted() const
{
	return mDoEncrypt;
}


#pragma mark ____________________________Quoting

// Include message selection indented
void CLetterWindow::IncludeMessage(CMessage* theMsg, bool forward, bool header)
{
	// Only do if text available
	if (!theMsg->HasText())
		return;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return;

	// Focus before fiddling with TEHandle
	StFocusTE clip_hide(mText);

	// Read it in
	try
	{
		// Get first suitable quotable part
		CAttachment* attach = theMsg->FirstDisplayPart();

		if (attach)
		{
			// Get header if required
			const char* msg_hdr = header ? theMsg->GetHeader() : NULL;

			// Read text of specific message part
			const char* msg_txt = attach->ReadPart(theMsg);
			i18n::ECharsetCode charset = attach->GetContent().GetCharset();

			// Convert to utf8
			cdstring utf8;
			if (msg_txt != NULL)
			{
				if (charset == i18n::eUTF8)
				{
					utf8 = msg_txt;
				}
				else
				{
					std::ostrstream sout;
					i18n::CCharsetManager::sCharsetManager.ToUTF8(charset, msg_txt, ::strlen(msg_txt), sout);
					sout << std::ends;
					utf8.steal(sout.str());
				}
			}

			// Quote it if available
			if (header && msg_hdr || msg_txt)
				IncludeMessageTxt(theMsg, msg_hdr, utf8, forward, attach->GetContent().GetContentSubtype(), attach->GetContent().IsFlowed());
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

}

// Include message selection indented
void CLetterWindow::IncludeMessageTxt(CMessage* theMsg,
										const char* msg_hdr,
										const char* msg_txt,
										bool forward,
										EContentSubType type,
										bool is_flowed)
{
	// Get pos of insert cursor
	SInt32 selStart;
	SInt32 selEnd;
	mText->GetSelectionRange(selStart, selEnd);
	cdstring theTxt;

	// Force insert of CR if at very top of message of cursor will be placed at top
	if ((selStart == 0) &&
		((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
		 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue())))
	{
		cdstring convertTxt = os_endl2;
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
			theTxt += convertTxt;
			break;
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
			theTxt += converted.get();
			break;
		}
		default:;
		}
	}

	// Get start caption
	{
		cdstring convertTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardStart.GetValue() :
														CPreferences::sPrefs->mReplyStart.GetValue(), theMsg);;
		if (convertTxt.length())
		{
			switch(mCurrentPart->GetContent().GetContentSubtype())
			{
			case eContentSubPlain:
				theTxt += convertTxt;
				break;
			case eContentSubEnriched:
			{
				std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
				theTxt += converted.get();
				break;
			}
			case eContentSubHTML:
			{
				std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
				theTxt += converted.get();
				break;
			}
			default:;
			}
		}
	}

	// Start styled quotation
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		// Must add line end after caption - the other two sub-types will automatically add
		// a line end when the tag is encountered
		theTxt += os_endl;
		break;
	}
	case eContentSubEnriched:
	{
		theTxt += "<excerpt>";
		break;
	}
	case eContentSubHTML:
	{
		theTxt += "<blockquote>";
		break;
	}
	default:;
	}

	// Include the message's header as addition if required
	if (msg_hdr)
	{
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
		{
			std::auto_ptr<const char> quoted(QuoteText(msg_hdr, forward, true));
			theTxt += quoted.get();
			break;
		}
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(msg_hdr));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(msg_hdr));
			theTxt += converted.get();
			break;
		}
		default:;
		}
	}

	// Always flow Enriched and HTML
	switch(type)
	{
	case eContentSubPlain:
	default:
		break;
	case eContentSubEnriched:
	case eContentSubHTML:
		is_flowed = true;
		break;
	}

	// Include the message's text as addition
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		// Parse the (possibly styled) original text out into plain text format
		cdustring text_utf16(msg_txt);
		mText->ParseBody(text_utf16, type, eViewPlain, -1);
		
		// Get parsed text as utf8
		cdstring text_utf8 = mText->GetParsedText().ToUTF8();
		
		// Quote it and add to the text being added to the draft
		std::auto_ptr<const char> quoted(QuoteText(text_utf8, forward, false, is_flowed));
		theTxt += quoted.get();
		break;
	}
	case eContentSubEnriched:
	{
		switch(type)
		{
		case eContentSubPlain:
		{
			// Must wrap if flowed
			std::auto_ptr<const char> wrapped;
			const char* txt = msg_txt;
			if (is_flowed)
			{
				wrapped.reset(CTextEngine::WrapLines(msg_txt, ::strlen(msg_txt), CRFC822::GetWrapLength(), false));
				txt = wrapped.get();
			}
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubEnriched:
		{
			theTxt += msg_txt;
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ConvertToEnriched(msg_txt));
			theTxt += converted.get();
			break;
		}
		default:;
		}
		break;
	}
	case eContentSubHTML:
	{
		switch(type)
		{
		case eContentSubPlain:
		{
			// Must wrap if flowed
			std::auto_ptr<const char> wrapped;
			const char* txt = msg_txt;
			if (is_flowed)
			{
				wrapped.reset(CTextEngine::WrapLines(msg_txt, ::strlen(msg_txt), CRFC822::GetWrapLength(), false));
				txt = wrapped.get();
			}
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ConvertToHTML(msg_txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			theTxt += msg_txt;
			break;
		}
		default:;
		}
		break;
	}
	default:;
	}

	// Stop styled quotation before line end
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		break;
	}
	case eContentSubEnriched:
	{
		theTxt += "</excerpt>";
		break;
	}
	case eContentSubHTML:
	{
		theTxt += "</blockquote>";
		break;
	}
	default:;
	}

	// Finish with CRs
	{
		cdstring convertTxt;
		if (theTxt[theTxt.length() -1] != '\r')
			convertTxt += os_endl;
		convertTxt += os_endl;

		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
			theTxt += convertTxt;
			break;
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
			theTxt += converted.get();
			break;
		}
		default:;
		}
	}

	// Get end caption
	{
		cdstring convertTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardEnd.GetValue() :
														CPreferences::sPrefs->mReplyEnd.GetValue(), theMsg);;
		if (convertTxt.length())
		{
			convertTxt += os_endl;
			switch(mCurrentPart->GetContent().GetContentSubtype())
			{
			case eContentSubPlain:
				theTxt += convertTxt;
				break;
			case eContentSubEnriched:
			{
				std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
				theTxt += converted.get();
				break;
			}
			case eContentSubHTML:
			{
				std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
				theTxt += converted.get();
				break;
			}
			default:;
			}
		}
	}

	SetToolbar(CPreferences::sPrefs->compose_as.GetValue());

	{
		bool cursort_at_start = (forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
			 						(!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue());

		// Convert to utf16 and insert
		cdustring text_utf16(theTxt);
		mText->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);

		SyncPart();

		if (cursort_at_start)
			mText->SetSelectionRange(selStart, selStart);
		mText->SetDirty(true);
		
	}

	// Force caltext, adjust image & refresh
	mText->ScrollImageBy(0, 0, false);
	mText->Refresh();
	mText->Reset(true);
}

// Include some text from scrap
void CLetterWindow::IncludeScrap(void)
{
	StHandleBlock txt(0L);
	if (UScrap::GetData(kScrapFlavorTypeUnicode, txt))
	{
		StHandleLocker lock(txt);
		UniCharCount insert_length = ::GetHandleSize(txt) / sizeof(UniChar);

		cdustring utf16((UniChar*)*txt, insert_length);
		IncludeText(utf16.ToUTF8(), false, true);
	}
	else if (UScrap::GetData(kScrapFlavorTypeText, txt))
	{
		StHandleLocker lock(txt);
		std::auto_ptr<char> temp(new char[::GetHandleSize(txt) + 1]);
		::memmove(temp.get(), *txt, ::GetHandleSize(txt));
		temp.get()[::GetHandleSize(txt)] = 0;
		LCFString cfstr(temp.get());
		temp.reset();

		UniCharCount insert_length = ::CFStringGetLength(cfstr);
		std::auto_ptr<UniChar> tempu(new UniChar[insert_length + 1]);
		::CFStringGetCharacters(cfstr, CFRangeMake(0, (CFIndex) insert_length), tempu.get());
		tempu.get()[insert_length] = 0;

		cdustring utf16;
		utf16.steal(tempu.release());
		IncludeText(utf16.ToUTF8(), false, true);
	}
}

// Include the replied to mail
void CLetterWindow::IncludeFromReply(bool forward, bool header)
{
	// Beep if not replying
	if (!mMsgs)
	{
		::SysBeep(1);
	}
	else
	{
		// See whether cursor at top or bottom
		if ((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
			 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue()))
		{
			// Add all to message (cursor at top => in reverse)
			// Protect against mMsgs changing whilst doing ops
			size_t old_size = mMsgs->size();
			for(CMessageList::reverse_iterator riter = mMsgs->rbegin(); mMsgs && (mMsgs->size() == old_size) && (riter != mMsgs->rend()); riter++)
				IncludeMessage(*riter, forward, header);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			// Protect against mMsgs changing whilst doing ops
			size_t old_size = mMsgs->size();
			for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
				IncludeMessage(*iter, forward, header);
		}
	}
}

// Include some text indented
void CLetterWindow::IncludeText(const char* theText, bool forward, bool adding)
{
	cdstring theTxt;

	// Focus before fiddling with TEHandle
	StFocusTE clip_hide(mText);

	if (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain)
	{
		// Start with CR if not adding
		if (!adding)
			theTxt += CR;

		std::auto_ptr<const char> quoted(QuoteText(theText, forward));
		theTxt += quoted.get();

		// Finish with CR
		theTxt += CR;
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched)
	{
		// Start with <br> if not adding
		theTxt += "<excerpt>\r";
		theTxt += theText;
		theTxt += "</excerpt>";

		// Finish with CR
		theTxt += "\r\r";
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML)
	{
		// Start with <br> if not adding
		theTxt += "<blockquote>\r";
		theTxt += theText;
		theTxt += "\r</blockquote>\r";

		// Finish with CR
		theTxt += "<br>\r";
	}

	// Convert to utf16 and insert
	cdustring text_utf16(theTxt);
	mText->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);
	mText->SetDirty(true);
}

#pragma mark ____________________________Parts

// Handle twist of attachments
void CLetterWindow::DoPartsTwist()
{
	// Handle parts twist and focus
	DoPartsTwist(mPartsTwister->GetValue(), true);
}

// Is parts exposed
bool CLetterWindow::IsPartsTwist() const
{
	return mPartsTwister->GetValue() == 1;
}

// Handle twist of attachments
void CLetterWindow::DoPartsTwist(bool expand, bool change_focus)
{
	// Determine motion size and hide/show
	if (!expand)
	{
		// Hide the parts table pane in the splitter
		mSplitter->ShowView(false, true);

		// Focus on text if requested
		if (change_focus)
			SwitchTarget(mText);
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitter->ShowView(true, true);

		// Focus on parts if requested
		if (change_focus)
			SwitchTarget(mPartsTable);
	}

	// Always make sure twister is in sync as this can be called
	// without the twister being clicked
	if ((mPartsTwister->GetValue() == 1) ^ expand)
	{
		StopListening();
		mPartsTwister->SetValue(expand ? 1 : 0);
		StartListening();
	}

	// Update caption
	UpdatePartsCaption();

	Refresh();
}

// Update attachments caption
void CLetterWindow::UpdatePartsCaption(void)
{
	unsigned long count = mPartsTable->CountParts();
	mPartsNumber->SetNumber(count);

	mPartsIcon->SetVisible(mPartsTable->HasAttachments());
	
	// Use proper label for parts
	FindPaneByID(paneid_LetterPartsCaptionP)->SetVisible(!mPartsTable->GetAttachmentsOnly());
	FindPaneByID(paneid_LetterPartsCaptionA)->SetVisible(mPartsTable->GetAttachmentsOnly());
}

// Set body content
void CLetterWindow::SetBody(CAttachment* attach, bool converting)
{
	// Sync current data if converting to ensure it is not lost.
	// When converting we know that the current part is valid.
	if (converting)
		SyncPart();

	// Just replace - do not delete
	mBody = attach;
	mCurrentPart = NULL;
	CAttachment* part = (mBody != NULL) ? mBody->FirstEditPart() : NULL;

	if (part == NULL)
	{
		cdstring txt;
		txt.FromResource("Alerts::Letter::NoTextWarning");
		mText->SetText(txt);
		mText->SetReadOnly(true);
		HideToolbar();
	}
	else
		SetCurrentPart(part);
}

// Reset message text
void CLetterWindow::SetCurrentPart(CAttachment* part)
{
	// Sync current part
	SyncPart();

	// Get new part and copy text into window
	if (mBody)
	{
		if (part != NULL)
		{
			// Get part text
			mCurrentPart = part;
			const char* data = mCurrentPart->GetData();
			i18n::ECharsetCode charset = mCurrentPart->GetContent().GetCharset();

			// Do charset conversion to utf16
			cdustring data_utf16;
			std::ostrstream sout;
			if ((data != NULL))
			{
				if ((charset != i18n::eUTF16) && i18n::CCharsetManager::sCharsetManager.ToUTF16(charset, data, ::strlen(data), sout))
				{
					sout << std::ends << std::ends;
					data_utf16.steal(reinterpret_cast<unichar_t*>(sout.str()));
				}
				else
					data_utf16.assign(reinterpret_cast<const unichar_t*>(data));
			}

			const CMIMEContent& mime = mCurrentPart->GetContent();

			if (mime.GetContentType() == eContentText)
			{
				mText->SetText(NULL);
				mText->FocusDraw();
				{
					StStopRedraw noDraw(mText);
					bool html = ((mime.GetContentSubtype() == eContentSubEnriched) || (mime.GetContentSubtype() == eContentSubHTML)) &&
									CAdminLock::sAdminLock.mAllowStyledComposition;
					mText->SetTextTraits(html ? CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits : CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
					mText->Reset(true);
					mText->ParseBody(data_utf16, mime.GetContentSubtype(), eViewFormatted, -1);
					mText->InsertFormatted(html ? eViewFormatted : eViewPlain);
					mText->SetSelectionRange(0,0);
				}
				mText->ScrollImageTo(0, 0, false);
				mText->Refresh();
				// Make it active
				SwitchTarget(mText);
			}
			else
			{
				mText->SetText(data);
			}

			// Don't allow edit if reject
			mText->SetReadOnly(mReject);
			SetToolbar(mime);
		}
		else
		{
			mCurrentPart = NULL;
			cdstring txt;
			txt.FromResource("Alerts::Letter::NoTextWarning");
			mText->SetText(txt);
			mText->SetReadOnly(true);
			HideToolbar();
		}
	}
}

// Sync text in current part
void CLetterWindow::SyncPart(void)
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		char *data = NULL;
		{
			if (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched)
			{
				CEnrichedTransformer trans(mText);
				data = trans.Transform();
			}
			else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML)
			{
				CHTMLTransformer trans(mText);
				data = trans.Transform();
			}
			else
			{
				cdstring txt;
				mText->GetText(txt);
				data = txt.release();
			}

			mCurrentPart->GetContent().SetCharset(i18n::eUTF8);

			mCurrentPart->SetData(data);
		}
	}
}

// Sync text size in current part
void CLetterWindow::SyncPartSize(void)
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		SInt32 length = mText->GetTextLength();
		mCurrentPart->GetContent().SetContentSize(length);
	}
}

// Handle identities popup command
void CLetterWindow::DoStyleChangePopup(long index)
{
	EContentSubType subtype;

	switch(index)
	{
	case 1:
	case 2:
		subtype = eContentSubPlain;
		break;
	case 3:
		subtype = eContentSubEnriched;
		break;
	case 4:
		subtype = eContentSubHTML;
		break;
	}

	if (mCurrentPart && (mCurrentPart->GetContent().GetContentType() == eContentText))
	{
		// Do warning if changing from styled to unstyled
		const CMIMEContent& mime = mCurrentPart->GetContent();
		if ((mime.GetContentSubtype() != eContentSubPlain) &&
			(subtype == eContentSubPlain))
		{
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::WarnLoseFormatting") == CErrorHandler::Cancel)
			{
				// Change back to previous value
				switch(mime.GetContentSubtype())
				{
				case eContentSubPlain:
					index = mText->GetHardWrap() ? 2 : 1;
					break;
				case eContentSubEnriched:
					index = 3;
					break;
				case eContentSubHTML:
					index = 4;
					break;
				default:;
				}
				return;
			}
		}

		// Make sure wrapping correctly set before doing content change
		switch(index)
		{
		case 1:
		case 3:
		case 4:
			mText->SetHardWrap(false);
			break;
		case 2:
			mText->SetHardWrap(true);
			break;
		}

		mCurrentPart->GetContent().SetContent(eContentText, subtype);
		mPartsTable->ChangedCurrent();
	}
}

#pragma mark ____________________________Identities

// Handle identities popup command
void CLetterWindow::SetIdentityPopup(unsigned long index)
{
	StopListening();
	mIdentities->SetValue(mIdentities->FirstIndex() + index);
	StartListening();
}

// Handle identities popup command
void CLetterWindow::DoIdentitiesPopup(long index)
{
	switch(index)
	{
	// New identity wanted
	case eIdentityPopup_New:
		mIdentities->DoNewIdentity(CPreferences::sPrefs);
		break;

	// New identity wanted
	case eIdentityPopup_Edit:
		if (mIdentities->DoEditIdentity(CPreferences::sPrefs))
			// Reset to keep display in sync with changes
			SetIdentity(GetIdentity());
		break;

	// Delete existing identity
	case eIdentityPopup_Delete:
		mIdentities->DoDeleteIdentity(CPreferences::sPrefs);
		break;

	// Set custom identity
	case eIdentityPopup_Custom:
		{
			SetIdentity(mIdentities->GetCustomIdentity(), true);
		}
		break;

	// Select an identity
	default:
		{
			const CIdentity& identity = CPreferences::sPrefs->mIdentities.GetValue()[index - mIdentities->FirstIndex()];
			SetIdentity(&identity);
		}
		break;
	}
	
	// Check value and show/hide custom edit item
	mIdentityEditBtn->SetVisible(mIdentities->GetValue() == eIdentityPopup_Custom);
}

// Edit custom identity
void CLetterWindow::OnEditCustom()
{
	if (mIdentities->EditCustomIdentity())
		SetIdentity(mIdentities->GetCustomIdentity(), true);
}

// Change identity
void CLetterWindow::SetIdentity(const CIdentity* identity, bool custom, bool add_signature)
{
	// Preserve target
	LCommander* target = LCommander::GetTarget();

	// Must have identity
	if (!identity)
		identity = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Cache identity for use in message
	const CIdentity* old_identity = mCustomIdentity ? NULL : CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mIdentity);
	mIdentity = identity->GetIdentity();
	mCustomIdentity = custom;

	// Change over addresses only if not reject
	if (!mReject)
	{
		// First remove existing extra addresses if previous identity still exists
		if (old_identity)
		{
			cdstring temp;
			temp = old_identity->GetAddTo(true);
			CAddressList to_addrs(temp, temp.length());
			temp = old_identity->GetAddCC(true);
			CAddressList cc_addrs(temp, temp.length());
			temp = old_identity->GetAddBcc(true);
			CAddressList bcc_addrs(temp, temp.length());
			RemoveAddressLists(&to_addrs, &cc_addrs, &bcc_addrs);
		}

		// Add new extra addresses
		{
			cdstring temp;
			temp = identity->GetAddTo(true);
			CAddressList to_addrs(temp, temp.length());
			temp = identity->GetAddCC(true);
			CAddressList cc_addrs(temp, temp.length());
			temp = identity->GetAddBcc(true);
			CAddressList bcc_addrs(temp, temp.length());
			AddAddressLists(&to_addrs, &cc_addrs, &bcc_addrs);
		}
	}

	// Do Copy To
	mAppendList->SetSelectedMbox(identity->GetCopyTo(true),
									identity->GetCopyToNone(true),
									identity->GetCopyToChoose(true));
	mAppendList->Refresh();

	// Do Also Copy
	mCopyOriginal = identity->GetCopyReplied(true);

	// Do signature insert
	if (CPreferences::sPrefs->mAutoInsertSignature.GetValue() && !mReject)
	{
		// Must focus
		mText->FocusDraw();

		StStopRedraw noDraw(mText);

		// Get pos of insert cursor
		CTextDisplay::StPreserveSelection _selection(mText);

		// Replace signature
		mText->SetSelectionRange(mText->GetTextLength() - mSignatureLength, mText->GetTextLength());
		if (add_signature)
		{
			cdstring newsig = identity->GetSignature(true);

			// May need sigdashes before signature
			CPreferences::sPrefs->SignatureDashes(newsig);

			ReplaceSignature(mSignature, newsig);
			mSignature = newsig;
		}
		else
		{
			mText->InsertUTF8(cdstring::null_str);
			mSignature = cdstring::null_str;
			mSignatureLength = 0;
		}

		//mText->ScrollImageBy(0, 0, true);
		//mText->Refresh();
	}

	// Set sign/encryption buttons
	if (CPluginManager::sPluginManager.HasSecurity() && !mReject)
	{
		// Determine mode and any key
		mDoSign = identity->GetSign(true);
		mDoEncrypt = identity->GetEncrypt(true);
	}
	
	// Set DSN
	mDSN = identity->GetDSN(true);
	if (!mDSN.GetRequest() || mReject)
	{
		mDSN.SetSuccess(false);
		mDSN.SetFailure(false);
		mDSN.SetDelay(false);
		mDSN.SetFull(false);
	}

	// Restore target
	LCommander::SwitchTarget(target);

	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Insert signature
void CLetterWindow::InsertSignature(const cdstring& signature)
{
	// Cannot do this in read-only text
	if (mText->IsReadOnly())
		return;

	{
		StStopRedraw noDraw(mText);

		// Get pos of insert cursor
		SInt32 selStart;
		SInt32 selEnd;
		mText->GetSelectionRange(selStart, selEnd);

		bool char1 = false;
		bool char2 = false;
		// Get current text and lock while dereferences
		{
			cdustring utxt;
			mText->GetSpellTextRange(selStart - 2, selStart, utxt);
			if (utxt.length() > 0)
				char1 = (utxt[utxt.length() - 1] == '\r');

			if (utxt.length() > 1)
				char2 = (utxt[utxt.length() - 2] == '\r');
		}

		// Focus before fiddling with TEHandle
		StFocusTE clip_hide(mText);

		// Accumulate chages into one operation (for undo)
		cdstring replace_with;

		// Replace existing
		if (!char1 && CPreferences::sPrefs->mSignatureEmptyLine.GetValue())
			replace_with = os_endl2;
		else if (!char1 && !CPreferences::sPrefs->mSignatureEmptyLine.GetValue() ||
				 char1 && !char2 && CPreferences::sPrefs->mSignatureEmptyLine.GetValue())
			replace_with = os_endl;
		
		// May need sigdashes before signature
		cdstring temp(signature);
		CPreferences::sPrefs->SignatureDashes(temp);

		replace_with += temp;

		// Replace text
		mText->InsertUTF8(replace_with);
	}

	mText->ScrollImageBy(0, 0, true);
	mText->Refresh();
}

#pragma mark ____________________________Window changes

// Header twister clicked
void CLetterWindow::OnHeaderExpand()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;
	
	// Update recipient text if that is being shown
	if (!mHeaderState.mExpanded)
		UpdateRecipients();

	LayoutHeader();
}

// To twister clicked
void CLetterWindow::OnToTwist()
{
	// Toggle state
	mHeaderState.mToExpanded = (mToTwister->GetValue() == 1);
	DoTwist(mToMove, mToEdit, mToTwister);
}

// Cc twister clicked
void CLetterWindow::OnCcTwist()
{
	// Toggle state
	mHeaderState.mCcExpanded = (mCCTwister->GetValue() == 1);
	DoTwist(mCCMove, mCCEdit, mCCTwister);
}

// Bcc twister clicked
void CLetterWindow::OnBccTwist()
{
	// Toggle state
	mHeaderState.mBccExpanded = (mBCCTwister->GetValue() == 1);
	DoTwist(mBCCMove, mBCCEdit, mBCCTwister);
}

// To twister clicked
void CLetterWindow::DoTwist(LView* mover, CTextDisplay* aEdit, LDisclosureTriangle* aTwister)
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

// Add Cc button clicked
void CLetterWindow::OnAddCc()
{
	// Make the Cc header visible
	mHeaderState.mCcVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();

	// Focus on the Cc field
	SwitchTarget(mCCEdit);

	// Force main toolbar to update
	RefreshToolbar();
}

// Add Bcc button clicked
void CLetterWindow::OnAddBcc()
{
	// Make the Cc header visible
	mHeaderState.mBccVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();
	
	// Focus on the Bcc field
	SwitchTarget(mBCCEdit);

	// Force main toolbar to update
	RefreshToolbar();
}

// Layout header component based on state
void CLetterWindow::LayoutHeader()
{
	// Start with some initial offset
	SInt16 top = 2;
	
	SPoint32 header_loc;
	mHeaderMove->GetFrameLocation(header_loc);

	// Get the from area position and size which is always visible
	// Get current position
	SPoint32 pos;
	mFromMove->GetFrameLocation(pos);

	// Get current size
	SDimension16 size;
	mFromMove->GetFrameSize(size);

	// Move current top to bottom of from area
	top += size.height;

	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Hide recipients
		mRecipientView->Hide();
		
		// Show from (if more than one identity)
		// NB We need to determine numner of identities from the identity popup, because during a preference
		// change, the sPrefs may actually point to the old set, not the new one we need for this
		if (mIdentities->GetCount() > 1)
		{
			mFromView->Show();
		}
		else
		{
			mFromView->Hide();
			top -= size.height;
		}

		// Check for To field
		if (mHeaderState.mToVisible)
		{
			// Get current position
			mToMove->GetFrameLocation(pos);

			// Get current size
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
			mCCMove->GetFrameLocation(pos);

			// Get current size
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

		// Check for Bcc field
		if (mHeaderState.mBccVisible)
		{
			// Get current position
			mBCCMove->GetFrameLocation(pos);

			// Get current size
			mBCCMove->GetFrameSize(size);

			// Move to current top and increment top by height
			mBCCMove->MoveBy(0, top - (pos.v - header_loc.v), false);
			top += size.height;

			// Always show it
			mBCCMove->Show();
		}
		else
			// Always hide it
			mBCCMove->Hide();

		// Check for Subject field
		if (mHeaderState.mSubjectVisible)
		{
			// Get current position
			mSubjectMove->GetFrameLocation(pos);

			// Get current size
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
		// Hide from show recipients
		mFromView->Hide();
		mRecipientView->Show();

		// Hide all addresses and subject
		mToMove->Hide();
		mCCMove->Hide();
		mBCCMove->Hide();
		mSubjectMove->Hide();
	}
	
	// Get current position
	mPartsMove->GetFrameLocation(pos);

	// Get current size
	mPartsMove->GetFrameSize(size);

	// Move to current top and increment top by height
	mPartsMove->MoveBy(0, top - (pos.v - header_loc.v), false);
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

// Update recipients caption
void CLetterWindow::UpdateRecipients()
{
	// Get the address lists
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	
	GetAddressLists(to_list, cc_list, bcc_list);
	
	// Get the first one
	const CAddress* first_addr = NULL;
	if (to_list->size() != 0)
	{
		first_addr = to_list->front();
	}
	else if (cc_list->size() != 0)
	{
		first_addr = cc_list->front();
	}
	else if (bcc_list->size() != 0)
	{
		first_addr = bcc_list->front();
	}
	
	// Get total size
	unsigned long addrs = to_list->size() + cc_list->size() + bcc_list->size();

	// Create appropriate string
	cdstring txt;
	if (addrs == 0)
		txt.FromResource("UI::Letter::NoRecipients");
	else
	{
		txt += first_addr->GetFullAddress();
		if (addrs > 1)
		{
			cdstring temp;
			temp.FromResource("UI::Letter::AdditionalRecipients");
			temp.Substitute(addrs - 1);
			txt += temp;
		}
	}
	delete to_list;
	delete cc_list;
	delete bcc_list;
	
	mRecipientText->SetText(txt);
}

// Reset toolbar commands
void CLetterWindow::RefreshToolbar()
{
	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Reset text traits from prefs
void CLetterWindow::ResetTextTraits(const TextTraitsRecord& list_traits,
										const TextTraitsRecord& display_traits)
{
	mPartsTitles->SetTextTraits(list_traits);
	mPartsTable->SetTextTraits(list_traits);
	mPartsTitles->SyncTable(mPartsTable, true);
	mText->SetTextTraits(display_traits);
}

// Reset state from prefs
void CLetterWindow::ResetState(void)
{
	// Get window state
	CLetterWindowState& state = CPreferences::sPrefs->mLetterWindowDefault.Value();
	Rect set_rect = state.GetBestRect(state);

	// Do not set if empty
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// Force twist down if required
	mPartsTwister->SetValue(state.GetPartsTwisted());
	mSplitter->SetPixelSplitPos(state.GetSplitChange());
	mHeaderState.mExpanded = !state.GetCollapsed();

	// Force layout
	LayoutHeader();

	// Set zoom state
	if (state.GetState() == eWindowStateMax)
		DoSetZoom(true);
}

// Save current state in prefs
void CLetterWindow::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CLetterWindowState state(NULL, &mUserBounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mSplitter->GetPixelSplitPos(), (mPartsTwister->GetValue()==1), !mHeaderState.mExpanded);
	if (CPreferences::sPrefs->mLetterWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mLetterWindowDefault.SetDirty();

} // CLetterWindow::SaveDefaultState

// Check to see whether drop from another window
Boolean CLetterWindow::CheckIfViewIsAlsoSender(DragReference inDragRef)
{
	// Get drag attributes
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);

	// Compare with relevant flag
	return (theDragAttributes & kDragInsideSenderWindow);
}

// Check its a message
Boolean CLetterWindow::ItemIsAcceptable(
	DragReference	inDragRef,
	ItemReference	inItemRef)
{
	// Check for cDragMsgType
	FlavorFlags		theFlags;
	return (::GetFlavorFlags(inDragRef, inItemRef, cDragMsgType, &theFlags) == noErr);
}

// Receive a message
void CLetterWindow::ReceiveDragItem(
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

		// Get size of data
		::GetFlavorDataSize(inDragRef, inItemRef, cDragMsgType, &theDataSize);
		if (theDataSize) {
			ThrowIf_(theDataSize != sizeof(CMessage*));	// sanity check

			// Get the data about the message we are receiving.
			::GetFlavorData(inDragRef, inItemRef, cDragMsgType, &theFlavorData, &theDataSize, 0L);

			// Make sure text is loaded
			CAttachment* attach = theFlavorData->FirstDisplayPart();
			if (attach != NULL)
			{
				const char* msg_txt = NULL;
				try
				{
					msg_txt = attach->ReadPart(theFlavorData);
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Throw out of here as its terminal
					CLOG_LOGRETHROW;
					throw;
				}

				// Read text of specific message part
				i18n::ECharsetCode charset = attach->GetContent().GetCharset();

				// Convert to utf8
				cdstring utf8;
				if (msg_txt != NULL)
				{
					if (charset == i18n::eUTF8)
					{
						utf8 = msg_txt;
					}
					else
					{
						std::ostrstream sout;
						i18n::CCharsetManager::sCharsetManager.ToUTF8(charset, msg_txt, ::strlen(msg_txt), sout);
						sout << std::ends;
						utf8.steal(sout.str());
					}
				}

				// Include the message's text
				if (!utf8.empty())
					IncludeText(utf8, false);
			}
		}
	}
}

// Hilite/unhilite region two pixels wide round border
void CLetterWindow::HiliteDropArea(
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

void CLetterWindow::SetToolbar(const CMIMEContent& mime)
{
	if(mime.GetContentType() == eContentText)
		SetToolbar(mime.GetContentSubtype());
}

void CLetterWindow::SetToolbar(EContentSubType type)
{
	switch(type)
	{
	case eContentSubEnriched:
	case eContentSubHTML:
		if (CAdminLock::sAdminLock.mAllowStyledComposition)
			ShowToolbar(type);
		else
			HideToolbar();
		break;
	case eContentSubPlain:
	default:
		HideToolbar();
		break;
	}
	
	LCommander::SetUpdateCommandStatus(true);
}


void CLetterWindow::ShowToolbar(EContentSubType type, bool override)
{
	if(!(mToolbar->InUse()) || override){
		mToolbar->Show();
		mToolbar->Setup(type);
		mText->GetSuperView()->MoveBy(0, +24, false);
		mText->GetSuperView()->ResizeFrameBy(0, -24, false);

		// Change style of text viewer to accept styled D&D / Copy&Paste
		mText->SetAllowStyles(true);
		Refresh();
	}
	else{
		mToolbar->Setup(type);
	}
}

void CLetterWindow::HideToolbar(bool override){
	if(mToolbar->InUse() || override){
		//mEnriched->MoveBy(24,0,true);
		mToolbar->Hide();
		//mEnriched->Disable();
		mText->GetSuperView()->MoveBy(0, -24, false);
		mText->GetSuperView()->ResizeFrameBy(0, +24, false);

		// Change style of text viewer to reject styled D&D / Copy&Paste
		mText->SetAllowStyles(false);
		Refresh();
	}
}
