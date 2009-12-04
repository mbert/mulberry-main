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

#include "CAddressList.h"
#include "CAddressBookManager.h"
#include "CAdminLock.h"
#include "CCharsetManager.h"
#include "CConnectionManager.h"
#include "CFileTable.h"
#include "CCaptionParser.h"
#include "CClipboard.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CDataAttachment.h"
#include "CErrorHandler.h"
#include "CIdentityPopup.h"
#include "CIMAPCommon.h"
#include "CLetterHeaderView.h"
#include "CLetterTextEditView.h"
#include "CLetterView.h"
#include "CLocalCommon.h"
#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMenuBar.h"
#include "CMessage.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSaveDraftDialog.h"
#include "CServerWindow.h"
#include "CSimpleTitleTable.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSecurityPlugin.h"
#include "CSpellPlugin.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStringUtils.h"
#include "CStyleToolbar.h"
#include "CTableScrollbarSet.h"
#include "CTextEngine.h"
#include "CTextField.h"
#include "CToolbarView.h"
#include "CTwister.h"
#include "CEnrichedTransformer.h"
#include "CHTMLTransformer.h"
#include "CWindowsMenu.h"

#include "cdfstream.h"
#include "diriterator.h"

#include "CAddressDisplay.h"
#include "HButtonText.h"
#include "HResourceMap.h"

#include <JXButton.h>
#include <JXColormap.h>
#include <JXChooseSaveFile.h>
#include <jFStreamUtil.h>
#include <JXDisplay.h>
#include <JXFlatRect.h>
#include <JXImageButton.h>
#include <JXImageWidget.h>
#include <JXInputField.h>
#include <JXIntegerInput.h>
#include <JXMenuBar.h>
#include "JXMultiImageButton.h"
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextEditor.h>
#include <JXTextMenu.h>
#include <JXUpRect.h>
#include <JXWidgetSet.h>
#include <jXGlobals.h>
#include "CInputField.h"

#include <JSimpleProcess.h>

#include <algorithm>

const int cWindowWidth = 575;
const int cWindowHeight = 300;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cViewTop = cToolbarTop + cToolbarHeight;
const int cViewHeight = cWindowHeight - cViewTop;

const int cHeaderHeight = 156;
const int cHeaderCollapsedHeight = 64;
const int cSplitterTop = cHeaderHeight;
const int cSplitterHeight = cViewHeight - cHeaderHeight;
const int cPartsCollapsedHeight = 24;
const int cPartsMinimumHeight = 84;
const int cTextHeight = cViewHeight - cPartsCollapsedHeight;
const int cStyledToolbarHeight = 26;

const int cTwistMove = 32;
const int cPartsTwistMove = 76;

UInt32 CLetterWindow::mDraftNum;

/////////////////////////////////////////////////////////////////////////////
// CLetterWindow construction/destruction

cdmutexprotect<CLetterWindow::CLetterWindowList> CLetterWindow::sLetterWindows;
cdstring  CLetterWindow::sLastAppendTo;

// Default constructor
CLetterWindow::CLetterWindow(JXDirector* owner) :
	super(owner, "", kFalse, kTrue, ".mbd"), JXIdleTask(500)
{
	// Add to list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		sLetterWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this);

	// NULL out others
	mStyledVisible = true;
	mSaveToMailbox = false;
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
	mExternalEdit = false;
	mExternalEditProcess = NULL;
	mBounceHeader = NULL;
}

// Default destructor
CLetterWindow::~CLetterWindow()
{
	// Set status
	SetClosing();

	// Delete all parts
	delete mBody;

	// Delete any stored bounce header
	delete mBounceHeader;

	// Delete any message lists
	delete mMsgs;
	mMsgs = NULL;

	// Remove from list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		CLetterWindowList::iterator found = std::find(sLetterWindows->begin(), sLetterWindows->end(), this);
		if (found != sLetterWindows->end())
			sLetterWindows->erase(found);
	}
	CWindowsMenu::RemoveWindow(this);

	DeleteTemporary();

	// Set status
	SetClosed();
}

// Manually create document
CLetterWindow* CLetterWindow::ManualCreate()
{

	CLetterWindow* pWnd = new CLetterWindow(CMulberryApp::sApp);
	pWnd->OnCreate();

	// Reset to default size
	pWnd->ResetState(true);

	// Do after reset state, once its visible
	if (pWnd->mHeaderState.mExpanded)
		pWnd->mHeader->mToField->Focus();
	else
		pWnd->mText->Focus();

	return (CLetterWindow*) pWnd;
}

// Open from disk
bool CLetterWindow::OpenDraft()
{
	// Choose file first
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Draft to open:", NULL, &fname))
		// Open from file
		return CLetterWindow::OpenDraft(fname);
	else
		return false;
}

// Open from disk
bool CLetterWindow::OpenDraft(const char* fpath)
{
	// Create new window
	CLetterWindow* wnd = ManualCreate();

	if (wnd)
	{
		// Read in file
		cdifstream input(fpath);
		wnd->ReadTextFile(input);
		wnd->FileChanged(fpath, kTrue);
		return true;
	}
	else
		return false;
}

const int cTitleHeight = 16;

void CLetterWindow::OnCreate()
{
	JXWindow* window = new JXWindow(this, cWindowWidth, cWindowHeight, "Draft");
	SetWindow(window);
	JString name;
	NextDefaultDraftName(name);
	FileChanged(name, kFalse);
	DataModified();
	
	CFileDocument::OnCreate();

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Create server view
	mView = new CLetterView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cViewTop, cWindowWidth, cViewHeight);
	mView->OnCreate();

	// Create toolbars for a view we own
	mToolbarView->SetSibling(mView);
	mView->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Header view
	mHeader = 
		new CLetterHeaderView(mView, mainMenus[CMainMenu::eEdit],
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
	  CFileTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0, cTitleHeight, cWindowWidth, cSplitterHeight);

	mPartsTitles = new CSimpleTitleTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
														JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 550, cTitleHeight);

	mPartsTable->OnCreate();
	mPartsTitles->OnCreate();

	mPartsTitles->SyncTable(mPartsTable, true);
	mPartsTable->SetTitles(mPartsTitles);

	mPartsTitles->LoadTitles("UI::Titles::LetterParts", 6);
	mPartsTable->SetAttachmentsOnly(CPreferences::sPrefs->mDisplayAttachments.GetValue());
								 
	// Create dummy text part
	mBody = mCurrentPart = new CDataAttachment(NULL);
	mCurrentPart->SetContent(eContentText, CPreferences::sPrefs->compose_as.GetValue());
	mPartsTable->SetBody(mBody);
	mPartsTable->SetRowShow(mCurrentPart);
	UpdatePartsCaption();

    mBottomPane =
        new JXFlatRect(mSplitterView, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, cWindowWidth, cSplitterHeight);

    mToolbar =
        new CStyleToolbar(mBottomPane, JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, cWindowWidth, cStyledToolbarHeight);
    mToolbar->OnCreate(this);

	mTextDisplay = mText = new CLetterTextEditView(mBottomPane, JXWidget::kHElastic,  JXWidget::kVElastic,
										0, cStyledToolbarHeight, cWindowWidth, cSplitterHeight - cStyledToolbarHeight);
	mText->OnCreate();
	mText->SetLetterWindow(this);
	mText->SetToolbar(mToolbar);
	SetToolbar(CPreferences::sPrefs->compose_as.GetValue());

	// Set font info
	bool html = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML) || (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched);
#ifndef USE_FONTMAPPER
	mText->ResetFont(html ? CPreferences::sPrefs->mHTMLTextFontInfo.GetValue() : CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);
#else
	mText->ResetFont(html ?
							CPreferences::sPrefs->mHTMLFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits.traits :
							CPreferences::sPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits.traits, 0);
#endif

	// Set spaces per tab in text entry and do not allow select all when tabbing
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetWrap(CPreferences::sPrefs->wrap_length.GetValue());
	mText->SetHardWrap(!CPreferences::sPrefs->mWindowWrap.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetWrapAllowed(true);
	mText->SetFindAllowed(true);

	// Give default reply quotation to text display
	mText->SetQuotation(CPreferences::sPrefs->mReplyQuote.GetValue());

	// Hide the styled toolbar
	HideToolbar();

	// Install the splitter items
	mSplitterView->InstallViews(mPartsScroller, mBottomPane, true);
	mSplitterView->ShowView(false, true);
	mSplitterView->SetMinima(84, 64);
	mSplitterView->SetPixelSplitPos(84);
	mSplitterView->SetLockResize(true);
	
	//listen to widgets
	ListenTo(mHeader->mIdentityPopup);
	ListenTo(mHeader->mIdentityEditBtn);
	ListenTo(mHeader->mToTwister);
	ListenTo(mHeader->mCCTwister);
	ListenTo(mHeader->mBCCTwister);
	ListenTo(mHeader->mCopyTo);
	ListenTo(mHeader->mPartsTwister);
	
	// So we'll know when these change and can mark ourselves as dirty
	ListenTo(mHeader->mToField);
	ListenTo(mHeader->mCCField);
	ListenTo(mHeader->mBCCField);
	ListenTo(mHeader->mSubjectField);
	ListenTo(mText);

	// Tooltips
	mHeader->mToTwister->SetHint(stringFromResource(IDC_LETTERTOTWISTER));
	mHeader->mToField->SetHint(stringFromResource(IDC_LETTERTOFIELD));
	mHeader->mCCTwister->SetHint(stringFromResource(IDC_LETTERCCTWISTER));
	mHeader->mCCField->SetHint(stringFromResource(IDC_LETTERCCFIELD));
	mHeader->mBCCTwister->SetHint(stringFromResource(IDC_LETTERBCCTWISTER));
	mHeader->mBCCField->SetHint(stringFromResource(IDC_LETTERBCCFIELD));

	mHeader->mSubjectField->SetHint(stringFromResource(IDC_LETTERSUBJECTFIELD));

	mHeader->mCopyTo->SetHint(stringFromResource(IDC_LETTERCOPYTOPOPUP));

	mHeader->mSentIcon->SetHint(stringFromResource(IDC_LETTERSENTICON));

	mHeader->mPartsTwister->SetHint(stringFromResource(IDC_LETTERPARTSTWISTER));
	mHeader->mPartsField->SetHint(stringFromResource(IDC_LETTERPARTSFIELD));
	mHeader->mAttachments->SetHint(stringFromResource(IDC_LETTERPARTSICON));
	//mHeader->mIdentityPopup->SetHint(stringFromResource(IDC_LETTERIDENTITYPOPUP));

	// Do CC/Bcc show if required
	if (CPreferences::sPrefs->mShowCCs.GetValue())
		OnDraftAddCc();
	if (CPreferences::sPrefs->mShowBCCs.GetValue())
		OnDraftAddBcc();

	// Set to default identity
#if 0
	cdstring temp = sLastAppendTo;
	SetIdentity(NULL);
	sLastAppendTo = temp;

	// Set to cached copy to if prefs set
	if (CPreferences::sPrefs->remember_record.GetValue() && !sLastAppendTo.empty())
		mHeader->mCopyTo->SetSelectedMbox(sLastAppendTo, sLastAppendTo.empty(), (sLastAppendTo == "\1"));

	// Check for missing Copy To
	if (mHeader->mCopyTo->IsMissingMailbox())
		CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::MissingCopyTo", mHeader->mCopyTo.GetMissingName());
#endif

	// Make to address focus
	if (mHeaderState.mExpanded)
		mHeader->mToField->Focus();
	else
		mText->Focus();

	SetSaveNewFilePrompt("Save draft as:");

	// Set current width and height as minimum
	GetWindow()->SetWMClass(cLetterWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_LETTERDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	CreateMainMenu(window,
					 CMainMenu::fFile |
					 CMainMenu::fEdit |
					 CMainMenu::fDraft | 
					 CMainMenu::fAddresses |
					 CMainMenu::fCalendar | 
					 CMainMenu::fWindows | 
					 CMainMenu::fHelp);

	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());

	mAutoSaveTime = ::time(NULL);
	CMulberryApp::sApp->InstallIdleTask(this);

	// Set status
	SetOpen();
}

void CLetterWindow::Activate()
{
	CFileDocument::Activate();
	
	mView->RefreshToolbar();
	
}

void CLetterWindow::WindowFocussed(JBoolean focussed)
{
	if (focussed)
	{
		PauseAutoSaveTimer(false);
	}
	else
	{
		SaveTemporary();
		PauseAutoSaveTimer(true);
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________Auto-save

unsigned long	CLetterWindow::sTemporaryCount = 1;
const char* cTemporaryDraftName = "MulberryDraft";
const char* cTemporaryDraftExtension = ".mbd";
const char* cRecoveredDraftName = "Recovered Draft ";

void CLetterWindow::SaveTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	cdstring newTemporary;

	try
	{
		// Find temporary folder
		cdstring dir = CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory();

		// Find unused temp name
		while(true)
		{
			cdstring tempname(cTemporaryDraftName);
			tempname += cdstring(sTemporaryCount++);
			tempname += cTemporaryDraftExtension;

			newTemporary = dir;
			::addtopath(newTemporary, tempname);
			if (!::fileexists(newTemporary))
				break;
		}

		// Create new file
		WriteFile(newTemporary, kTrue);

		// Delete the existing file here
		DeleteTemporary();
		
		// Assign temporary file
		mTemporary = newTemporary;
		
		// Reset timer
		ResetAutoSaveTimer();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

unsigned long CLetterWindow::SaveAllTemporary()
{
	// Init counters
	unsigned long total = 0;

	// Protect against all exceptions
	try
	{
		// Now look for any drafts
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		for(CLetterWindowList::iterator iter = sLetterWindows->begin(); iter != sLetterWindows->end(); iter++)
		{
			// Adjust counters and attempt safety save
			(*iter)->SaveTemporary();
			total++;
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return total;
}

void CLetterWindow::DeleteTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	try
	{
		if (!mTemporary.empty() && ::fileexists(mTemporary))
			::remove_utf8(mTemporary);
		mTemporary.clear();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CLetterWindow::ReadTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	unsigned long ctr = 1;
	try
	{
		// Find temporary folder
		cdstring dir = CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory();

		diriterator diter(dir, cTemporaryDraftExtension);
		const char* fname = NULL;
		while(diter.next(&fname))
		{
			// Convert to full path and attempt open
			cdstring fullpath = dir;
			::addtopath(fullpath, fname);
			
			// Create new draft from the temp file
			MakeTemporary(fullpath, ctr++);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Not much we can do so do not throw up
	}
}

void CLetterWindow::MakeTemporary(const cdstring& path, unsigned long ctr)
{
	// Protect against all execptions so that failure to open this one
	// will not prevent others from opening
	try
	{
		// Create new window
		CLetterWindow* wnd = ManualCreate();

		if (wnd)
		{
			// Read in file
			cdifstream input(path);
			wnd->ReadTextFile(input);

			// Now make sure draft nows about the current temp file
			wnd->mTemporary = path;

			// Give window a suitable title
			cdstring tempname(cRecoveredDraftName);
			tempname += cdstring(ctr);
			wnd->FileChanged(tempname.c_str(), kFalse);
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CLetterWindow::ResetAutoSaveTimer()
{
	mAutoSaveTime = ::time(NULL);
}

void CLetterWindow::PauseAutoSaveTimer(bool pause)
{
	if (pause)
	{
		CMulberryApp::sApp->RemoveIdleTask(this);
	}
	else
		CMulberryApp::sApp->InstallIdleTask(this);
}

// Called during idle
void CLetterWindow::Perform(const Time delta, Time* maxSleepTime)
{
	*maxSleepTime = 500;
	Time bogus;
	if (!TimeToPerform(delta, &bogus))
		return;

	// See if we are equal or greater than trigger
	if (::time(NULL) >= mAutoSaveTime + CPreferences::sPrefs->mAutoSaveDraftsInterval.GetValue())
	{
		// Do auto save
		SaveTemporary();
	}
}

#pragma mark ____________________________Initialise

// Add address list to caption/field:
void CLetterWindow::SetTwistList(CAddressDisplay* aField,
								 CTwister* aTwister,
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
	aField->SetText(addrs);

	// Force twist down if one or more
	if ((aTwister->IsChecked() && !got_two) ||
		(!aTwister->IsChecked() && got_two))
	  aTwister->ToggleState();//will cause DoTwist to be called

}

// Add address lists
void CLetterWindow::SetAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc)
{

	// Add all tos
	if (to)
		SetTwistList(mHeader->mToField, mHeader->mToTwister, to);

	// Add all Ccs
	if (cc)
	{
		SetTwistList(mHeader->mCCField, mHeader->mCCTwister, cc);
		
		// Make sure Cc is visible
		if (!mHeaderState.mCcVisible && cc->size())
			OnDraftAddCc();
	}

	// Add all Bccs
	if (bcc)
	{
		SetTwistList(mHeader->mBCCField, mHeader->mBCCTwister, bcc);
		
		// Make sure Bcc is visible
		if (!mHeaderState.mBccVisible && bcc->size())
			OnDraftAddBcc();
	}

	// Set target to text if some items inserted into list
	if (mHeaderState.mExpanded)
	{
		if (to || cc || bcc)
			mHeader->mSubjectField->Focus();
		else
			mHeader->mToField->Focus();
	}
}


// Get address lists
void CLetterWindow::GetAddressLists(CAddressList*& to,
									CAddressList*& cc,
									CAddressList*& bcc)
{
	// Get to text
	to = mHeader->mToField->GetAddresses();

	// Get ccs
	cc = mHeader->mCCField->GetAddresses();

	// Get Bccs
	bcc = mHeader->mBCCField->GetAddresses();
}

void CLetterWindow::SetSubject(const cdstring& text)
{
	mHeader->mSubjectField->SetText(text);
	OnChangeSubject();
}

void CLetterWindow::GetSubject(cdstring& text) const
{
	text = mHeader->mSubjectField->GetText();
}

// Do common bits after setting a message
void CLetterWindow::PostSetMessage(const CIdentity* id)
{
	// Enable append replied if not sub-message (not for rejects)
	CMbox* found = NULL;
	if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce) &&
		mHeader->mCopyTo->GetSelectedMbox(found, false) &&
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

	// If message comes from INBOX and pref set check it as true
	// Do this after init'ing the identity
	if (mMsgs && mMsgs->size() && !mReject &&
		!::strcmpnocase(mMsgs->front()->GetMbox()->GetName(), cINBOX) &&
		CPreferences::sPrefs->inbox_append.GetValue())
	{
		mCopyOriginal = true;
	}

	// Need to reset focus
	if (mReply || mReject || mSendAgain || !mHeaderState.mExpanded)
	{
		// Set target to text
		mText->Focus();
	}
	else
	{
		// Set target to to address
		mHeader->mToField->Focus();
	}

	// Disable certain controls for rejects
	if (mReject)
	{
		// Disable all edit fields
		mHeader->mToField->Deactivate();
		mHeader->mCCField->Deactivate();
		mHeader->mBCCField->Deactivate();
		mHeader->mSubjectField->Deactivate();

		// Disable copy to controls
		mHeader->mCopyTo->Deactivate();
		mCopyOriginalAllowed = false;
	}

	// Finally force redraw
	UpdatePartsCaption();

	// Make sure window items are not dirty
	SetDirty(false);
	
	// Force main toolbar to update
	mView->RefreshToolbar();
	
	// Make sure its visible and redraw
	Show();
	GetWindow()->Refresh();
	
	// Do address capture if needed
	if ((mReply || mForward) && mMsgs && CPreferences::sPrefs->mCaptureRespond.GetValue())
		CAddressBookManager::sAddressBookManager->CaptureAddress(*mMsgs);
	
	// Do auto external edit if required
	if (CPreferences::sPrefs->mUseExternalEditor.GetValue() &&
		CPreferences::sPrefs->mExternalEditAutoStart.GetValue())
		OnExternalEdit();
}

#pragma mark ____________________________Sending

// Copy to mailbox now
void CLetterWindow::OnDraftAppendNow()
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();
	CopyNow(option_key);
}

void CLetterWindow::OnDraftAppendNowBtn()
{
	CopyNow(false);
}

void CLetterWindow::OnDraftAppendNowOptionBtn()
{
	CopyNow(true);
}

// Copy to mailbox now
void CLetterWindow::OnDraftAppend(JXTextMenu* menu, JIndex nID)
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();

	// Get mailbox to append to
	CMbox* append_mbox = NULL;
	CCopyToMenu::GetMbox(menu, false, nID, append_mbox);
	CopyNow(append_mbox, option_key);
}

#pragma mark ____________________________Quoting

// Include message selection indented
void CLetterWindow::IncludeMessage(CMessage* theMsg, bool forward, int start, int length, bool header)
{
	// Only do if text available
	if (!theMsg->HasText())
		return;

	// Reset undo action in edit control
	mText->ClearUndo();
	
	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return;

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
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CLetterWindow::IncludeMessageTxt(CMessage* theMsg,
										const char* msg_hdr,
										const char* msg_txt,
										bool forward,
										EContentSubType type,
										bool is_flowed)
{
	JIndex selStart, selEnd;
	mText->GetSelectionRange(selStart, selEnd);
	cdstring theTxt;
	
	// Force insert of CRLF if at very top of message if cursor will be placed at top
	if ((selStart == 0) &&
		((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
		 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue())))
	{
		cdstring convertTxt = os_endl2;
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
		default:
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
			default:
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
			}
		}
	}

	// Start styled quotation
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	default:
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
	}

	// Include the message's header as addition if required
	if (msg_hdr)
	{
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
		default:
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
	default:
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
		default:
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
		}
		break;
	}
	case eContentSubHTML:
	{
		switch(type)
		{
		case eContentSubPlain:
		default:
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
		}
		break;
	}
	}

	// Stop styled quotation before line end
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	default:
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
	}

	// Finish with LFs if not already
	{
		cdstring convertTxt;
		if (theTxt[theTxt.length() - 1] != '\n')
			convertTxt += os_endl;
		convertTxt += os_endl;

		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
		default:
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
			default:
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
			}
		}
	}

	SetToolbar(CPreferences::sPrefs->compose_as.GetValue());
	{
		// Convert to utf16 and insert
		cdustring text_utf16(theTxt);
		mText->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);
	}

	SyncPart();
	
	if ((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
		 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue()))
		mText->SetSelectionRange(selStart, selStart);
	
	// Force caltext, adjust image & refresh
	mText->Reset();

	// Force dirty off
	DataReverted();
}


// Include the replied to mail
void CLetterWindow::IncludeFromReply(bool forward, bool header)
{
	// Beep if not replying
	if (!mMsgs)
	{
		::MessageBeep(1);
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
				IncludeMessage(*riter, forward, 0, 0, header);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			// Protect against mMsgs changing whilst doing ops
			size_t old_size = mMsgs->size();
			for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
				IncludeMessage(*iter, forward, 0, 0, header);
		}
		
		// Force dirty as IncludeMessage clears dirty flag
		DataModified();
	}
	
	// Reset focus to text
	mText->Focus();
}

// Include some text indented
void CLetterWindow::IncludeText(const char* theText, bool forward, bool adding)
{
	cdstring theTxt;

	if (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain)
	{
		// Start with endl if not adding
		if (!adding)
			theTxt += os_endl;

		std::auto_ptr<const char> quoted(QuoteText(theText, forward));
		theTxt += quoted.get();

		// Finish with endl
		theTxt += os_endl;
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched)
	{
		// Start with <br> if not adding
		theTxt += "<excerpt>\n";
		theTxt += theText;
		theTxt += "</excerpt>";

		// Finish with endls
		theTxt += os_endl2;
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML)
	{
		// Start with <br> if not adding
		theTxt += "<blockquote>\n";
		theTxt += theText;
		theTxt += "\n</blockquote>\n";

		// Finish with endl
		theTxt += "<br>\n";
	}

	cdustring text_utf16(theTxt);
	mText->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);
}

#pragma mark ____________________________Parts

// Update attachments caption
void CLetterWindow::UpdatePartsCaption()
{
	unsigned long count = mPartsTable->CountParts();
	char buf[32];
	::snprintf(buf, 32, "%ld", count);
	mHeader->mPartsField->SetText(buf);

	mHeader->mAttachments->SetVisible(mPartsTable->HasAttachments() ? kTrue : kFalse);

	mHeader->mPartsTitle->SetVisible(!mPartsTable->GetAttachmentsOnly() ? kTrue : kFalse);
	mHeader->mAttachmentTitle->SetVisible(mPartsTable->GetAttachmentsOnly() ? kTrue : kFalse);
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
	}	
	else
		SetCurrentPart(part);
}

void CLetterWindow::SetText(const char *text, bool plain)
{
//	mText->SetWindowText(text);

	{
		const char* data = NULL;
		SetToolbar(mBody->GetContent());
		bool html = ((mBody->GetContent().GetContentSubtype() == eContentSubEnriched) ||
					 (mBody->GetContent().GetContentSubtype() == eContentSubHTML)) &&
								CAdminLock::sAdminLock.mAllowStyledComposition;
#ifndef USE_FONTMAPPER
				mText->ResetFont(html ? CPreferences::sPrefs->mHTMLTextFontInfo.GetValue() : CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);
#else
				mText->ResetFont(html ?
										CPreferences::sPrefs->mHTMLFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits :
										CPreferences::sPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, 0);
#endif

		if (text && *text)
		{
			mText->Reset();
			{
				cdustring text_utf16(text);
				mText->ParseBody(text_utf16, plain ? eContentSubPlain : mBody->GetContent().GetContentSubtype(), eViewFormatted, -1);
			}
			mText->InsertFormatted(html ? eViewFormatted : eViewPlain);
		}
		else
			mText->SetText("");

		mText->SetSelectionRange(0, 0);
	}
	mText->Refresh();
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
			mCurrentPart = part;
			const char* data = mCurrentPart->GetData();
			SetToolbar(mCurrentPart->GetContent());
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
				mText->Reset();
				bool html = ((mime.GetContentSubtype() == eContentSubEnriched) || (mime.GetContentSubtype() == eContentSubHTML)) &&
								CAdminLock::sAdminLock.mAllowStyledComposition;
				mText->ResetFont(html ? CPreferences::sPrefs->mHTMLTextFontInfo.GetValue() : CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);
				mText->ParseBody(data_utf16, mCurrentPart->GetContent().GetContentSubtype(), eViewFormatted, -1);
				mText->InsertFormatted(html ? eViewFormatted : eViewPlain);
			}
			else
			{
				mText->SetText(data);
			}

			mText->SetSelectionRange(0, 0);

			// Don't allow edit if reject
			mText->SetReadOnly(mReject);
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

	mText->Refresh();
}

// Sync text in current part
void CLetterWindow::SyncPart()
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		char *data = NULL;
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubEnriched:
			{
				CEnrichedTransformer trans(mText);
				data = trans.Transform();
			}
			break;
		case eContentSubHTML:
			{
				CHTMLTransformer trans(mText);
				data = trans.Transform();
			}
			break;
		default:
			{
				cdstring temp;
				mText->GetText(temp);
				data = temp.release();
			}
			break;
		}

		mCurrentPart->GetContent().SetCharset(i18n::eUTF8);
		mCurrentPart->SetData(data);
	}
}

// Sync text size in current part
void CLetterWindow::SyncPartSize()
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		int length = mText->GetTextLength();
		mCurrentPart->GetContent().SetContentSize(length);
	}
}

void CLetterWindow::SetToolbar(const CMIMEContent& mime)
{
	if (mime.GetContentType() == eContentText)
		SetToolbar(mime.GetContentSubtype());
}

void CLetterWindow::SetToolbar(EContentSubType type)
{
	JIndex index;
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
}


void CLetterWindow::ShowToolbar(EContentSubType type, bool override)
{
	if (!mStyledVisible || override)
	{
		mText->GetScroller()->Move(0, cStyledToolbarHeight);
		mText->GetScroller()->AdjustSize(0, -cStyledToolbarHeight);

		mToolbar->Show();
		mToolbar->Setup(type);
		mStyledVisible = true;
		mText->ShowToolbar(true);

		// Change style of text viewer to accept styled D&D / Copy&Paste
		mText->AllowStyles(true);
	}
	else
	{
		mToolbar->Setup(type);
	}
}

void CLetterWindow::HideToolbar(bool override)
{
	if (mStyledVisible || override)
	{
		mToolbar->Hide();
		mStyledVisible = false;

		mText->GetScroller()->Move(0, -cStyledToolbarHeight);
		mText->GetScroller()->AdjustSize(0, cStyledToolbarHeight);
		mText->ShowToolbar(false);

		// Change style of text viewer to reject styled D&D / Copy&Paste
		mText->AllowStyles(false);
	}
}

// Handle style text popup command
void CLetterWindow::OnDraftCurrentText(unsigned long cmd)
{
	EContentSubType subtype;

	switch(cmd)
	{
	case CCommand::eCurrentTextPlain:
	case CCommand::eCurrentTextPlainWrapped:
		subtype = eContentSubPlain;
		break;
	case CCommand::eCurrentTextEnriched:
		subtype = eContentSubEnriched;
		break;
	case CCommand::eCurrentTextHTML:
		subtype = eContentSubHTML;
		break;
	}

	if (mCurrentPart && (mCurrentPart->GetContent().GetContentType() == eContentText))
	{
		// Do warning if changing from styled to unstyled
		const CMIMEContent& mime = mCurrentPart->GetContent();
		if ((mime.GetContentSubtype() != eContentSubPlain) && (subtype == eContentSubPlain))
		{
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::WarnLoseFormatting") == CErrorHandler::Cancel)
			{
				return;
			}
		}

		// Make sure wrapping correctly set before doing content change
		switch(cmd)
		{
		case CCommand::eCurrentTextPlain:
		case CCommand::eCurrentTextEnriched:
		case CCommand::eCurrentTextHTML:
			mText->SetHardWrap(false);
			break;
		case CCommand::eCurrentTextPlainWrapped:
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
	mHeader->mIdentityPopup->SetValue(mHeader->mIdentityPopup->FirstIndex() + index);
}

// Handle identities popup command
void CLetterWindow::OnChangeIdentity(JIndex nID)
{
	switch(nID)
	{
	// New identity wanted
	case eIdentityPopup_New:
		mHeader->mIdentityPopup->DoNewIdentity(CPreferences::sPrefs);
		break;
	
	// Edit identity
	case eIdentityPopup_Edit:
		mHeader->mIdentityPopup->DoEditIdentity(CPreferences::sPrefs);
		break;
	
	// Delete existing identity
	case eIdentityPopup_Delete:
		mHeader->mIdentityPopup->DoDeleteIdentity(CPreferences::sPrefs);
		break;
	
	// Set custom identity
	case eIdentityPopup_Custom:
		SetIdentity(mHeader->mIdentityPopup->GetCustomIdentity(), true);
		break;
	
	// Select an identity
	default:
		{
			const CIdentity& identity = CPreferences::sPrefs->mIdentities.GetValue()[nID - mHeader->mIdentityPopup->FirstIndex()];
			SetIdentity(&identity);
		}
		break;
	}
	
	// Check value and show/hide custom edit item
	mHeader->mIdentityEditBtn->SetVisible((mHeader->mIdentityPopup->GetValue() == eIdentityPopup_Custom) ? kTrue : kFalse);
}

// Edit custom identity
void CLetterWindow::OnEditCustom()
{
	if (mHeader->mIdentityPopup->EditCustomIdentity())
		SetIdentity(mHeader->mIdentityPopup->GetCustomIdentity(), true);
}

// Change identity
void CLetterWindow::SetIdentity(const CIdentity* identity, bool custom, bool add_signature)
{
	// Preserve focus
	JXWidget* focus;
	GetWindow()->GetFocusWidget(&focus);
 
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
	mHeader->mCopyTo->SetSelectedMbox(identity->GetCopyTo(true),
										identity->GetCopyToNone(true),
										identity->GetCopyToChoose(true));

	// Do Also Copy
	mCopyOriginal = identity->GetCopyReplied(true);

	// Do signature insert
	if (CPreferences::sPrefs->mAutoInsertSignature.GetValue() && !mReject)
	{
		// Preserve selection
		CTextBase::StPreserveSelection selection(mText);

		// Replace signature
		JSize textLen = mText->GetTextLength();
		mText->SetSelectionRange(textLen - mSignatureLength, textLen);

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

	// Restore focus
	if (focus)
		focus->Focus();

	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Insert signature
void CLetterWindow::OnDraftInsertSignature()
{
	InsertSignature(GetIdentity()->GetSignature(true));
}

// Insert signature
void CLetterWindow::InsertSignature(const cdstring& signature)
{
	// Cannot do this in read-only text
	if (mText->IsReadOnly())
		return;

	// Get pos of insert cursor
	JIndex selStart, selEnd;
	mText->GetSelectionRange(selStart, selEnd);

	// Look for preceeding LFs
	cdstring sel_text;
	bool keepChecking = true;
	{
		mText->SetSelectionRange((selStart > 2 ? selStart - 2 : 0), selStart);
		mText->GetSelectedText(sel_text);
		mText->SetSelectionRange(selStart, selEnd);
	}

	bool char1 = false;
	bool char2 = false;
	if (keepChecking)
	{
		if (selStart > 2)
		{
			char1 = (sel_text[1UL] == '\n');
			char2 = (sel_text[0UL] == '\n');
		}
		else if (selStart > 1)
			char1 = (sel_text[0UL] == '\n');
	}

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
	mText->Refresh();
}

#pragma mark ____________________________External Edit

void CLetterWindow::OnExternalEdit()
{
	// Only allow this once
	if (mExternalEdit)
		return;

	// Create temp file name
	mExternalEditFile = CConnectionManager::sConnectionManager.GetExternalEditDraftDirectory();
	::TempFileSpec(mExternalEditFile, "MulberryExternalEdit.txt", false, false);

	// Create temp file to edit
	FILE* fp = ::fopen_utf8(mExternalEditFile, "a+");
	if (!fp)
		return;
	
	// Write current text data to file
	int tlength = mText->GetTextLength();
	cdstring txt = mText->GetText();
	::fwrite(txt.c_str(), 1, txt.length(), fp);
	::fclose(fp);

	// Get editor string
	cdstring editor = CPreferences::sPrefs->mExternalEditor.GetValue();
	if (editor == "%s")
	{
		char* editor_env = ::getenv("EDITOR");
		if (editor_env && *editor_env)
		{
			editor = editor_env;
			editor += " %s";
		}
		else
		{
			CErrorHandler::PutStopAlert(IDE_LETTER_MISSING_EDITOR);
			return;
		}
	}

	// Form command line, quoting path
	cdstring cmd;
	::InsertPathIntoStringWithQuotes(cmd, CPreferences::sPrefs->mExternalEditor.GetValue(), mExternalEditFile);

	// Create a process to edit a temp file
	mExternalEditProcess = NULL;
	const JError err = JSimpleProcess::Create(&mExternalEditProcess, cmd, kTrue);
	if (err.OK())
	{
		mExternalEdit = true;
		ListenTo(mExternalEditProcess);
		
		mPartsTable->Deactivate();
		mText->Deactivate();
		RefreshToolbar();
	}
	else
	{
		mExternalEditProcess = NULL;
		CErrorHandler::PutStopAlertStr(IDE_LETTER_FAILED_EDITOR, cmd);
	}
}

void CLetterWindow::OnExternalEditEnd()
{
	// Read in temp file
	FILE* fp = ::fopen_utf8(mExternalEditFile, "r");
	if (!fp)
		return;
	
	cdstring buf;
	buf.reserve(4096);
	size_t read = 0;
	bool added = false;
	while((read = ::fread(buf.c_str_mod(), 1, 4095, fp)) != 0)
	{
		buf[read] = 0;
		if (!added)
		{
			mText->SetText(buf);
			JSize textLen = mText->GetTextLength();
			mText->SetSelectionRange(textLen, textLen);
			added = true;
		}
		else
			mText->InsertUTF8(buf);
	}
	::fclose(fp);
	::remove_utf8(mExternalEditFile);
	mExternalEditFile = cdstring::null_str;
	
	mExternalEditProcess = NULL;
	mExternalEdit = false;

	mText->SetSelectionRange(0, 0);

	mPartsTable->Activate();
	mText->Activate();
	RefreshToolbar();
}

#pragma mark ____________________________Window changes

// Reset toolbar commands
void CLetterWindow::RefreshToolbar()
{
	// Force main toolbar to update
	mView->RefreshToolbar();
}

// Reset fonts
void CLetterWindow::ResetFont(const SFontInfo& list_font, const SFontInfo& display_font)
{
	mPartsTitles->ResetFont(list_font);
	mPartsTable->ResetFont(list_font);
	mPartsTitles->SyncTable(mPartsTable, true);
	mText->ResetFont(display_font, 0);
}

// Temporarily add header summary for printing
void CLetterWindow::AddPrintSummary(const CMessage* msg)
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue())
	{
		// Get summary from envelope
		std::ostrstream hdr;
		msg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		cdstring header_insert;
		header_insert.steal(hdr.str());

		// Parse as header
		mText->InsertFormattedHeader(header_insert.c_str());
	}
}

// Remove temp header summary after printing
void CLetterWindow::RemovePrintSummary(const CMessage* msg)
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue())
	{
		// Get summary from envelope
		std::ostrstream hdr;
		msg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		cdstring header_insert;
		header_insert.steal(hdr.str());

		// Parse as header
		mText->SetSelectionRange(0, header_insert.length());
		mText->InsertUTF8(cdstring::null_str);
	}
}

// Check dirty state
bool CLetterWindow::IsDirty()
{
#if NOTYET
	bool dirty = false;
	dirty = dirty | mHeader->mToField.GetModify();
	dirty = dirty | mHeader->mCCField.GetModify();
	dirty = dirty | mHeader->mBCCField.GetModify();
	dirty = dirty | mHeader->mSubjectField.GetModify();
	dirty = dirty | mPartsTable.IsDirty();
	dirty = dirty | mText->GetModify();
	return dirty;
#endif	
	return NeedsSave();
}

//	Set dirty state
void CLetterWindow::SetDirty(bool dirty)
{
#if NOTYET
	mHeader->mToField.SetModify(dirty);
	mHeader->mCCField.SetModify(dirty);
	mHeader->mBCCField.SetModify(dirty);
	mHeader->mSubjectField.SetModify(dirty);
	mPartsTable.SetDirty(dirty);
	mText->SetModify(dirty);
#endif
	if (dirty) {
		DataModified();
	} else {
		DataReverted();
	}
}

void CLetterWindow::ReadTextFile(std::istream& input)
{
	// Get tos
	cdstring txt;
	::getline(input, txt);
	mHeader->mToField->SetText(txt.c_str() + ::strlen(cHDR_TO));

	// Get ccs
	::getline(input, txt);
	mHeader->mCCField->SetText(txt.c_str() + ::strlen(cHDR_CC));
	
	// Make sure Cc is visible
	if (!txt.empty() && !mHeaderState.mCcVisible)
		OnDraftAddCc();

	// Get bccs
	::getline(input, txt);
	mHeader->mBCCField->SetText(txt.c_str() + ::strlen(cHDR_BCC));

	// Make sure Bcc is visible
	if (!txt.empty() && !mHeaderState.mBccVisible)
		OnDraftAddBcc();

	// Get subject
	::getline(input, txt);
	mHeader->mSubjectField->SetText(txt.c_str() + ::strlen(cHDR_SUBJECT));

	// Check for files or content
	::getline(input, txt);
	
	if (::strncmp(txt, cHDR_COPYTO, sizeof(cHDR_COPYTO) - 1) == 0)
	{
		cdstring copyto = &((const char*) txt)[sizeof(cHDR_COPYTO) - 1];
		mHeader->mCopyTo->SetSelectedMbox(copyto, false, copyto.length() == 0);

		// Read next line
		::getline(input, txt);
	}

	if (::strncmp(txt, cHDR_MIME_TYPE, sizeof(cHDR_MIME_TYPE) - 1) == 0)
	{
		mBody->GetContent().SetContent(txt.c_str() + sizeof(cHDR_MIME_TYPE) - 1);
		//mText->SetToolbar(mBody->GetContent());

		// Get files
		::getline(input, txt);
		::getline(input, txt);
	}
	else
	{
		// Get files
		::getline(input, txt);
	}

	// Get main text one line at a time
	{
		cdstring txt_total;
		while(!::getline(input, txt).fail())
		{
			txt_total += txt;
			txt_total += os_endl;
		}

		SetText(txt_total);
	}

	ResetState(true);
}

// Save data to file
void CLetterWindow::WriteTextFile(std::ostream& output, const JBoolean safetySave) const
{
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	cdstring	subjectText;
	cdstring	mailText;
	
	try
	{
		// Get to text
		cdstring addrs;
		addrs = mHeader->mToField->GetText();
		int text_length = addrs.length();
		to_list = new CAddressList(addrs, text_length);
		
		// Write to archive
		output << cHDR_TO;
		bool first = true;
		for(CAddressList::const_iterator iter = to_list->begin(); iter != to_list->end(); iter++)
		{
			cdstring str = (*iter)->GetFullAddress();
			if (!first)
				output << ", ";
			else
				first = false;
			output << str;
		}
		output << os_endl;
		
		// Get cc text
		addrs = mHeader->mCCField->GetText();
		text_length =  addrs.length();
		cc_list =  new CAddressList(addrs, text_length);

		// Write to archive
		output << cHDR_CC;
		first = true;
		for(CAddressList::const_iterator iter =  cc_list->begin(); iter !=  cc_list->end(); iter++)
		{
			cdstring str =  (*iter)->GetFullAddress();
			if (!first)
				output << ", ";
			else
				first =  false;
			output << str;
		}
		output << os_endl;

		// Get bcc text
		addrs = mHeader->mBCCField->GetText();
		text_length =  addrs.length();
		bcc_list =  new CAddressList(addrs, text_length);

		// Write to archive
		output << cHDR_BCC;
		first =  true;
		for(CAddressList::const_iterator iter =  bcc_list->begin(); iter !=  bcc_list->end(); iter++)
		{
			cdstring str =  (*iter)->GetFullAddress();
			if (!first)
				output << ", ";
			else
				first =  false;
			output << str;
		}
		output << os_endl;

		// Get subject
		subjectText = mHeader->mSubjectField->GetText();
		output << cHDR_SUBJECT;
		output << subjectText;
		output << os_endl;

		// Get copy to
		cdstring copy_to;
		mHeader->mCopyTo->GetSelectedMboxName(copy_to, false);
		output << cHDR_COPYTO;
		output << copy_to;
		output << os_endl;

		// Get content type/subtype
		output << cHDR_MIME_TYPE;
		if (mCurrentPart)
		{
			cdstring content =  CMIMESupport::GenerateContentHeader(mCurrentPart, false, lendl, false);
			output << content;
		}
		output << os_endl;

		// Get files
		output << cHDR_FILES;
		output << os_endl;
		output << os_endl;

		// Get mail text & length
		if (mCurrentPart)
		{
			const_cast<CLetterWindow*>(this)->SyncPart();
			mailText =  mCurrentPart->GetData();
			output << mailText;
		}
		output << os_endl;

		delete to_list;
		delete cc_list;
		delete bcc_list;
	}
	catch(...)
	{
		// Clean up and throw up
		delete to_list;
		delete cc_list;
		delete bcc_list;
		
		CLOG_LOGRETHROW;
		throw;
	}
	// Always clean
}

void CLetterWindow::SafetySave(const JXDocumentManager::SafetySaveReason reason)
{
	// Do not do abort safety save here as our own CSafetySave will handle it
	if ((reason != JXDocumentManager::kServerDead) &&
		(reason != JXDocumentManager::kKillSignal) &&
		(reason != JXDocumentManager::kAssertFired))
		CFileDocument::SafetySave(reason);
}

bool CLetterWindow::AbortSafetySave(const char* path)
{
	bool result = false;
	try
	{
		WriteFile(path, kTrue);
		result = true;
	}
	catch(...)
	{
		// Do not log this as it happens during abort and the logs may not be valid
		// but drafts are and we don't want failed logging to stop safety saves
	}
	
	return result;
}

#pragma mark ____________________________________Command Updaters

void CLetterWindow::OnUpdateNotReject(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject && !mExternalEdit);	// Not for rejects
}

void CLetterWindow::OnUpdateFileImport(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(mText->HasFocus() && !mReject && !mExternalEdit);

	cdstring txt;
	txt.FromResource(IDS_LETTER_IMPORTTEXT);
	pCmdUI->SetText(txt);
}

void CLetterWindow::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	mUndoer.FindUndoStatus(pCmdUI->mMenu, pCmdUI->mMenuIndex);
}

void CLetterWindow::OnUpdateAllowStyled(CCmdUI* pCmdUI)
{
	// Check for allowed and not reject
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowStyledComposition && !mReject && !mExternalEdit);
}

void CLetterWindow::OnUpdatePasteIndented(CCmdUI* pCmdUI)
{
	// Only if text display active
	JXWidget* focussed = NULL;
	if (GetWindow()->GetFocusWidget(&focussed) && (focussed == mText))
	{
		// JX always enables the Paste command irrecpective of whether there's any
		// text in the clipboard. We might as well do the same!
		pCmdUI->Enable(true);
	}
}

void CLetterWindow::OnUpdateDraftInclude(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(mMsgs && mMsgs->size() && mReply && !mExternalEdit);
}

void CLetterWindow::OnUpdateDraftAddCc(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(!mHeaderState.mCcVisible);
}

void CLetterWindow::OnUpdateDraftAddBcc(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(!mHeaderState.mBccVisible);
}

void CLetterWindow::OnUpdateDraftNotReject(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(!mReject);
}

void CLetterWindow::OnUpdateTextStyle(CCmdUI* pCmdUI)
{
	// Only if allowed by admin and not rejecting
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowStyledComposition && !mReject &&
							(mCurrentPart != NULL) && (mCurrentPart->GetContent().GetContentType() == eContentText));

	bool marked = false;
	switch(pCmdUI->mCmd)
	{
	case CCommand::eCurrentTextPlain:
		marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && !mText->GetHardWrap();
		break;
	case CCommand::eCurrentTextPlainWrapped:
		marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && mText->GetHardWrap();
		break;
	case CCommand::eCurrentTextEnriched:
		marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched);
		break;
	case CCommand::eCurrentTextHTML:
		marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML);
		break;
	}
	pCmdUI->SetCheck(marked);
}

void CLetterWindow::OnUpdateDraftAttachFile(CCmdUI* pCmdUI)
{
	// Only if not locked out and not reject
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoAttachments && !mReject && !mExternalEdit);
}

void CLetterWindow::OnUpdateDraftSendMail(CCmdUI* pCmdUI)
{
	// Only if in SMTP sender is availble
	const CIdentity* id = GetIdentity();
	pCmdUI->Enable(!mExternalEdit &&
					(CConnectionManager::sConnectionManager.IsConnected() ||
						CSMTPAccountManager::sSMTPAccountManager &&
						CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected(*id)));
}

void CLetterWindow::OnUpdateDraftAppend(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	CCopyToMenu::ResetMenuList(NULL, pCmdUI->mMenu);

	// Do update if not reject
	OnUpdateNotReject(pCmdUI);

	// Make sure we are listening to submenus of main append to
	const JXMenu* submenu = NULL;
	if (pCmdUI->mMenu && pCmdUI->mMenu->GetSubmenu(pCmdUI->mMenuIndex, &submenu) &&
		(dynamic_cast<const CCopyToMenu::CCopyToSub*>(submenu) != NULL))
		ListenTo(submenu);
}

void CLetterWindow::OnUpdateDraftCopyOriginal(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mCopyOriginalAllowed);
	pCmdUI->SetCheck(mCopyOriginal);
}

void CLetterWindow::OnUpdateDraftSign(CCmdUI* pCmdUI)
{
	// Only if plugin and not reject
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity() && !mReject);
	pCmdUI->SetCheck(mDoSign);
}

void CLetterWindow::OnUpdateDraftEncrypt(CCmdUI* pCmdUI)
{
	// Only if plugin and not reject
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity() && !mReject);
	pCmdUI->SetCheck(mDoEncrypt);
}

void CLetterWindow::OnUpdateDraftExternalEdit(CCmdUI* pCmdUI)
{
	// Only if plugin and not reject
	pCmdUI->Enable(!mReject && !mExternalEdit);
}

void CLetterWindow::OnUpdateDraftMDN(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetMDN());
}

void CLetterWindow::OnUpdateDraftDSNSuccess(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetSuccess());
}

void CLetterWindow::OnUpdateDraftDSNFailure(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetFailure());
}

void CLetterWindow::OnUpdateDraftDSNDelay(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetDelay());
}

void CLetterWindow::OnUpdateDraftDSNFull(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetFull());
}

void CLetterWindow::OnUpdateMenuExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mHeaderState.mExpanded ? IDS_COLLAPSEHEADER : IDS_EXPANDHEADER);
	
	pCmdUI->SetText(txt);
}

void CLetterWindow::OnUpdateExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
}

void CLetterWindow::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(!mHeader->mPartsTwister->IsChecked() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	pCmdUI->SetText(txt);
}

void CLetterWindow::OnUpdateAdminLockDraftSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalDrafts && CanRevert());
}

#pragma mark ____________________________________Command Handlers

void CLetterWindow::OnFileNewDraft()
{
	// Pass up to app
	CMulberryApp::sApp->OnAppNewDraft();
}

void CLetterWindow::OnFileSave()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		SaveInCurrentFile();
		break;

	case eSaveDraftToMailbox:
		OnFileSaveAs();
		break;

	case eSaveDraftChoose:
		{
			CSaveDraftDialog::SSaveDraft details;
			if (CSaveDraftDialog::PoseDialog(details, !CAdminLock::sAdminLock.mNoLocalDrafts))
			{
				mSaveToMailbox = !details.mFile;
				mMailboxSave = details.mMailboxName;
				if (mSaveToMailbox)
					DoMailboxSave();
				else
					SaveInCurrentFile();
			}
		}
	}
}

void CLetterWindow::OnFileSaveAs()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		SaveInNewFile();
		break;

	case eSaveDraftToMailbox:
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		break;

	case eSaveDraftChoose:
		{
			CSaveDraftDialog::SSaveDraft details;
			if (CSaveDraftDialog::PoseDialog(details, !CAdminLock::sAdminLock.mNoLocalDrafts))
			{
				mSaveToMailbox = !details.mFile;
				mMailboxSave = details.mMailboxName;
				if (mSaveToMailbox)
					DoMailboxSave();
				else
					SaveInNewFile();
			}
		}
	}
}

bool CLetterWindow::DoFileSave()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		SaveInCurrentFile();
		return true;

	case eSaveDraftToMailbox:
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		return true;

	case eSaveDraftChoose:
		if (mSaveToMailbox)
		{
			DoMailboxSave();
			return true;
		}
		else
		{
			SaveInCurrentFile();
			return true;
		}
	}
}

// Save to a mailbox
void CLetterWindow::DoMailboxSave()
{
	CMbox* mbox = NULL;

	// Must have a mailbox name
	if (mMailboxSave.empty())
	{
	}	
	// Resolve mailbox name
	else if (mMailboxSave == "\1")
	{
		// Fake mailbox popup choice to do browse dialog - always return if cancelled
		const int cPopupChoose = 1;
		if (!CCopyToMenu::GetPopupMbox(mHeader->mCopyTo, false, cPopupChoose, mbox, true))
			return;
	}
	else
	{
		// Resolve mailbox name
		mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(mMailboxSave);		
	}

	// Force mailbox choice if mailbox is missing
	if (!mbox)
	{
		// Fake mailbox popup choice to do browse dialog - return if cancelled
		const int cPopupChoose = 1;
		if (!CCopyToMenu::GetPopupMbox(mHeader->mCopyTo, false, cPopupChoose, mbox, true))
			return;
	}

	// Do copy, always as draft
	CopyNow(mbox, !CPreferences::sPrefs->mAppendDraft.GetValue());
}

// Command handlers
void CLetterWindow::OnFileImport()
{
	// Do standard open dialog
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Text file to Import:", NULL, &fname))
	{
		// Reset undo action in edit control
		mText->ClearUndo();
		
		try
		{
			// Create file object
			ifstream import(fname);

			JString data;
			JReadFile(import, &data);
			
			// Do line end translation
			std::ostrstream out;
			const char* p = data.GetCString();
			bool got_cr = false;
			while(*p)
			{
				switch(*p)
				{
				case '\r':
					got_cr = true;
					out << os_endl;
					p++;
					break;
				case '\n':
					if (got_cr)
						got_cr = false;
					else
						out << os_endl;
					p++;
					break;
				default:
					got_cr = false;
					out.put(*p++);
					break;
				}
			}
			out << std::ends;

			const char* converted = out.str();
			out.freeze(false);

			// Replace current selection with new text and decrement count
			mText->InsertUTF8(converted);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
	}
}

void CLetterWindow::OnEditUndo()
{
	mUndoer.ToggleAction();
}

void CLetterWindow::OnPasteIndented()
{
	// Get text from secondary clipboard
	cdstring text;
	bool shift_key = GetDisplay()->GetLatestKeyModifiers().shift();
	if (shift_key)
		CClipboard::GetPrimaryClipboard(GetDisplay(), text);
	else
		CClipboard::GetSecondaryClipboard(GetDisplay(), text);

	// Now add text
	if (text.length())
		IncludeText(text, false);
}

void CLetterWindow::OnEditSpellCheck()
{
	DoSpellCheck(false);
}

void CLetterWindow::OnDraftAttachFile()
{
	mPartsTable->OnDraftAttachFile();
	UpdatePartsCaption();
}

// Include the replied to mail
void CLetterWindow::OnDraftInclude()
{
	IncludeFromReply(false, false);
}

void CLetterWindow::OnDraftNewPlainPart()
{
	mPartsTable->OnDraftNewPlainPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftNewEnrichedPart()
{
	mPartsTable->OnDraftNewEnrichedPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftNewHTMLPart()
{
	mPartsTable->OnDraftNewHTMLPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartMixed()
{
	mPartsTable->OnDraftMultipartMixed();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartParallel()
{
	mPartsTable->OnDraftMultipartParallel();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartDigest()
{
	mPartsTable->OnDraftMultipartDigest();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartAlternative()
{
	mPartsTable->OnDraftMultipartAlternative();
	UpdatePartsCaption();
}

// Expand/collapse to full size header
void CLetterWindow::OnLetterHeaderTwister()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;

	// Update recipient text if that is being shown
	if (!mHeaderState.mExpanded)
		UpdateRecipients();

	LayoutHeader();
}

// Toggle statevoid CLetterWindow::OnLetterToTwister()
void CLetterWindow::OnLetterToTwister()
{
	mHeaderState.mToExpanded = mHeader->mToTwister->IsChecked();
	DoTwist(mHeader->mToField, mHeader->mToTwister, mHeader->mToMove);
}

void CLetterWindow::OnLetterCCTwister()
{
	mHeaderState.mCcExpanded = mHeader->mCCTwister->IsChecked();
	DoTwist(mHeader->mCCField, mHeader->mCCTwister, mHeader->mCCMove);
}

void CLetterWindow::OnLetterBCCTwister()
{
	mHeaderState.mBccExpanded = mHeader->mBCCTwister->IsChecked();
	DoTwist(mHeader->mBCCField, mHeader->mBCCTwister, mHeader->mBCCMove);
}

// Handle twist
void CLetterWindow::DoTwist(CAddressDisplay* aField,
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

// Add Cc button clicked
void CLetterWindow::OnDraftAddCc()
{
	// Make the Cc header visible
	mHeaderState.mCcVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();
	
	// Focus on the CC field
	mHeader->mCCField->Focus();

	// Force main toolbar to update
	RefreshToolbar();
}

// Add Bcc button clicked
void CLetterWindow::OnDraftAddBcc()
{
	// Make the Cc header visible
	mHeaderState.mBccVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();
	
	// Focus on the BCC field
	mHeader->mBCCField->Focus();

	// Force main toolbar to update
	RefreshToolbar();
}

// Layout header component based on state
void CLetterWindow::LayoutHeader()
{
	// Start with some initial offset
	int top = 0;
	const int left = 0;
	
	// Get the from area position and size which is always visible
	// Get current position
	JCoordinate hheight = mHeader->mHeaderMove->GetFrameHeight();

	// Move current top to bottom of from area
	top += hheight;

	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Hide recipients
		mHeader->mRecipientMove->Hide();
		
		// Show from (if more than one identity)
		// NB We need to determine numner of identities from the identity popup, because during a preference
		// change, the sPrefs may actually point to the old set, not the new one we need for this
		if (mHeader->mIdentityPopup->GetCount() > 1)
		{
			mHeader->mFromMove->Show();
		}
		else
		{
			mHeader->mFromMove->Hide();
			top -= hheight;
		}

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

		// Check for CC field
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

		// Check for Bcc field
		if (mHeaderState.mBccVisible)
		{
			// Get current size
			int height = mHeader->mBCCMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mBCCMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mBCCMove->Show();
		}
		else
			// Always hide it
			mHeader->mBCCMove->Hide();

		// Check for Subject field
		if (mHeaderState.mSubjectVisible)
		{
			// Get current size
			int height = mHeader->mSubjectMove->GetFrameHeight();

			// Move to current top and increment top by height
			mHeader->mSubjectMove->Place(left, top);
			top += height;

			// Always show it
			mHeader->mSubjectMove->Show();
		}
		else
			// Always hide it
			mHeader->mSubjectMove->Hide();
	}
	else
	{
		// Hide from show recipients
		mHeader->mFromMove->Hide();
		mHeader->mRecipientMove->Show();

		// Hide all addresses and subject
		mHeader->mToMove->Hide();
		mHeader->mCCMove->Hide();
		mHeader->mBCCMove->Hide();
		mHeader->mSubjectMove->Hide();
	}
	
	// Get current size
	int height = mHeader->mPartsMove->GetFrameHeight();

	// Move to current top and increment top by height
	mHeader->mPartsMove->Place(0, top);
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
	
	// Do recipient text
	mHeader->mRecipientText->SetText(txt);

}

// Resort addresses
void CLetterWindow::OnLetterCopyToPopup(JIndex nID)
{
	mHeader->mCopyTo->GetSelectedMboxName(sLastAppendTo, false);

	// Disable buttons
	CMbox* mbox =  NULL;
	mHeader->mCopyTo->GetSelectedMbox(mbox, false);
	if (mbox ==  (CMbox*) -1)
	{
		mCopyOriginalAllowed = false;
	}
	else
	{
		// Stop appending replied to same mailbox
		if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce))
		{
			if ((mMsgs->front()->GetMbox() ==  mbox) || mMsgs->front()->IsSubMessage())
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
}

void CLetterWindow::OnLetterPartsTwister()
{
	// Handle parts twist and focus
	DoPartsTwist(mHeader->mPartsTwister->IsChecked(), true);
}

// Is parts exposed
bool CLetterWindow::IsPartsTwist() const
{
	return mHeader->mPartsTwister->IsChecked();
}

// Handle twist of attachments
void CLetterWindow::DoPartsTwist(bool expand, bool change_focus)
{
	// Determine motion size and hide/show
	if (!expand)
	{
		// Turn off resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kFixedTop);

		// Hide the parts table pane in the splitter
		mSplitterView->ShowView(false, true);

		// Focus on text if required
		if (change_focus)
			mText->Focus();
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitterView->ShowView(true, true);

		// Turn on resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);
		
		// Focus on parts if required
		if (change_focus)
			mPartsTable->Focus();
	}
	// Always make sure twister is in sync as this can be called
	// without the twister being clicked
	if (mHeader->mPartsTwister->IsChecked() ^ expand)
	{
		SetListening(kFalse);
		mHeader->mPartsTwister->SetState(expand ? kTrue : kFalse);
		SetListening(kTrue);
	}

	GetWindow()->Refresh();
}

void CLetterWindow::OnChangeSubject()
{
	// Only if not specified
	if (IsFirstSave())
	{
		// Cache original doc title in case subject is deleted
		if (mOriginalTitle.empty())
			mOriginalTitle =  GetWindowTitle();

		cdstring subjectText;
		subjectText = mHeader->mSubjectField->GetText();

		// Check for empty subject and use original title
		if (subjectText.empty())
			subjectText =  mOriginalTitle;


		// Set the file name to use when saving
		// Make it sage for unix
		::strreplace(subjectText.c_str_mod(), "/", '_');
		FileChanged(subjectText.c_str(), kFalse);

		// Update window menu state to ensure it is refreshed
		CWindowsMenu::RenamedWindow();
	}
}

CMailboxPopup* CLetterWindow::GetAppendPopup()
{
	return mHeader->mCopyTo;
}

CFileTable* CLetterWindow::GetPartsTable()
{
	return mPartsTable;
}

void CLetterWindow::ShowSentIcon()
{
	mHeader->mSentIcon->Show();
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

// Reset state from prefs
void CLetterWindow::ResetState(bool reset)
{
	// Check for available state
	CLetterWindowState& state =  CPreferences::sPrefs->mLetterWindowDefault.Value();

	// Do not set if empty
	JRect set_rect =  state.GetBestRect(CPreferences::sPrefs->mLetterWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset bounds
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Force twist down if required
	mHeader->mPartsTwister->SetState(state.GetPartsTwisted() ? kTrue : kFalse);
	mHeaderState.mExpanded = !state.GetCollapsed();

	// Force layout
	LayoutHeader();
	mSplitterView->SetPixelSplitPos(state.GetSplitChange());

	// Do zoom
	//if (state.GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	//Always activate
	Activate();
}

// Save current state as default
void CLetterWindow::SaveDefaultState()
{
	// Get bounds
	JRect bounds;
	bounds = GetWindow()->GlobalToRoot(GetWindow()->GetFrameGlobal());
	bool zoomed = false;

	// Add info to prefs
	CLetterWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mSplitterView->GetPixelSplitPos(), mHeader->mPartsTwister->IsChecked(), !mHeaderState.mExpanded);
	if (CPreferences::sPrefs->mLetterWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mLetterWindowDefault.SetDirty();

}

void CLetterWindow::OnUpdateStyle(JXWidget* pWidget)
{
#if NOTYET
	mText->GetToolbar()->OnUpdateStyle(pCmdUI);
#endif
}

void CLetterWindow::OnUpdateAlignment(JXWidget* pWidget)
{
#if NOTYET
	mText->GetToolbar()->OnUpdateAlignment(pCmdUI);
#endif
}

void CLetterWindow::OnUpdateFont(JXWidget* pWidget)
{
#if NOTYET
	mText->GetToolbar()->OnUpdateFont(pCmdUI);
#endif
}

void CLetterWindow::OnUpdateFontSize(JXWidget* pWidget)
{
#if NOTYET
	mText->GetToolbar()->OnUpdateSize(pCmdUI);
#endif
}

void CLetterWindow::OnUpdateColor(JXWidget* pWidget)
{
#if NOTYET
	mText->GetToolbar()->OnUpdateColor(pCmdUI);
#endif
}

void CLetterWindow::OnStyle(JIndex nID)
{
#if NOTYET
	mText->GetToolbar()->OnStyle(nID);
#endif
}

void CLetterWindow::OnAlignment(JIndex nID)
{
#if NOTYET
	mText->GetToolbar()->OnAlignment(nID);
#endif
}

void CLetterWindow::OnFont(JIndex nID)
{
#if NOTYET
	mText->GetToolbar()->OnFont(nID);
#endif
}

void CLetterWindow::OnFontSize(JIndex nID)
{
#if NOTYET
	mText->GetToolbar()->OnFontSize(nID);
#endif
}

void CLetterWindow::OnColor(JIndex nID)
{
#if NOTYET
	mText->GetToolbar()->OnColor(nID);
#endif
}

void CLetterWindow::Receive (JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JTextEditor16::kTextChanged))
	{
		//If we get this message then one of our text fields has changed, so
		//mark ourselves dirty
		SetDirty(true);

		// Change to subject
		if (sender == mHeader->mSubjectField)
			OnChangeSubject();
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mHeader->mToTwister)
		{
			OnLetterToTwister();
			return;
		}
		else if (sender == mHeader->mCCTwister)
		{
			OnLetterCCTwister();
			return;
		}
		else if (sender == mHeader->mBCCTwister)
		{
			OnLetterBCCTwister();
			return;
	    }
		else if (sender == mHeader->mPartsTwister)
		{
			OnLetterPartsTwister();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mHeader->mIdentityEditBtn)
		{
			OnEditCustom();
			return;
		}
	} 
	else if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		if (sender == mHeader->mCopyTo)
		{
			OnLetterCopyToPopup(index);
			return;
		}
		else if (sender == mHeader->mIdentityPopup)
		{
			OnChangeIdentity(index);
			return;
		}
	}
	else if (message.Is(JProcess::kFinished) && (sender == mExternalEditProcess))
		OnExternalEditEnd();

	super::Receive(sender, message);
}

void CLetterWindow::NextDefaultDraftName(JString& mDraftName)
{
	char buf[11];
	mDraftName = "Draft";
	mDraftName += JString(mDraftNum++/1.0); 
}

bool CLetterWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnFileImport();
		return true;

	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		OnFileSave();
		return true;

	case CCommand::eFileSaveAs:
		OnFileSaveAs();
		return true;

	case CCommand::eEditUndo:
		OnEditUndo();
		return true;

	case CCommand::eEditPasteAsQuote:
		OnPasteIndented();
		return true;

	case CCommand::eEditCheckSpelling:
	case CCommand::eToolbarLetterSpellBtn:
		OnEditSpellCheck();
		return true;

	case CCommand::eDraftIncludeOrig:
		OnDraftInclude();
		return true;

	case CCommand::eDraftInsertSig:
	case CCommand::eToolbarLetterSignatureBtn:
		OnDraftInsertSignature();
		return true;

	case CCommand::eDraftAddCc:
	case CCommand::eToolbarLetterAddCcBtn:
		OnDraftAddCc();
		return true;

	case CCommand::eDraftAddBcc:
	case CCommand::eToolbarLetterAddBccBtn:
		OnDraftAddBcc();
		return true;

	case CCommand::eDraftAttachFile:
	case CCommand::eToolbarLetterAttachBtn:
		// Only if not locked out
		if (!CAdminLock::sAdminLock.mNoAttachments)
			OnDraftAttachFile();
		return true;

	case CCommand::eDraftSend:
	case CCommand::eToolbarLetterSendBtn:
		OnDraftSendMail();
		return true;

	case CCommand::eToolbarLetterExternalBtn:
		OnExternalEdit();
		return true;

	case CCommand::eDraftAppendNow:
	case CCommand::eToolbarLetterAppendNowBtn:
		OnDraftAppendNow();
		return true;

	case CCommand::eDraftCopyOriginal:
	case CCommand::eToolbarLetterCopyOriginalBtn:
		OnDraftCopyOriginal();
		return true;

	case CCommand::eDraftSign:
	case CCommand::eToolbarLetterSignBtn:
		OnDraftSign();
		return true;

	case CCommand::eDraftEncrypt:
	case CCommand::eToolbarLetterEncryptBtn:
		OnDraftEncrypt();
		return true;

	case CCommand::eNewTextPlain:
		OnDraftNewPlainPart();
		return true;

	case CCommand::eNewTextEnriched:
		OnDraftNewEnrichedPart();
		return true;

	case CCommand::eNewTextHTML:
		OnDraftNewHTMLPart();
		return true;

	case CCommand::eNewMultiMixed:
		OnDraftMultipartMixed();
		return true;

	case CCommand::eNewMultiParallel:
		OnDraftMultipartParallel();
		return true;

	case CCommand::eNewMultiDigest:
		OnDraftMultipartDigest();
		return true;

	case CCommand::eNewMultiAlt:
		OnDraftMultipartAlternative();
		return true;

	case CCommand::eCurrentTextPlain:
	case CCommand::eCurrentTextPlainWrapped:
	case CCommand::eCurrentTextEnriched:
	case CCommand::eCurrentTextHTML:
		OnDraftCurrentText(cmd);
		return true;

	case CCommand::eNotifyReceipt:
	case CCommand::eToolbarLetterReceiptBtn:
		OnDraftMDN();
		return true;

	case CCommand::eNotifySuccess:
	case CCommand::eToolbarLetterDSNBtn:
		OnDraftDSNSuccess();
		return true;

	case CCommand::eNotifyFailure:
		OnDraftDSNFailure();
		return true;

	case CCommand::eNotifyDelay:
		OnDraftDSNDelay();
		return true;

	case CCommand::eNotifyEntireMsg:
		OnDraftDSNFull();
		return true;

	case CCommand::eWindowsExpandHeader:
	case CCommand::eToolbarWindowsExpandHeaderBtn:
		OnLetterHeaderTwister();
		return true;

	case CCommand::eWindowsShowParts:
		mHeader->mPartsTwister->ToggleState();
		return true;
	}

	if (menu)
	{
		// Process append to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(menu->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(menu->mMenu) != NULL))
		{
			OnDraftAppend(menu->mMenu, menu->mIndex);
			return true;
		}
	}

	return super::ObeyCommand(cmd, menu);
}

void CLetterWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnUpdateFileImport(cmdui);
		return;

	case CCommand::eFileSave:
	case CCommand::eFileSaveAs:
	case CCommand::eToolbarFileSaveBtn:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eFileRevert:
		OnUpdateAdminLockDraftSave(cmdui);
		return;

	case CCommand::eEditUndo:
		OnUpdateEditUndo(cmdui);
		return;

	case CCommand::eEditPasteAsQuote:
		OnUpdatePasteIndented(cmdui);
		return;

	case CCommand::eEditCheckSpelling:
	case CCommand::eToolbarLetterSpellBtn:
		if (mCurrentPart && CPluginManager::sPluginManager.HasSpelling() && !mExternalEdit)
			OnUpdateAlways(cmdui);
		return;

	case CCommand::eDraftIncludeOrig:
		OnUpdateDraftInclude(cmdui);
		return;

	case CCommand::eDraftInsertSig:
	case CCommand::eToolbarLetterSignatureBtn:
		OnUpdateNotReject(cmdui);
		return;

	case CCommand::eDraftAddCc:
	case CCommand::eToolbarLetterAddCcBtn:
		OnUpdateDraftAddCc(cmdui);
		return;

	case CCommand::eDraftAddBcc:
	case CCommand::eToolbarLetterAddBccBtn:
		OnUpdateDraftAddBcc(cmdui);
		return;

	case CCommand::eDraftAttachFile:
	case CCommand::eToolbarLetterAttachBtn:
		OnUpdateDraftAttachFile(cmdui);
		return;

	case CCommand::eDraftSend:
	case CCommand::eToolbarLetterSendBtn:
		OnUpdateDraftSendMail(cmdui);
		return;

	case CCommand::eToolbarLetterExternalBtn:
		OnUpdateDraftExternalEdit(cmdui);
		return;

	case CCommand::eDraftAppendTo:
	case CCommand::eDraftAppendNow:
	case CCommand::eToolbarLetterAppendNowBtn:
		OnUpdateNotReject(cmdui);
		return;

	case CCommand::eDraftCopyOriginal:
	case CCommand::eToolbarLetterCopyOriginalBtn:
		OnUpdateDraftCopyOriginal(cmdui);
		return;

	case CCommand::eDraftSign:
	case CCommand::eToolbarLetterSignBtn:
		OnUpdateDraftSign(cmdui);
		return;

	case CCommand::eDraftEncrypt:
	case CCommand::eToolbarLetterEncryptBtn:
		OnUpdateDraftEncrypt(cmdui);
		return;

	case CCommand::eDraftNewTextPart:
	case CCommand::eNewTextPlain:
		OnUpdateNotReject(cmdui);
		return;

	case CCommand::eNewTextEnriched:
	case CCommand::eNewTextHTML:
		OnUpdateAllowStyled(cmdui);
		return;

	case CCommand::eDraftNewMultipart:
	case CCommand::eNewMultiMixed:
	case CCommand::eNewMultiParallel:
	case CCommand::eNewMultiDigest:
	case CCommand::eNewMultiAlt:
		OnUpdateNotReject(cmdui);
		return;

	case CCommand::eDraftCurrentTextPart:
	case CCommand::eCurrentTextPlain:
	case CCommand::eCurrentTextPlainWrapped:
	case CCommand::eCurrentTextEnriched:
	case CCommand::eCurrentTextHTML:
		OnUpdateTextStyle(cmdui);
		return;

	case CCommand::eDraftNotifications:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eNotifyReceipt:
	case CCommand::eToolbarLetterReceiptBtn:
		OnUpdateDraftMDN(cmdui);
		return;

	case CCommand::eNotifySuccess:
	case CCommand::eToolbarLetterDSNBtn:
		OnUpdateDraftDSNSuccess(cmdui);
		return;

	case CCommand::eNotifyFailure:
		OnUpdateDraftDSNFailure(cmdui);
		return;

	case CCommand::eNotifyDelay:
		OnUpdateDraftDSNDelay(cmdui);
		return;

	case CCommand::eNotifyEntireMsg:
		OnUpdateDraftDSNFull(cmdui);
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
		// Process append to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(cmdui->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(cmdui->mMenu) != NULL))
		{
			OnUpdateDraftAppend(cmdui);
			return;
		}
	}

	super::UpdateCommand(cmd, cmdui);
}

CCommander* CLetterWindow::GetTarget()
{
	// Route through parts list or text display if active
	if (mHeader->mToField->HasFocus())
		return mHeader->mToField;
	else if (mHeader->mCCField->HasFocus())
		return mHeader->mCCField;
	else if (mHeader->mBCCField->HasFocus())
		return mHeader->mBCCField;
	else if (mHeader->mSubjectField->HasFocus())
		return mHeader->mSubjectField;
	else if (mPartsTable->HasFocus())
		return mPartsTable;
	else if (mText->HasFocus())
		return mText;

	return this;
}
