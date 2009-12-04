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
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CAttachment.h"
#include "CBodyTable.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CLetterWindow.h"
#include "CMailboxToolbarPopup.h"
#include "CMainMenu.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMenuBar.h"
#include "CMessageFwd.h"
#include "CMessage.h"
#include "CMessageHeaderView.h"
#include "CMessageList.h"
#include "CMessageSimpleView.h"
#include "CMessageToolbar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CTableScrollbarSet.h"
#include "CSimpleTitleTable.h"
#include "CSpacebarEdit.h"
#include "CSplitterView.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CToolbarButton.h"
#include "CToolbarView.h"
#include "CTwister.h"
#include "CWindowsMenu.h"
#include "CWaitCursor.h"

#include "TPopupMenu.h"
#include "HResourceMap.h"

#include <jXActionDefs.h>
#include <JXApplication.h>
#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXFlatRect.h>
#include <JXImageButton.h>
#include <JXImageCheckbox.h>
#include <JXImageWidget.h>
#include <JXMenuBar.h>
#include "JXMultiImageButton.h"
#include <JXStaticText.h>
#include <JXScrollbarSet.h>
#include <JXTextMenu.h>
#include <JXUpRect.h>
#include <JXWidgetSet.h>
#include <JXWindow.h>

#include <algorithm>
#include <fstream>
#include <stdio.h>
#include <strstream>

const int cWindowWidth = 575;
const int cWindowHeight = 300;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cViewTop = cToolbarTop + cToolbarHeight;
const int cViewHeight = cWindowHeight - cViewTop;

const int cCaptionHeight = 30;
const int cSecurePaneHeight = 34;
const int cHeaderHeight = 174;
const int cHeaderCollapsedHeight = 64;
const int cSplitterTop = cHeaderHeight;
const int cSplitterHeight = cViewHeight - cHeaderHeight;
const int cPartsCollapsedHeight = 24;
const int cPartsMinimumHeight = 84;
const int cTextHeight = cViewHeight - cPartsCollapsedHeight;

const unsigned int cMaxTitleLength = 64;				// Maximum number of chars in window title
const int cTwistMove = 32;
const int cPartsTwistMove = 76;

/////////////////////////////////////////////////////////////////////////////
// CMessageWindow

/////////////////////////////////////////////////////////////////////////////
// CMessageWindow construction/destruction

cdmutexprotect<CMessageWindow::CMessageWindowList> CMessageWindow::sMsgWindows;
//CMultiDocTemplate* CMessageWindow::sMessageDocTemplate = NULL;

cdstring CMessageWindow::sNumberString;						// snprintf string for message number
cdstring CMessageWindow::sNumberStringBig;					// snprintf string for message number >= 10000
cdstring CMessageWindow::sSubMessageString;					// snprintf string for sub-message
cdstring CMessageWindow::sDigestString;						// snprintf string for digestive message

cdstring  CMessageWindow::sLastCopyTo;

// Default constructor
CMessageWindow::CMessageWindow(JXDirector* supervisor) 
  : super(supervisor, "", kFalse, kFalse, ".txt")
{
	// Add to list
	{
		cdmutexprotect<CMessageWindowList>::lock _lock(sMsgWindows);
		sMsgWindows->push_back(this);
	}

	CWindowsMenu::AddWindow(this);

	mItsMsg = NULL;
	mWasUnseen = false;
	mMsgs = NULL;
	mItsMsgError = false;
	mShowText = NULL;
	mCurrentPart = NULL;

	mShowSecure = true;
	mSecureMulti = true;
	mShowHeader = CPreferences::sPrefs->showMessageHeader.GetValue();
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

	mColorList = NULL;

	// Load strings if required
	if (!sNumberString.length())
		sNumberString.FromResource("UI::Message::MessageNumber");
	if (!sNumberStringBig.length())
		sNumberStringBig.FromResource("UI::Message::MessageNumberBig");
	if (!sSubMessageString.length())
		sSubMessageString.FromResource("UI::Message::SubMessage");
	if (!sDigestString.length())
		sDigestString.FromResource("UI::Message::DigestMessage");
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

	// Set status
	SetClosed();
	
	delete mColorList;
}

// Manually create document
CMessageWindow* CMessageWindow::ManualCreate(bool hidden)
{
	CMessageWindow* pWnd = new CMessageWindow(CMulberryApp::sApp);
	pWnd->OnCreate();
	if (!hidden)
	{
		pWnd->ResetState(true);
		pWnd->Activate();
	}
	return (CMessageWindow*) pWnd;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

void CMessageWindow::OnCreate()
{
	JXWindow* window = new JXWindow(this, cWindowWidth, cWindowHeight, "Message");
	SetWindow(window);
	
	CFileDocument::OnCreate();
 
	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Create server view
	mView = new CMessageSimpleView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cViewTop, cWindowWidth, cViewHeight);
	mView->OnCreate();

	// Create toolbars for a view we own
	mToolbarView->SetSibling(mView);
	mView->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set the copy to back to the last value used
	if (mView->GetToolbar()->GetCopyBtn())
		mView->GetToolbar()->GetCopyBtn()->SetSelectedMbox(sLastCopyTo, false);

	// Header view
	mHeader = 
		new CMessageHeaderView(mView, mainMenus[CMainMenu::eEdit],
								 JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cWindowWidth, cHeaderHeight);
	mHeader->OnCreate(this);

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cPartsCollapsedHeight);
	heights.InsertElementAtIndex(2, cTextHeight);
	minHeights.InsertElementAtIndex(1, cPartsCollapsedHeight);
	minHeights.InsertElementAtIndex(2, 16);
	
	mSplitterView = new CSplitterView(heights, 0, minHeights, mView,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);

	// Now create sub-views
    mPartsScroller =
        new CTableScrollbarSet(mSplitterView, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, cWindowWidth, cSplitterHeight);

	mPartsTable = new 
	  CBodyTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0, cTitleHeight, cWindowWidth, cSplitterHeight);

	mPartsTitles = new CSimpleTitleTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
														JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cWindowWidth, cTitleHeight);
	mPartsTable->OnCreate();
	mPartsTitles->OnCreate();

	mPartsTitles->SyncTable(mPartsTable, true);
	mPartsTable->SetTitles(mPartsTitles);

	mPartsTitles->LoadTitles("UI::Titles::MessageParts", 5);
								 
    mBottomPane =
        new JXFlatRect(mSplitterView, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, cWindowWidth, cSplitterHeight);

    mSecurePane =
        new JXUpRect(mBottomPane, JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, cWindowWidth, cSecurePaneHeight);
	mSecureInfo = 
	  new CTextDisplay(static_cast<JXScrollbarSet*>(NULL), mSecurePane, JXWidget::kHElastic, JXWidget::kVElastic, 2, 2, cWindowWidth - 4, cSecurePaneHeight - 4);
	mSecureInfo->SetBackgroundColor(GetColormap()->GetDefaultBackColor());
	mSecureInfo->SetBreakCROnly(kTrue);
	mSecureInfo->SetBorderWidth(0);
	mColorList = new JColorList(GetColormap());

	mTextDisplay = mText = 
	  new CFormattedTextDisplay(mBottomPane, JXWidget::kHElastic, JXWidget::kVElastic,
										0, cSecurePaneHeight, cWindowWidth, cSplitterHeight - cSecurePaneHeight);
	mText->OnCreate();
	mText->SetMessageWindow(this);
	mText->FitToEnclosure(kTrue, kFalse);
#ifndef USE_FONTMAPPER
	mText->ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), mFontScale);
#else
	mText->ResetFont(CPreferences::sPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, mFontScale);
#endif
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetFindAllowed(true);

	// Hide the verify/decrypt details
	ShowSecretPane(false);

	// Install the splitter items
	mSplitterView->InstallViews(mPartsScroller, mBottomPane, true);
	mSplitterView->ShowView(false, true);
	mSplitterView->SetMinima(84, 64);
	mSplitterView->SetPixelSplitPos(84);
	mSplitterView->SetLockResize(true);

	// spacebar handling
	mHeader->mFromField->SetMessageWindow(this);
	mHeader->mToField->SetMessageWindow(this);
	mHeader->mCCField->SetMessageWindow(this);
	mHeader->mSubjectField->SetMessageWindow(this);
	mHeader->mDateField->SetMessageWindow(this);

	// Toggle header button
	if (mShowHeader)
		mHeader->mHeaderBtn->ToggleState();
	
	// Listen to widgets
	ListenTo(mHeader->mFromTwister);
	ListenTo(mHeader->mToTwister);
	ListenTo(mHeader->mCCTwister);
	ListenTo(mHeader->mHeaderBtn);
	ListenTo(mHeader->mPartsTwister);
	ListenTo(mHeader->mFlatHierarchyBtn);
	ListenTo(mHeader->mTextFormat);
	ListenTo(mHeader->mFontIncreaseBtn);
	ListenTo(mHeader->mFontDecreaseBtn);
	ListenTo(mHeader->mQuoteDepthPopup);
	mParsing = eViewFormatted;
	mHeader->mTextFormat->SetToPopupChoice(kTrue, mParsing);

	// Tooltips
	mHeader->mFromTwister->SetHint(stringFromResource(IDC_MESSAGEFROMTWISTER));
	mHeader->mFromField->SetHint(stringFromResource(IDC_MESSAGEFROMFIELD));
	mHeader->mToTwister->SetHint(stringFromResource(IDC_MESSAGETOTWISTER));
	mHeader->mToField->SetHint(stringFromResource(IDC_MESSAGETOFIELD));
	mHeader->mCCTwister->SetHint(stringFromResource(IDC_MESSAGECCTWISTER));
	mHeader->mCCField->SetHint(stringFromResource(IDC_MESSAGECCFIELD));
	mHeader->mSubjectField->SetHint(stringFromResource(IDC_MESSAGESUBJECT));
	mHeader->mDateField->SetHint(stringFromResource(IDC_MESSAGEDATE));

	mHeader->mPartsTwister->SetHint(stringFromResource(IDC_MESSAGEPARTSTWISTER));
	mHeader->mPartsField->SetHint(stringFromResource(IDC_MESSAGEPARTSFIELD));
	mHeader->mFlatHierarchyBtn->SetHint(stringFromResource(IDC_MESSAGEPARTSFLATBTN));
	mHeader->mAttachments->SetHint(stringFromResource(IDC_MESSAGEPARTSATTACHMENTS));
	mHeader->mAlternative->SetHint(stringFromResource(IDC_MESSAGEPARTSALTERNATIVE));

	mHeader->mHeaderBtn->SetHint(stringFromResource(IDC_MESSAGES_SHOW_HEADER));
	mHeader->mTextFormat->SetHint(stringFromResource(IDC_MESSAGES_TEXT_FORMAT));

	// Set current width and height as minimum
	GetWindow()->SetWMClass(cMessageWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_MESSAGEDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	CreateMainMenu(window,
					 CMainMenu::fFile |
					 CMainMenu::fEdit |
					 CMainMenu::fMessages | 
					 CMainMenu::fAddresses |
					 CMainMenu::fCalendar | 
					 CMainMenu::fWindows | 
					 CMainMenu::fHelp);
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());

	// Set status
	SetOpen();

	return;
}

void CMessageWindow::Activate()
{
	CFileDocument::Activate();
	
	mView->RefreshToolbar();
}

void CMessageWindow::DoSaveAs()
{
	SaveInNewFile();
}

// Save data to file
void CMessageWindow::WriteTextFile(std::ostream& output, const JBoolean safetySave) const
{
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
				output << os_endl;

			// Add header if required or in raw mode
			if (CPreferences::sPrefs->saveMessageHeader.GetValue() || (GetViewAs() == eViewAsRaw))
				output << (*iter)->GetHeader();

			// Write the text
			const unichar_t* txt =  const_cast<CMessageWindow*>(this)->GetSaveText();
			if (txt)
			{
				cdustring temp(txt);
				cdstring utf8 = temp.ToUTF8();
				output << utf8;
			}
		}
	}
	else
	{
		// Write header if required
		if (CPreferences::sPrefs->saveMessageHeader.GetValue())
			output << mItsMsg->GetHeader();

		// Write text to file stream
		if (mShowText)
			output << mShowText;
	}
}

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

// Add address list to caption/field:
void CMessageWindow::SetTwistList(CTextBase* aField,
				  CTwister* aTwister,
				  JXDecorRect* aMover,
				  CAddressList* list)
{

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
			first = false;
		else
			got_two = true;
	}

	// Update field - will null if no addresses
	aField->SetText(to_addrs.c_str());

	// Disable twister if more than one line
	if (aField->GetLineCount() < 2)
		aTwister->Deactivate();
	else
		aTwister->Activate();

	// Force twist down if more than one address and preference set
	if (CPreferences::sPrefs->mExpandHeader.GetValue() &&
		((aTwister->IsChecked() && !got_two) ||
		 (!aTwister->IsChecked() && got_two)))
	  //if we toggle the state, we should get the message and do
	  //the right update, so we don't call DoTwist ourselves
	  aTwister->ToggleState();
}

void CMessageWindow::SetMessage(CMessage* theMsg)
{
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
	cdstring		theTxt;
	cdstring		theTitle;

	// Set From: text
	SetTwistList(mHeader->mFromField, mHeader->mFromTwister, mHeader->mFromMove, theEnv->GetFrom());

	// Set To: text
	SetTwistList(mHeader->mToField, mHeader->mToTwister, mHeader->mToMove, theEnv->GetTo());

	// Set CC: text
	SetTwistList(mHeader->mCCField, mHeader->mCCTwister, mHeader->mCCMove, theEnv->GetCC());

	// Set Subject: text
	theTxt = theEnv->GetSubject();
	mHeader->mSubjectField->SetText(theTxt.c_str());

	// Set Date: text
	theTxt = theEnv->GetTextDate(true, true);
	mHeader->mDateField->SetText(theTxt.c_str());

	// Set window title to subject (if there), else from (if there) else unknown
	if (!theEnv->GetSubject().empty())
	{
		unsigned short subj_len = theEnv->GetSubject().length();
		if (subj_len < cMaxTitleLength)
			theTitle= theEnv->GetSubject();
		else
			theTitle = cdstring(theEnv->GetSubject(), 0, cMaxTitleLength);
	}
	else if (theEnv->GetFrom()->size())
	{
		theTitle = theEnv->GetFrom()->front()->GetNamedAddress();
		theTitle.erase(cMaxTitleLength);
	}
	else
		theTitle.FromResource("UI::Message::NoSubject");

	mText->GetWindow()->SetTitle(theTitle.c_str());

	// Set the file name to use when saving
	// Make it sage for unix
	::strreplace(theTitle.c_str_mod(), "/", '_');
	FileChanged(theTitle.c_str(), kFalse);

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
	UpdatePartsCaption();

	// Reset text
	CAttachment* attach = mItsMsg->FirstDisplayPart();
	ShowPart(attach);
	mPartsTable->SetRowShow(attach);

	// If raw mode read in raw body
	if (mParsing == eViewAsRaw)
	{
		// Handle cancel or failure of raw mode
		if (!ShowRawBody())
		{
			// Reset to formatted parse and update popup
			mParsing = eViewFormatted;
			mHeader->mTextFormat->SetToPopupChoice(kTrue, mParsing);
		}
		
		// Always reset the text as either the text has changed or a new format display is being used
		ResetText();
	}

	// Do attachments icon
	if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1))
	{
		if (mItsMsg->GetBody()->HasUniqueTextPart())
		{
			mHeader->mAttachments->Hide();
			mHeader->mAlternative->Show();
		}
		else
		{
			mHeader->mAttachments->Show();
			mHeader->mAlternative->Hide();
		}
	}
	else
	{
		mHeader->mAttachments->Hide();
		mHeader->mAlternative->Hide();
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
	if (GetWindow()->IsVisible())
	{
		// Do parts expand if no visible part or multiparts
		if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
		{
			// Do auto expansion or expand if no parts
			if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
				!mHeader->mPartsTwister->IsChecked())
			{
				mHeader->mPartsTwister->SetState(kTrue);
				mDidExpandParts = true;

				// Must set focus back to text as expanding parts sets it to the parts table
				mText->Focus();
			}
		}
		else
		{
			// Do auto collapse
			if (CPreferences::sPrefs->mExpandParts.GetValue() &&
				mHeader->mPartsTwister->IsChecked() && mDidExpandParts)
			{
				mHeader->mPartsTwister->SetState(kFalse);
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
			CPreferences::sPrefs->mAttachmentNotification.GetValue().DoNotification("Alerts::Message::MessageHasAttachments");
		}
		
		// Look for top-level multipart/digest
		if (CPreferences::sPrefs->mAutoDigest.GetValue() && mItsMsg)
		{
			CAttachment* digest = mItsMsg->GetBody()->GetFirstDigest();
			if (digest)
				ShowSubMessage(digest);
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
	mPartsTable->ClearBody();
}

void CMessageWindow::SetMessageList(CMessageList* msgs)
{
	mMsgs = msgs;

	CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
	cdstring		theTitle;

	// Set window title to subject (if there), else from (if there) else unknown
	if (!theEnv->GetSubject().empty())
	{
		unsigned short subj_len = theEnv->GetSubject().length();
		if (subj_len < cMaxTitleLength)
			theTitle= theEnv->GetSubject();
		else
			theTitle = cdstring(theEnv->GetSubject(), 0, cMaxTitleLength);
	}
	else if (theEnv->GetFrom()->size())
	{
		theTitle = theEnv->GetFrom()->front()->GetNamedAddress();
		theTitle.erase(cMaxTitleLength);
	}
	else
		theTitle.FromResource("UI::Message::NoSubject");

	mText->GetWindow()->SetTitle(theTitle.c_str());

	// Set the file name to use when saving
	// Make it sage for unix
	::strreplace(theTitle.c_str_mod(), "/", '_');
	FileChanged(theTitle.c_str(), kFalse);
}

// Someone else changed this message
void CMessageWindow::MessageChanged(void)
{
	// Prevent redisplay if blocked
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
			if (mItsMsg->CountDigest())
			{
				char buf[64];
			    ::snprintf(buf, 64, sDigestString.c_str(), mItsMsg->GetDigestNumber(), mItsMsg->CountDigest());
			    newTxt = buf;
			}
			else
				newTxt = sSubMessageString;
		}
		else
		{
		  char buf[64];
		  if (mItsMsg->GetMbox()->GetNumberFound() < 10000)
		  	::snprintf(buf, 64, sNumberString.c_str(), mItsMsg->GetMessageNumber(), mItsMsg->GetMbox()->GetNumberFound());
		  else
		  	::snprintf(buf, 64, sNumberStringBig.c_str(), mItsMsg->GetMessageNumber(), mItsMsg->GetMbox()->GetNumberFound());
		  newTxt = buf;
		}
	}

	if (newTxt != cdstring(mHeader->mNumberField->GetText()))
		mHeader->mNumberField->SetText(newTxt.c_str());
	
	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Reset message text
void CMessageWindow::ResetText(void)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Start cursor for busy operation
	CWaitCursor wait;

	// Determine actual view type and content
	bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
	EView actual_view = mParsing;
	bool actual_styles = CPreferences::sPrefs->mUseStyles.GetValue();
	EContentSubType actual_content = use_part ? mCurrentPart->GetContent().GetContentSubtype() : eContentSubPlain;
	i18n::ECharsetCode charset = use_part ? mCurrentPart->GetContent().GetCharset() : i18n::eUSASCII;
	const SFontInfo* actual_font = NULL;
	switch(mParsing)
	{
	case eViewFormatted:
		{
			// Use plain text font when not using styles
			bool html = ((actual_content == eContentSubEnriched) || (actual_content == eContentSubHTML)) && actual_styles;
						 
			actual_font = html ? &CPreferences::sPrefs->mHTMLTextFontInfo.GetValue() : &CPreferences::sPrefs->mDisplayTextFontInfo.GetValue();
		}
		break;
	case eViewPlain:
	case eViewRaw:
	case eViewAsRaw:
		actual_font = &CPreferences::sPrefs->mDisplayTextFontInfo.GetValue();
		break;
	case eViewAsHTML:
		actual_view = eViewFormatted;
		actual_styles = true;				// Force style rendering on
		actual_content = eContentSubHTML;
		actual_font = &CPreferences::sPrefs->mHTMLTextFontInfo.GetValue();
		break;
	case eViewAsEnriched:
		actual_view = eViewFormatted;
		actual_styles = true;				// Force style rendering on
		actual_content = eContentSubEnriched;
		actual_font = &CPreferences::sPrefs->mHTMLTextFontInfo.GetValue();
		break;
	case eViewAsFixedFont:
		actual_view = eViewFormatted;
		actual_font = &CPreferences::sPrefs->mFixedTextFontInfo.GetValue();
		break;
	}

	{
		mText->ResetFont(*actual_font, mFontScale);
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

			mText->ParseHeader(hdr.str(), actual_view);
			hdr.freeze(false);
		}

		// Only do if message exists
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

		// Give text to control
		mText->InsertFormatted(actual_view);

		// Set selection at start
		mText->SetSelectionRange(0, 0);
	}

	// Make it active
	mText->Focus();
}

// Reset font scale text
void CMessageWindow::ResetFontScale()
{
	// Set text
	cdstring txt;
	txt.reserve(32);
	if (mFontScale != 0)
		::snprintf(txt.c_str_mod(), 32, "%+ld", mFontScale);
	mHeader->mFontScaleField->SetText(txt);

	// Enable/disable controls
	mHeader->mFontIncreaseBtn->SetActive(JBoolean(mFontScale < 7));
	mHeader->mFontDecreaseBtn->SetActive(JBoolean(mFontScale > -5));
}

// Update attachments caption
void CMessageWindow::UpdatePartsCaption(void)
{
	long count = 0;
	if (mPartsTable->GetFlat())
	{
		TableIndexT rows;
		TableIndexT cols;
		mPartsTable->GetTableSize(rows, cols);
		count = rows;
	}
	else
		count = mItsMsg ? mItsMsg->GetBody()->CountParts() : 0;

	cdstring caption;
	if ((count >= 1) || !mItsMsg || !mItsMsg->GetBody()->CanDisplay())
		caption = count;
	else
		caption.FromResource("Alerts::Letter::NoAttachments");

	mHeader->mPartsField->SetText(caption.c_str());
}

// Update list of attachments, show/hide
void CMessageWindow::UpdatePartsList(void)
{
	mPartsTable->SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
}

// Reset message text
void CMessageWindow::ShowSubMessage(CAttachment* attach)
{
	// Check that its a message part
	if (!attach->IsMessage())
		return;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Read the message
		// attach->GetMessage()->ReadPart(); <- Not required - done in new message window

		// Create the message window
		newWindow = CMessageWindow::ManualCreate();
		newWindow->SetMessage(attach->GetMessage());
		
		// Stagger relative to this
		JRect frame = GetWindow()->GetFrame();
		frame.Shift(20, 20);
		newWindow->GetWindow()->Place(frame.left, frame.top);
		newWindow->GetWindow()->SetSize(frame.width(), frame.height());
		newWindow->GetWindow()->Show();
		//newWindow->GetText()->UpdateMargins();
		
		attach->SetSeen(true);
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
	bool multi_line = false;

	{
		// Don't allow drawing until complete
		StNoRedraw _noredraw(mSecureInfo);

		// Delete any previous text
		mSecureInfo->SetText(cdstring::null_str);

		if (info.GetSuccess())
		{
			if (info.GetDidSignature())
			{
				mSecureInfo->SetCurrentFontBold(kTrue);
				cdstring txt;
				if (info.GetSignatureOK())
				{
					txt += "Signature: OK";
					JRGB green(0x0000, 0xDDDD, 0x0000);
					mSecureInfo->SetCurrentFontColor(mColorList->Add(green));
				}
				else
				{
					txt += "Signature: Bad";
					JRGB red(0xDDDD, 0x0000, 0x0000);
					mSecureInfo->SetCurrentFontColor(mColorList->Add(red));
				}
				mSecureInfo->InsertUTF8(txt);

				JRGB black(0x0000, 0x0000, 0x0000);
				mSecureInfo->SetCurrentFontColor(mColorList->Add(black));
				txt = "    Signed By: ";
				mSecureInfo->InsertUTF8(txt);

				mSecureInfo->SetCurrentFontBold(kFalse);

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
					JRGB red(0xDDDD, 0x0000, 0x0000);
					mSecureInfo->SetCurrentFontColor(mColorList->Add(red));
					
					addr += " WARNING: Does not match From address";
				}

				// Insert address data
				mSecureInfo->InsertUTF8(addr);

				// Next line
				if (info.GetDidDecrypt())
				{
					mSecureInfo->InsertUTF8(os_endl);
					multi_line = true;
				}
			}

			if (info.GetDidDecrypt())
			{
				mSecureInfo->SetCurrentFontBold(kTrue);
				JRGB green(0x0000, 0xDDDD, 0x0000);
				mSecureInfo->SetCurrentFontColor(mColorList->Add(green));

				cdstring txt;
				txt += "Decrypted: OK";
				mSecureInfo->InsertUTF8(txt);

				JRGB black(0x0000, 0x0000, 0x0000);
				mSecureInfo->SetCurrentFontColor(mColorList->Add(black));
				txt = "    Encrypted To: ";
				mSecureInfo->InsertUTF8(txt);

				mSecureInfo->SetCurrentFontBold(kFalse);

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
			mSecureInfo->SetCurrentFontBold(kTrue);
			JRGB red(0xDDDD,0x0000,0x0000);
			mSecureInfo->SetCurrentFontColor(mColorList->Add(red));

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

	// Reset selection to start and force redraw
	mSecureInfo->SetSelectionRange(0, 0);
	mSecureInfo->Refresh();

	// Check multi-line state
	if (mSecureMulti ^ multi_line)
	{
		const JCoordinate moveby = 12;

		if (multi_line)
		{
			if (mShowSecure)
			{
				mText->GetScroller()->AdjustSize(0, -moveby);
				mText->GetScroller()->Move(0, moveby);
			}
			mSecurePane->AdjustSize(0, moveby);
		}
		else
		{
			mSecurePane->AdjustSize(0, -moveby);
			if (mShowSecure)
			{
				mText->GetScroller()->AdjustSize(0, moveby);
				mText->GetScroller()->Move(0, -moveby);
			}
		}

		mSecureMulti = multi_line;
	}
}

// Reset fonts
void CMessageWindow::ResetFont(const SFontInfo& list_font, const SFontInfo& display_font)
{
	mPartsTable->ResetFont(list_font);
	mPartsTitles->ResetFont(list_font);
	mPartsTitles->SyncTable(mPartsTable, true);
	mText->ResetFont(display_font, mFontScale);
}

// Temporarily add header summary for printing
void CMessageWindow::AddPrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader && mHeaderState.mExpanded))
	{
		// Get summary from envelope
		std::ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());

		// Parse as header
		mText->InsertFormattedHeader(header_insert.c_str());
	}
}

// Remove temp header summary after printing
void CMessageWindow::RemovePrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader && mHeaderState.mExpanded))
	{
		// Get summary from envelope
		std::ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());

		// Parse as header
		mText->SetSelectionRange(0, header_insert.length());
		mText->InsertUTF8(cdstring::null_str);
	}
}

#pragma mark ____________________________________Command Updaters

// Common updaters
void CMessageWindow::OnUpdateMessageReadPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true) ||
					 mItsMsg->IsSubMessage() && mItsMsg->GetPrevDigest()));
}

void CMessageWindow::OnUpdateMessageReadNext(CCmdUI* pCmdUI)
{
	if (mItsMsg &&
		(!mItsMsg->IsSubMessage() ||
			mItsMsg->IsSubMessage() && mItsMsg->GetNextDigest()))
	{
		pCmdUI->Enable(true);
		
		// Check for toolbar button
		if (pCmdUI->mOther && dynamic_cast<CToolbarButton*>(pCmdUI->mOther))
		{
			CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(pCmdUI->mOther);
			ResIDT icon;
			if (mItsMsg->IsSubMessage())
			{
				// Reset navigation buttons
				icon = mItsMsg->GetNextDigest() ? IDI_NEXTMSG : IDI_LASTNEXTMSG;
			}
			else
			{
				// Reset navigation buttons
				icon = mItsMsg->GetMbox()->GetNextMessage(mItsMsg, true) ? IDI_NEXTMSG : IDI_LASTNEXTMSG;
			}
			tbtn->SetImage(icon, 0);
			tbtn->Refresh();
		}
	}
	else
		pCmdUI->Enable(false);
}

void CMessageWindow::OnUpdateMessageCopyNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg != NULL);
}

void CMessageWindow::OnUpdateMessageReject(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowRejectCommand);
}

void CMessageWindow::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(mItsMsg && mItsMsg->GetEnvelope()->GetFrom()->size() &&
					CPreferences::TestSmartAddress(*mItsMsg->GetEnvelope()->GetFrom()->front()));
}

void CMessageWindow::OnUpdateMessageCopy(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	if (pCmdUI->mMenu)
		CCopyToMenu::ResetMenuList(pCmdUI->mMenu, NULL);

	// Do update for non-deleted selection
	pCmdUI->Enable(true);
	const JXMenu* submenu = NULL;
	if (pCmdUI->mMenu && pCmdUI->mMenu->GetSubmenu(pCmdUI->mMenuIndex, &submenu) &&
		(dynamic_cast<const CCopyToMenu::CCopyToSub*>(submenu) != NULL))
		ListenTo(submenu);

	// Adjust menu title move/copy
	cdstring txt;
	switch(pCmdUI->mCmd)
	{
	case CCommand::eMessagesCopyNow:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVENOW_CMD_TEXT : IDS_COPYNOW_CMD_TEXT);
		break;
	case CCommand::eToolbarMessageCopyBtn:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETO_CMD_TEXT : IDS_COPYTO_CMD_TEXT);
		break;
	case CCommand::eMessagesCopyTo:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
		break;
	default:;
		break;
	}

	if (!txt.empty())
		pCmdUI->SetText(txt);
}

void CMessageWindow::OnUpdateMessageDelete(CCmdUI* pCmdUI)
{
	// Only if not sub-message and not read only
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eDeleted));
	
	// Change title of Delete menu item but not Delete & Next
	if ((pCmdUI->mCmd == CCommand::eMessagesDelete) ||
		(pCmdUI->mCmd == CCommand::eToolbarMessageDeleteBtn))
	{
		cdstring txt;
		txt.FromResource(mItsMsg && mItsMsg->IsDeleted() ? IDS_UNDELETE_CMD_TEXT : IDS_DELETE_CMD_TEXT);
		
		pCmdUI->SetText(txt);

		if (!pCmdUI->mMenu)
			pCmdUI->SetCheck(mItsMsg && mItsMsg->IsDeleted());
	}
}

void CMessageWindow::OnUpdateMessageViewCurrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mCurrentPart != NULL) && (mParsing != eViewAsRaw));
}

void CMessageWindow::OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity());
}

void CMessageWindow::OnUpdateMessageFlagsTop(CCmdUI* pCmdUI)
{
	bool enabled = mItsMsg && !mItsMsg->IsSubMessage() &&
					mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eIMAPFlags);
	pCmdUI->Enable(enabled);
}

void CMessageWindow::OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eSeen);
}

void CMessageWindow::OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eFlagged);
}

void CMessageWindow::OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eAnswered);
}

void CMessageWindow::OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDeleted);
}

void CMessageWindow::OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDraft);
}

void CMessageWindow::OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->mCmd - CCommand::eFlagsLabel1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->mCmd - CCommand::eFlagsLabel1]->name);
}

void CMessageWindow::OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag)
{
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(flag));
	pCmdUI->SetCheck(mItsMsg && mItsMsg->HasFlag(flag));
}

void CMessageWindow::OnUpdateMessageShowHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(!mShowHeader ? IDS_SHOWHEADER : IDS_HIDEHEADER);
	
	pCmdUI->SetText(txt);
}

void CMessageWindow::OnUpdateMenuExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mHeaderState.mExpanded ? IDS_COLLAPSEHEADER : IDS_EXPANDHEADER);
	
	pCmdUI->SetText(txt);
}

void CMessageWindow::OnUpdateExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
}

void CMessageWindow::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(!mHeader->mPartsTwister->IsChecked() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	pCmdUI->SetText(txt);
}

//#pragma mark ____________________________________Command Handlers

void CMessageWindow::OnFileNewDraft()
{
	DoNewLetter(GetDisplay()->GetLatestKeyModifiers().control());
}

void CMessageWindow::OnMessageCopyRead(void)
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();
	CopyReadNextMessage(option_key);
}

void CMessageWindow::OnMessageReply(void)
{
	ReplyToThisMessage(replyReplyTo, GetDisplay()->GetLatestKeyModifiers().control());
}

void CMessageWindow::OnMessageReplySender(void)
{
	ReplyToThisMessage(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageWindow::OnMessageReplyFrom(void)
{
	ReplyToThisMessage(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageWindow::OnMessageReplyAll(void)
{
	ReplyToThisMessage(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

// Forward this message
void CMessageWindow::OnMessageForward(void)
{
	ForwardThisMessage(GetDisplay()->GetLatestKeyModifiers().control());
}

// Copy the message to chosen mailbox
void CMessageWindow::OnMessageCopyBtn()
{
	TryCopyMessage(GetDisplay()->GetLatestKeyModifiers().control());
}

// Copy the message to another mailbox
void CMessageWindow::OnMessageCopy(JXTextMenu* menu, JIndex nID)
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();

	// Get mbox corresponding to menu selection
	CMbox* mbox = NULL;
	if (CCopyToMenu::GetMbox(menu, true, nID, mbox))
	{
		if (mbox)
			CopyThisMessage(mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

// Copy the message
void CMessageWindow::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMessageWindow::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
}

// Try to copy a message
bool CMessageWindow::TryCopyMessage(bool option_key)
{
	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (mView->GetToolbar()->GetCopyBtn() && mView->GetToolbar()->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
		{
			// Save the name of the mailbox being used
			mView->GetToolbar()->GetCopyBtn()->GetSelectedMboxName(sLastCopyTo, false);
			return CopyThisMessage(mbox, option_key);
		}
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}

	return false;
}

void CMessageWindow::OnTextFormatPopup(JIndex nID)
{
	mHeader->mTextFormat->SetValue(nID);
	EView old_view = mParsing;
	EView new_view = static_cast<EView>(nID);
	OnViewAs(new_view);
	
	// Check for failure to change and reset popup to old value
	if (mParsing == old_view)
		mHeader->mTextFormat->SetValue(mParsing);
	
	// Turn off quote depth if not formatted mode
	mHeader->mQuoteDepthPopup->SetActive(JBoolean(mParsing == eViewFormatted));
}

#if NOTYET
// Copy the message's addresses to an address book
void CMessageWindow::OnAddressCopy(UINT nID)
{
	// Find address book for id
	CAddressBookDoc* adbk_doc = CAddressBookDoc::sAddressBooks[nID - IDM_AddressCopyStart];
	
	// Find focussed field and copy corresponding addresses
	CWnd* focus = GetFocus();
	CAddressList* list = new CAddressList;
	CAddressList* copy = NULL;
	if (focus == &mHeader->mFromField)
		copy = mItsMsg->GetEnvelope()->GetFrom();
	else if (focus == &mHeader->mToField)
		copy = mItsMsg->GetEnvelope()->GetTo();
	else if (focus == &mHeader->mCCField)
		copy = mItsMsg->GetEnvelope()->GetCC();

	// Copy all addresses
	if (copy)
		for(CAddressList::const_iterator iter = copy->begin(); iter != copy->end(); iter++)
			list->push_back(new CAdbkAddress(**iter));
	
	// Add to address book
	adbk_doc->GetAddressBookWindow()->AddEntryLists(list, NULL);
}
#endif

void CMessageWindow::OnMessageFlagsSeen()
{
	OnMessageFlags(NMessage::eSeen);
}

void CMessageWindow::OnMessageFlagsAnswered()
{
	OnMessageFlags(NMessage::eAnswered);
}

void CMessageWindow::OnMessageFlagsImportant()
{
	OnMessageFlags(NMessage::eFlagged);
}

void CMessageWindow::OnMessageFlagsDraft()
{
	OnMessageFlags(NMessage::eDraft);
}

void CMessageWindow::OnMessageFlagsLabel(JIndex nID)
{
	OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << nID));
}

void CMessageWindow::OnMessageShowHeader(void)
{
	// Toggle header display
	mShowHeader = mHeader->mHeaderBtn->IsChecked();

	// Reset this message
	ResetText();
}

void CMessageWindow::OnMessageFromTwister(void)
{
	mHeaderState.mFromExpanded = mHeader->mFromTwister->IsChecked();
	DoTwist(mHeader->mFromField, mHeader->mFromTwister, mHeader->mFromMove);
}

void CMessageWindow::OnMessageToTwister(void)
{
	mHeaderState.mToExpanded = mHeader->mToTwister->IsChecked();
	DoTwist(mHeader->mToField, mHeader->mToTwister, mHeader->mToMove);
}

void CMessageWindow::OnMessageCCTwister(void)
{
	mHeaderState.mCcExpanded = mHeader->mCCTwister->IsChecked();
	DoTwist(mHeader->mCCField, mHeader->mCCTwister, mHeader->mCCMove);
}

// Handle twist
void CMessageWindow::DoTwist(CTextBase* aField,
			     CTwister* aTwister,
			     JXDecorRect* aMove)
{
	JCoordinate	moveby;

	if (aTwister->IsChecked())
	{
		moveby = cTwistMove;
		aField->Focus();
	}
	else
	{
		moveby = -cTwistMove;
		aField->Focus();
	}

	// Change edit field
	aMove->AdjustSize(0, moveby);

	// Do header layout after change
	LayoutHeader();
}

// Expand to full size header
void CMessageWindow::OnWindowsExpandHeader()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;

	LayoutHeader();
	
	// Reset text as message header summary display will also change
	ResetText();
}

// Layout header component based on state
void CMessageWindow::LayoutHeader()
{
	// Start with some initial offset
	int top = 8;
	const int left = 0;

	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Check for From field
		if (mHeaderState.mFromVisible)
		{
			// Get current size
			int height = mHeader->mFromMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mFromMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mFromMove->Show();
		}
		else
			// Always hide it
			mHeader->mFromMove->Hide();

		// Check for To field
		if (mHeaderState.mToVisible)
		{
			// Get current size
			int height = mHeader->mToMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mToMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mToMove->Show();
		}
		else
			// Always hide it
			mHeader->mToMove->Hide();

		// Check for Cc field
		if (mHeaderState.mCcVisible)
		{
			// Get current size
			int height = mHeader->mCCMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mCCMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mCCMove->Show();
		}
		else
			// Always hide it
			mHeader->mCCMove->Hide();

		// Check for Subject field
		if (mHeaderState.mSubjectVisible)
		{
			// Get current size
			int height = mHeader->mSubjectDateMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mSubjectDateMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mSubjectDateMove->Show();
		}
		else
			// Always hide it
			mHeader->mSubjectDateMove->Hide();
	}
	else
	{
		// Hide all addresses and subject
		mHeader->mFromMove->Hide();
		mHeader->mToMove->Hide();
		mHeader->mCCMove->Hide();
		mHeader->mSubjectDateMove->Hide();

		// Make sure we have something to show the message count
		top = 12;
	}
	
	// Get current size
	int height = mHeader->mPartsHeader->GetFrameHeight();

	// Move to current top and increment top by height
	mHeader->mPartsHeader->Place(0, top);
	top += height;

	// Get enclosure area
	JRect rect = mView->GetAperture();

	// Set current size of top header area
	mHeader->SetSize(rect.width(), top);

	// Move splitter to fill remainder of view space
	mSplitterView->Place(0, top);
	mSplitterView->SetSize(rect.width(), rect.height() - top);
	
	GetWindow()->Refresh();
}

void CMessageWindow::OnMessagePartsTwister(void)
{
	bool expanding = mHeader->mPartsTwister->IsChecked();
	JCoordinate moveby = mPartsScroller->GetFrameHeight();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kFixedTop);

		// Hide the parts table pane in the splitter
		mSplitterView->ShowView(false, true);

		// Show/hide items
		mHeader->mFlatHierarchyBtn->Hide();
		mHeader->mPartsField->Show();

		// Always focus on text
		mText->Focus();
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitterView->ShowView(true, true);

		// Turn on resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);
		
		// Show/hide items
		mHeader->mFlatHierarchyBtn->Show();
		mHeader->mPartsField->Hide();
		
		// Always focus on parts
		mPartsTable->Focus();
	}

	GetWindow()->Refresh();
}

void CMessageWindow::OnMessagePartsFlat()
{
	// Toggle flat state
	DoPartsFlat(!mPartsTable->GetFlat());
}

// Flatten parts table
void CMessageWindow::DoPartsFlat(bool flat)
{
	mPartsTable->SetFlat(flat);
	UpdatePartsCaption();
}

// Show/hide parts area
void CMessageWindow::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	int moveby = mSecurePane->GetFrameHeight();

	if (show)
	{
		// Shrink/move text pane
		mText->GetScroller()->AdjustSize(0, -moveby);
		mText->GetScroller()->Move(0, moveby);

		// Show parts after all other changes
		mSecurePane->Show();
	}
	else
	{
		// Hide parts before other changes
		mSecurePane->Hide();

		// Expand/move splitter
		mText->GetScroller()->AdjustSize(0, moveby);
		mText->GetScroller()->Move(0, -moveby);
	}

	mShowSecure = show;
}

// Reset state from prefs
void CMessageWindow::ResetState(bool force)
{
	// Check for available state
	CMessageWindowState& state = CPreferences::sPrefs->mMessageWindowDefault.Value();

	// Get visible state
	bool visible = GetWindow()->IsVisible();

	// Do not set if empty
	JRect set_rect = state.GetBestRect(CPreferences::sPrefs->mMessageWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Force twist down if required
	mHeader->mPartsTwister->SetState(state.GetPartsTwisted() ? kTrue : kFalse);
	mHeaderState.mExpanded = !state.GetCollapsed();
	mHeader->mFlatHierarchyBtn->SetState(state.GetFlat() ? kFalse : kTrue);

	// Force layout
	LayoutHeader();
	mSplitterView->SetPixelSplitPos(state.GetSplitChange());

	// Do zoom
	//if (state.GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	GetWindow()->Refresh();
}

// Save current state as default
void CMessageWindow::SaveDefaultState(void)
{
	// Get bounds
	JRect bounds;
	bounds = GetWindow()->GlobalToRoot(GetWindow()->GetFrameGlobal());
	bool zoomed = false;

	// Add info to prefs
	CMessageWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mSplitterView->GetPixelSplitPos(),
								mHeader->mPartsTwister->IsChecked(), !mHeaderState.mExpanded, mPartsTable->GetFlat(), false);
	if (CPreferences::sPrefs->mMessageWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMessageWindowDefault.SetDirty();

}


void CMessageWindow::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mHeader->mFromTwister)
		{
			OnMessageFromTwister();
			return;
		}
		else if (sender == mHeader->mToTwister)
		{
			OnMessageToTwister();
			return;
		}
		else if (sender == mHeader->mCCTwister)
		{
			OnMessageCCTwister();
			return;
		}
		else if (sender == mHeader->mPartsTwister)
		{
			OnMessagePartsTwister();
			return;
		}
		else if (sender == mHeader->mFlatHierarchyBtn)
		{
			OnMessagePartsFlat();
			return;
		}
		else if (sender == mHeader->mHeaderBtn)
		{
			OnMessageShowHeader();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mHeader->mFontIncreaseBtn)
		{
			OnIncreaseFont();
			return;
		}		
		else if (sender == mHeader->mFontDecreaseBtn)
		{
			OnDecreaseFont();
			return;
		}		
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mHeader->mTextFormat)
		{
			OnTextFormatPopup(is->GetIndex());
			return;
		}
		else if (sender == mHeader->mQuoteDepthPopup)
		{
			OnQuoteDepth(static_cast<EQuoteDepth>(is->GetIndex()));
			return;
		}
	}
	super::Receive(sender, message);
}

// Handle key presses
bool CMessageWindow::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Get state of command modifiers (not shift)
	bool mods = modifiers.shift() || modifiers.control() || modifiers.alt() || modifiers.meta();

	unsigned long cmd = 0;
	switch (key)
	{
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
		cmd = CCommand::eMessagesReadNext;
		break;

	case 'p':
	case 'P':
		cmd = CCommand::eMessagesReadPrev;
		break;

	case 'h':
	case 'H':
		cmd = CCommand::eMessagesShowHeader;
		break;

	case 'v':
	case 'V':
		cmd = CCommand::eWindowsShowParts;
		break;

	default:;
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
	return CFileDocument::HandleChar(key, modifiers);
}

void CMessageWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eFileSave:
	case CCommand::eFileSaveAs:
	case CCommand::eToolbarFileSaveBtn:
	case CCommand::eMessagesReply:
	case CCommand::eMessagesReplyToSender:
	case CCommand::eMessagesReplyToFrom:
	case CCommand::eMessagesReplyToAll:
	case CCommand::eMessagesForward:
	case CCommand::eMessagesBounce:
	case CCommand::eToolbarMessagePrintBtn:
	case CCommand::eToolbarMessageReplyBtn:
	case CCommand::eToolbarMessageReplyOptionBtn:
	case CCommand::eToolbarMessageReplyAllBtn:
	case CCommand::eToolbarMessageForwardBtn:
	case CCommand::eToolbarMessageForwardOptionBtn:
	case CCommand::eToolbarMessageBounceBtn:
		//For now
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eEditCheckSpelling:
		if (CPluginManager::sPluginManager.HasSpelling())	
			OnUpdateAlways(cmdui);
		return;

	case CCommand::eMessagesReadPrev:
	case CCommand::eToolbarMessagePreviousBtn:
		OnUpdateMessageReadPrev(cmdui);
		return;

	case CCommand::eMessagesReadNext:
	case CCommand::eToolbarMessageNextBtn:
		OnUpdateMessageReadNext(cmdui);
		return;

	case CCommand::eMessagesDelAndReadNext:
	case CCommand::eToolbarMessageDeleteNextBtn:
		OnUpdateMessageDelete(cmdui);
		return;

	case CCommand::eMessagesCopyAndReadNext:
		OnUpdateMessageCopyNext(cmdui);
		return;

	case CCommand::eMessagesReject:
	case CCommand::eToolbarMessageRejectBtn:
		OnUpdateMessageReject(cmdui);
		return;

	case CCommand::eMessagesSendAgain:
	case CCommand::eToolbarMessageSendAgainBtn:
		OnUpdateMessageSendAgain(cmdui);
		return;

	case CCommand::eMessagesCopyTo:
	case CCommand::eToolbarMessageCopyBtn:
	case CCommand::eMessagesCopyNow:
		OnUpdateMessageCopy(cmdui);
		return;

	case CCommand::eMessagesViewParts:
		OnUpdateMessageViewCurrent(cmdui);
		return;

	case CCommand::eMessagesVerifyDecrypt:
		OnUpdateMessageVerifyDecrypt(cmdui);
		return;

	case CCommand::eMessagesDelete:
	case CCommand::eToolbarMessageDeleteBtn:
		OnUpdateMessageDelete(cmdui);
		return;

	case CCommand::eMessagesShowHeader:
		OnUpdateMessageShowHeader(cmdui);
		return;

	case CCommand::eMessagesFlags:
		OnUpdateMessageFlagsTop(cmdui);
		return;

	case CCommand::eToolbarMessageFlagsBtn:
		if (cmdui->mMenu != NULL)
		{
			switch(cmdui->mMenuIndex)
			{
			case 1:
				OnUpdateMessageFlagsSeen(cmdui);
				break;
			case 2:
				OnUpdateMessageFlagsImportant(cmdui);
				break;
			case 3:
				OnUpdateMessageFlagsAnswered(cmdui);
				break;
			case 4:
				OnUpdateMessageFlagsDeleted(cmdui);
				break;
			case 5:
				OnUpdateMessageFlagsDraft(cmdui);
				break;
			default:
				OnUpdateMessageFlags(cmdui, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (cmdui->mMenuIndex - 6)));
				cmdui->SetText(CPreferences::sPrefs->mLabels.GetValue()[cmdui->mMenuIndex - 6]->name);
				break;
			}
		}
		else
			OnUpdateMessageFlagsTop(cmdui);
		break;

	case CCommand::eFlagsSeen:
		OnUpdateMessageFlagsSeen(cmdui);
		return;

	case CCommand::eFlagsImportant:
		OnUpdateMessageFlagsImportant(cmdui);
		return;

	case CCommand::eFlagsAnswered:
		OnUpdateMessageFlagsAnswered(cmdui);
		return;

	case CCommand::eFlagsDeleted:
		OnUpdateMessageFlagsDeleted(cmdui);
		return;

	case CCommand::eFlagsDraft:
		OnUpdateMessageFlagsDraft(cmdui);
		return;

	case CCommand::eFlagsLabel1:
	case CCommand::eFlagsLabel2:
	case CCommand::eFlagsLabel3:
	case CCommand::eFlagsLabel4:
	case CCommand::eFlagsLabel5:
	case CCommand::eFlagsLabel6:
	case CCommand::eFlagsLabel7:
	case CCommand::eFlagsLabel8:
		OnUpdateMessageFlagsLabel(cmdui);
		break;

	case CCommand::eMessagesDecodeEmbedded:
	case CCommand::eDecodeBinHex:
	case CCommand::eDecodeUU:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eWindowsExpandHeader:
		OnUpdateMenuExpandHeader(cmdui);
		return;
	case CCommand::eToolbarWindowsExpandHeaderBtn:
		OnUpdateExpandHeader(cmdui);
		return;
	case CCommand::eWindowsShowParts:
		OnUpdateWindowsShowParts(cmdui);
		return;
	default:;
	}

	if (cmdui->mMenu)
	{
		// Process copy to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(cmdui->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(cmdui->mMenu) != NULL))
		{
			OnUpdateMessageCopy(cmdui);
			return;
		}
		else if (cmdui->mMenu == mHeader->mTextFormat)
		{
			OnUpdateAlways(cmdui);
			return;
		}
		else if (cmdui->mMenu == mHeader->mQuoteDepthPopup)
		{
			OnUpdateAlways(cmdui);
			return;
		}
	}

	super::UpdateCommand(cmd, cmdui);
}

bool CMessageWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnFileNewDraft();
		return true;

	case CCommand::eFileSave:
	case CCommand::eFileSaveAs:
	case CCommand::eToolbarFileSaveBtn:
		SaveInNewFile();
		return true;

	case CCommand::eToolbarMessagePrintBtn:
		// Convert to standard print command and pass-up to actual handler
		cmd = CCommand::eFilePrint;
		break;

	case CCommand::eEditCheckSpelling:
		OnEditSpellCheck();
		return true;

	case CCommand::eMessagesReadPrev:
	case CCommand::eToolbarMessagePreviousBtn:
		OnMessageReadPrev();
		return true;

	case CCommand::eMessagesReadNext:
	case CCommand::eToolbarMessageNextBtn:
		OnMessageReadNext();
		return true;

	case CCommand::eMessagesDelAndReadNext:
	case CCommand::eToolbarMessageDeleteNextBtn:
		OnMessageDeleteRead();
		return true;

	case CCommand::eMessagesCopyAndReadNext:
		OnMessageCopyRead();
		return true;

	case CCommand::eMessagesReply:
	case CCommand::eToolbarMessageReplyBtn:
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
	case CCommand::eToolbarMessageForwardBtn:
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

	case CCommand::eToolbarMessageCopyBtn:
	case CCommand::eToolbarMessageCopyPopup:
		// If there is a menu set the value from that now
		if (menu && mView->GetToolbar()->GetCopyBtn())
			mView->GetToolbar()->GetCopyBtn()->SetValue(menu->mIndex);
		OnMessageCopyBtn();
		return true;

	case CCommand::eMessagesCopyNow:
		OnMessageCopyBtn();
		return true;

	case CCommand::eCommandMessageCopy:
		OnMessageCopyCmd();
		return true;

	case CCommand::eCommandMessageMove:
		OnMessageMoveCmd();
		return true;

	case CCommand::eMessagesViewParts:
		OnMessageViewCurrent();
		return true;

	case CCommand::eMessagesVerifyDecrypt:
		OnMessageVerifyDecrypt();
		return true;

	case CCommand::eMessagesDelete:
	case CCommand::eToolbarMessageDeleteBtn:
	case CCommand::eFlagsDeleted:
		OnMessageDelete();
		return true;

	case CCommand::eMessagesShowHeader:
		mHeader->mHeaderBtn->ToggleState();
		return true;

	case CCommand::eFlagsSeen:
		OnMessageFlagsSeen();
		return true;

	case CCommand::eFlagsImportant:
		OnMessageFlagsImportant();
		return true;

	case CCommand::eFlagsAnswered:
		OnMessageFlagsAnswered();
		return true;

	case CCommand::eFlagsDraft:
		OnMessageFlagsDraft();
		return true;

	case CCommand::eFlagsLabel1:
	case CCommand::eFlagsLabel2:
	case CCommand::eFlagsLabel3:
	case CCommand::eFlagsLabel4:
	case CCommand::eFlagsLabel5:
	case CCommand::eFlagsLabel6:
	case CCommand::eFlagsLabel7:
	case CCommand::eFlagsLabel8:
		OnMessageFlagsLabel(cmd - CCommand::eFlagsLabel1);
		return true;

	case CCommand::eToolbarMessageFlagsBtn:
		switch(menu->mIndex)
		{
		case 1:
			OnMessageFlagsSeen();
			break;
		case 2:
			OnMessageFlagsImportant();
			break;
		case 3:
			OnMessageFlagsAnswered();
			break;
		case 4:
			OnMessageDelete();
			break;
		case 5:
			OnMessageFlagsDraft();
			break;
		default:
			OnMessageFlagsLabel(menu->mIndex - 7);
			break;
		}
		break;

	case CCommand::eDecodeBinHex:
		OnMessageDecodeBinHex();
		return true;

	case CCommand::eDecodeUU:
		OnMessageDecodeUU();
		return true;

	case CCommand::eWindowsExpandHeader:
	case CCommand::eToolbarWindowsExpandHeaderBtn:
		OnWindowsExpandHeader();
		return true;

	case CCommand::eWindowsShowParts:
		mHeader->mPartsTwister->ToggleState();
		return true;
	}

	if (menu)
	{
		// Process copy to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(menu->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(menu->mMenu) != NULL))
		{
			OnMessageCopy(menu->mMenu, menu->mIndex);
			return true;
		}
	}

	return super::ObeyCommand(cmd, menu);
}

CCommander* CMessageWindow::GetTarget()
{
	// Route through parts list or text display if active
	if (mHeader->mFromField->HasFocus())
		return mHeader->mFromField;
	else if (mHeader->mToField->HasFocus())
		return mHeader->mToField;
	else if (mHeader->mCCField->HasFocus())
		return mHeader->mCCField;
	else if (mHeader->mSubjectField->HasFocus())
		return mHeader->mSubjectField;
	else if (mPartsTable->HasFocus())
		return mPartsTable;
	else if (mText->HasFocus())
		return mText;

	return this;
}
