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
#include "CAddressList.h"
#include "CAddressText.h"
#include "CAdminLock.h"
#include "CBodyTable.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CDivider.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CFocusBorder.h"
#include "CFormattedTextDisplay.h"
#include "CIconLoader.h"
#include "CLetterWindow.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxWindow.h"
#include "CMainMenu.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessageFwd.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPSPrinter.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CStringUtils.h"
#include "CTableScrollbarSet.h"
#include "CToolbarButton.h"
#include "CTwister.h"
#include "CWaitCursor.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include "TPopupMenu.h"
#include "StValueChanger.h"
#include "HResourceMap.h"

#include <jXActionDefs.h>
#include <JXApplication.h>
#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXFlatRect.h>
#include <JXImageWidget.h>
#include <jXKeysym.h>
#include <JXKeyModifiers.h>
#include <JXMenuBar.h>
#include "JXMultiImageButton.h"
#include "JXMultiImageCheckbox.h"
#include <JXStaticText.h>
#include <JXScrollbar.h>
#include <JXTextMenu.h>
#include <JXUpRect.h>
#include <JXWidgetSet.h>
#include <JXWindow.h>

#include <algorithm>
#include <cassert>
#include <strstream>

const int cCaptionAreaHeight = 64;
const int cPartsCollapsedHeight = 24;
const int cSecurePaneHeight = 34;
const int cTextHeight = 64;
const int cSplitterTop = cCaptionAreaHeight;
const int cSplitterHeight = cCaptionAreaHeight;
const int cSplitterMin1 = cPartsCollapsedHeight;
const int cSplitterMin2 = 32;
const int cWindowWidth = 550;
const int cWindowHeight = 300;

// Static members


cdmutexprotect<CMessageView::CMessageViewList> CMessageView::sMsgViews;
cdstring CMessageView::sLastCopyTo;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageView::CMessageView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: CBaseView(enclosure, hSizing, vSizing, x, y, w, h), JXIdleTask(500), mColorList(GetColormap())
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

const int cTitleHeight = 16;

// Get details of sub-panes
void CMessageView::OnCreate()
{
	// Do inherited
	CBaseView::OnCreate();

	// Treat as 3-pane if in 3-pane window or is a preview in mailbox window
	CMailboxWindow* mbox_preview = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
	mIs3Pane = mbox_preview || (mOwnerWindow == C3PaneWindow::s3PaneWindow);

// begin JXLayout1

    mHeader =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 550,64);
    assert( mHeader != NULL );

    mCaption1 =
        new CAddressText((JXScrollbarSet*) NULL,mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,3, 255,30);
    assert( mCaption1 != NULL );

    mCaption2 =
        new CAddressText((JXScrollbarSet*) NULL,mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 263,3, 255,30);
    assert( mCaption2 != NULL );

    mZoomBtn =
        new CToolbarButton("",mHeader,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 525,8, 20,20);
    assert( mZoomBtn != NULL );

    mPartsMove =
        new JXFlatRect(mHeader,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 3,33, 544,28);
    assert( mPartsMove != NULL );

    mSeparator =
        new CDivider(mPartsMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 544,2);
    assert( mSeparator != NULL );

    mPartsTitle =
        new JXStaticText("Parts:", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,8, 49,20);
    assert( mPartsTitle != NULL );

    mPartsTwister =
        new CTwister(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,8, 16,16);
    assert( mPartsTwister != NULL );

    CDivider* obj1 =
        new CDivider(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 145,4, 2,24);
    assert( obj1 != NULL );

    CDivider* obj2 =
        new CDivider(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 318,4, 2,24);
    assert( obj2 != NULL );

    mPartsField =
        new JXStaticText("", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 70,8, 48,20);
    assert( mPartsField != NULL );

    mFlatHierarchyBtn =
        new JXMultiImageCheckbox(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 69,9, 16,16);
    assert( mFlatHierarchyBtn != NULL );

    mAttachments =
        new JXImageWidget(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 122,9, 16,16);
    assert( mAttachments != NULL );

    mAlternative =
        new JXImageWidget(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 122,9, 16,16);
    assert( mAlternative != NULL );

    mHeaderBtn =
        new JXMultiImageCheckbox(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 154,5, 24,22);
    assert( mHeaderBtn != NULL );

    mTextFormat =
        new HPopupMenu("",mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 185,6, 130,20);
    assert( mTextFormat != NULL );

    mFontIncreaseBtn =
        new JXMultiImageButton(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 330,5, 24,22);
    assert( mFontIncreaseBtn != NULL );

    mFontDecreaseBtn =
        new JXMultiImageButton(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 354,5, 24,22);
    assert( mFontDecreaseBtn != NULL );

    mFontScaleField =
        new JXStaticText("", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 380,8, 25,20);
    assert( mFontScaleField != NULL );

    mQuoteDepthPopup =
        new HPopupMenu("",mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 412,6, 130,20);
    assert( mQuoteDepthPopup != NULL );

// end JXLayout1

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cPartsCollapsedHeight);
	heights.InsertElementAtIndex(2, cTextHeight);
	minHeights.InsertElementAtIndex(1, cPartsCollapsedHeight);
	minHeights.InsertElementAtIndex(2, 16);
	
	mSplitterView = new CSplitterView(heights, 0, minHeights, this,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, GetApertureHeight() - cSplitterTop);

	// Now create sub-views
    mPartsFocus =
        new CFocusBorder(mSplitterView, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, cWindowWidth, cWindowHeight);
	
    mPartsScroller =
        new CTableScrollbarSet(mPartsFocus, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, cWindowWidth - 2 * cFocusBorderInset, cWindowHeight - 2 * cFocusBorderInset);
	mPartsScroller->SetBorderWidth(0);

	mPartsTable = new 
	  CBodyTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0, cTitleHeight, cWindowWidth, GetApertureHeight() - cSplitterTop);

	mPartsTitles = new CSimpleTitleTable(mPartsScroller, mPartsScroller->GetScrollEnclosure(),
														JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cWindowWidth, cTitleHeight);
	mPartsTable->OnCreate();
	mPartsTitles->OnCreate();

	mPartsTitles->SyncTable(mPartsTable, true);
	mPartsTable->SetTitles(mPartsTitles);
	mPartsTitles->LoadTitles("UI::Titles::MessageParts", 5);
			 
    mBottomPane =
        new JXFlatRect(mSplitterView, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, cWindowWidth, cWindowHeight);

    mSecurePane =
        new JXUpRect(mBottomPane, JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, cWindowWidth, cSecurePaneHeight);
	mSecureInfo = 
	  new CTextDisplay(static_cast<JXScrollbarSet*>(NULL), mSecurePane, JXWidget::kHElastic, JXWidget::kVElastic, 2, 2, cWindowWidth - 4, cSecurePaneHeight - 4);
	mSecureInfo->SetBackgroundColor(GetColormap()->GetDefaultBackColor());
	mSecureInfo->SetBreakCROnly(kTrue);
	mSecureInfo->SetBorderWidth(0);

    mTextFocus =
        new CFocusBorder(mBottomPane, JXWidget::kHElastic, JXWidget::kVElastic, 0, cSecurePaneHeight, cWindowWidth, cWindowHeight - cSecurePaneHeight);

	mText = 
	  new CFormattedTextDisplay(mTextFocus, JXWidget::kHElastic, JXWidget::kVElastic,
										0, 0, cWindowWidth - 2 * cFocusBorderInset, cWindowHeight - cSecurePaneHeight - 2 * cFocusBorderInset);
	mText->SetBorderWidth(0);
	mText->OnCreate();
	mText->SetMessageView(this);
	mText->FitToEnclosure();
#ifndef USE_FONTMAPPER
	mText->ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), mFontScale);
#else
	mText->ResetFont(CPreferences::sPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, mFontScale);
#endif
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetFindAllowed(true);

	// Only use focus if 3pane
	if (mOwnerWindow != C3PaneWindow::s3PaneWindow)
	{
		// NB Do this after the content of the focus widget has been created to ensure
		// the content size is properly reset when border goes away.
		mPartsFocus->HasFocus(false);
		mPartsFocus->SetBorderWidth(0);
		mTextFocus->HasFocus(false);
		mTextFocus->SetBorderWidth(0);
	}

	// Hide the verify/decrypt details
	ShowSecretPane(false);

	// Install the splitter items
	mSplitterView->InstallViews(mPartsFocus, mBottomPane, true);
	mSplitterView->ShowView(false, true);
	mSplitterView->SetMinima(54, 64);
	mSplitterView->SetPixelSplitPos(54);
	mSplitterView->SetLockResize(true);

	// Toggle header button
	if (mShowHeader)
		mHeaderBtn->ToggleState();

	mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
	mZoomBtn->SetSmallIcon(true);
	mZoomBtn->SetShowIcon(true);
	mZoomBtn->SetShowCaption(false);
	if (mOwnerWindow != C3PaneWindow::s3PaneWindow)
		mZoomBtn->Hide();
	
	mCaption1->SetBackgroundColor(GetColormap()->GetDefaultBackColor());
	mCaption1->SetBreakCROnly(kTrue);
	mCaption1->SetBorderWidth(0);

	mCaption2->SetBackgroundColor(GetColormap()->GetDefaultBackColor());
	mCaption2->SetBreakCROnly(kTrue);
	mCaption2->SetBorderWidth(0);

	// Null these out to prevent captions flash when first opened
	mPartsField->SetText(cdstring::null_str);

	mAttachments->SetImage(CIconLoader::GetIcon(IDI_MSG_ATTACHMENT, mAttachments, 16, 0x00CCCCCC), kFalse);
	mAlternative->SetImage(CIconLoader::GetIcon(IDI_MSG_ALTERNATIVE, mAlternative, 16, 0x00CCCCCC), kFalse);
	mFlatHierarchyBtn->SetImages(IDI_SERVERFLAT, 0, IDI_SERVERHIERARCHIC, 0);
	mFlatHierarchyBtn->Hide();
	mHeaderBtn->SetImage(IDI_HEADERMSG);
	mFontIncreaseBtn->SetImage(IDI_FONTINCREASE);
	mFontDecreaseBtn->SetImage(IDI_FONTDECREASE);

	// Format Menu
	mTextFormat->SetMenuItems("Formatted %r | Plain Text %r | Source %r%l | as HTML %r | as Enriched %r | With Fixed Font %r%l | Raw Message %r");
	mTextFormat->SetUpdateAction(JXMenu::kDisableNone);
	mTextFormat->SetToPopupChoice(kTrue, eViewFormatted);

	// Quote Depth Menu
	mQuoteDepthPopup->SetMenuItems("All Quotes %r | No Quotes %r%l | > 1 Level %r | >> 2 Levels%r | >>> 3 Levels%r | >>>> 4 Levels%r");
	mQuoteDepthPopup->SetUpdateAction(JXMenu::kDisableNone);
	mQuoteDepthPopup->SetToPopupChoice(kTrue, eQuoteDepth_All);

	// Listen to widgets
	ListenTo(mZoomBtn);
	ListenTo(mHeaderBtn);
	ListenTo(mPartsTwister);
	ListenTo(mFlatHierarchyBtn);
	ListenTo(mTextFormat);
	ListenTo(mFontIncreaseBtn);
	ListenTo(mFontDecreaseBtn);
	ListenTo(mQuoteDepthPopup);
	mParsing = eViewFormatted;

	mText->Add_Listener(this);

	// Tooltips
	mPartsTwister->SetHint(stringFromResource(IDC_MESSAGEPARTSTWISTER));
	mPartsField->SetHint(stringFromResource(IDC_MESSAGEPARTSFIELD));
	mFlatHierarchyBtn->SetHint(stringFromResource(IDC_MESSAGEPARTSFLATBTN));
	mAttachments->SetHint(stringFromResource(IDC_MESSAGEPARTSATTACHMENTS));
	mAlternative->SetHint(stringFromResource(IDC_MESSAGEPARTSALTERNATIVE));

	mHeaderBtn->SetHint(stringFromResource(IDC_MESSAGES_SHOW_HEADER));
	mTextFormat->SetHint(stringFromResource(IDC_MESSAGES_TEXT_FORMAT));

	// Reset to default size
	ResetState();

	// Reset based on current view options
	ResetOptions();

	// Set status
	Deactivate();
	SetOpen();
}

// Resize columns
void CMessageView::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CBaseView::ApertureResized(dw, dh);

	// Captions occupy half the width each, adjusted for zoom button
	JCoordinate pwidth = mHeader->GetApertureWidth();
	JCoordinate cwidth = (pwidth - ((mOwnerWindow == C3PaneWindow::s3PaneWindow) ? 18 : 0) - 10) / 2;
	
	JCoordinate cwidth1 = mCaption1->GetFrameWidth();
	
	if (cwidth1 != cwidth)
	{
		mCaption1->Place(0, 3);
		mCaption1->AdjustSize(cwidth - cwidth1, 0);
		mCaption2->Place(cwidth + 5, 3);
		mCaption2->AdjustSize(cwidth - cwidth1, 0);
	}
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
	return mText->IsTarget();
}

void CMessageView::Focus()
{
	if (GetWindow()->IsVisible())
		// Switch target to the text (will focus)
		mText->Focus();
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

	// Now check for auto verify/decrypt - but not when restoring
	else if (mItsMsg && !restore &&
		(CPreferences::sPrefs->mAutoVerify.GetValue() && mItsMsg->GetBody()->IsVerifiable() ||
		CPreferences::sPrefs->mAutoDecrypt.GetValue() && mItsMsg->GetBody()->IsDecryptable()))
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
	if (mItsMsg)
	{
		CAttachment* attach = mItsMsg->FirstDisplayPart();
		ShowPart(attach, restore);
		mPartsTable->SetRowShow(attach);

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
		// Wipe out the text
#ifndef USE_FONTMAPPER
		mText->ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), mFontScale);
#else
		mText->ResetFont(CPreferences::sPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, mFontScale);
#endif
		mText->SetText(cdstring::null_str);
	}
		
	// Do attachments icon
	if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1))
	{
		if (mItsMsg->GetBody()->HasUniqueTextPart())
		{
			mAttachments->Hide();
			mAlternative->Show();
		}
		else
		{
			mAttachments->Show();
			mAlternative->Hide();
		}
	}
	else
	{
		mAttachments->Hide();
		mAlternative->Hide();
	}

	// Update window features
	MessageChanged();

	// No longer allow deleted
	mAllowDeleted = false;

	// Do final set message processing
	PostSetMessage(restore);
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
			mHeaderBtn->SetState(JBoolean(state.mShowHeader));
		if (state.mPartsExpanded ^ mPartsTwister->IsChecked())
			mPartsTwister->SetState(JBoolean(!mPartsTwister->IsChecked()));
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
	// Do selection before scroll as selection change may cause scroll
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
}

// Processing after message set and displayed
void CMessageView::PostSetMessage(bool restore)
{
	// Only do this if visible and not restoring
	if (!restore && GetWindow()->IsVisible())
	{
		// Do parts expand if no visible part or multiparts and the parts area is visible
		if (mShowParts)
		{
			// Do parts expand if no visible part or multiparts
			if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
			{
				// Do auto expansion or expand if no parts
				if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
						!mPartsTwister->IsChecked())
				{
					mPartsTwister->SetState(kTrue);
					mDidExpandParts = true;

					// Must switch focus back to text as expanding parts sets it to the parts table
					// Only do this when in 1-pane mode
					if (!Is3Pane())
						mText->Focus();
				}
			}
			else
			{
				// Do auto collapse
				if (CPreferences::sPrefs->mExpandParts.GetValue() &&
					mPartsTwister->IsChecked() && mDidExpandParts)
				{
					mPartsTwister->SetState(kFalse);
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

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CMessageView::ClearMessage()
{
	// Remove the reference to it here
	mItsMsg = NULL;
	mItsMsgError = true;
	
	// Remove the reference to it in the attachment table
	mPartsTable->ClearBody();
}

// Someone else changed this message
void CMessageView::MessageChanged()
{
	// Do not process redisplay if blocked
	if (mRedisplayBlock)
		return;

	// Disable parts area controls if no suitable message
	if (mItsMsg && !mItsMsg->IsFake())
		Activate();
	else
	{
		Deactivate();

		// If currently active, force focus to owner view
		if (mText->HasFocus() || mPartsTable->HasFocus() || mCaption1->HasFocus() || mCaption2->HasFocus())
			mMailboxView->GetBaseTable()->Focus();
	}

	// Close window if message deleted
	if (!mAllowDeleted && mItsMsg && mItsMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
		SetMessage(NULL);
}

// Reset message text
void CMessageView::ResetText()
{
	// Only do if message exists and not blocked
	if (!mItsMsg || mResetTextBlock) return;

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
		else if (GetViewOptions().GetShowSummary())
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

	// Make it active in 1-pane mode
	if (!Is3Pane())
		mText->Focus();
}

// Reset font scale text
void CMessageView::ResetFontScale()
{
	// Set text
	cdstring txt;
	txt.reserve(32);
	if (mFontScale != 0)
		::snprintf(txt.c_str_mod(), 32, "%+ld", mFontScale);
	mFontScaleField->SetText(txt);

	// Enable/disable controls
	mFontIncreaseBtn->SetActive(JBoolean(mFontScale < 7));
	mFontDecreaseBtn->SetActive(JBoolean(mFontScale > -5));
}

// Reset message text
void CMessageView::ResetCaption()
{
	{
		// Don't allow drawing until complete
		StNoRedraw _noredraw1(mCaption1);
		StNoRedraw _noredraw2(mCaption2);

		// Delete any previous text
		mCaption1->SetText(cdstring::null_str);
		mCaption2->SetText(cdstring::null_str);

		CEnvelope* env = (mItsMsg && mItsMsg->GetEnvelope()) ? mItsMsg->GetEnvelope() : NULL;

		// From
		if (env && env->GetFrom()->size())
		{
			mCaption1->SetCurrentFontBold(kTrue);
			mCaption1->InsertUTF8(cHDR_FROM);
			mCaption1->SetCurrentFontBold(kFalse);
			cdstring addr = env->GetFrom()->front()->GetFullAddress();
			mCaption1->InsertUTF8(addr);
		}

		// To
		if (env && env->GetTo()->size())
		{
			mCaption2->SetCurrentFontBold(kTrue);
			mCaption2->InsertUTF8(cHDR_TO);
			mCaption2->SetCurrentFontBold(kFalse);
			cdstring addr = env->GetTo()->front()->GetFullAddress();
			if (env->GetTo()->size() > 1)
				addr += ", ...";
			mCaption2->InsertUTF8(addr);
		}

		// CC
		if (env && env->GetCC()->size())
		{
			cdstring addr = "    ";
			mCaption2->InsertUTF8(addr);

			mCaption2->SetCurrentFontBold(kTrue);
			mCaption2->InsertUTF8(cHDR_CC);
			mCaption2->SetCurrentFontBold(kFalse);
			addr = env->GetCC()->front()->GetFullAddress();
			if (env->GetCC()->size() > 1)
				addr += ", ...";
			addr += "    ";
			mCaption2->InsertUTF8(addr);
		}

		// Next line
		mCaption1->InsertUTF8(os_endl);
		mCaption2->InsertUTF8(os_endl);

		// Subject
		if (env)
		{
			mCaption1->SetCurrentFontBold(kTrue);
			mCaption1->InsertUTF8(cHDR_SUBJECT);
			mCaption1->SetCurrentFontBold(kFalse);
			cdstring subj = env->GetSubject();
			mCaption1->InsertUTF8(subj);
		}

		// Date
		if (env)
		{
			mCaption2->SetCurrentFontBold(kTrue);
			mCaption2->InsertUTF8(cHDR_DATE);
			mCaption2->SetCurrentFontBold(kFalse);
			cdstring date = env->GetTextDate(true, false);
			mCaption2->InsertUTF8(date);
		}
	}
	
	// Force redraw
	mCaption1->ScrollTo(0, 0);
	mCaption1->Refresh();
	mCaption2->ScrollTo(0, 0);
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
	if ((mOwnerWindow == C3PaneWindow::s3PaneWindow) && (mShowAddressCaption || mShowParts))
		mZoomBtn->Show();
	else
		mZoomBtn->Hide();
}

// Update attachments caption
void CMessageView::UpdatePartsCaption()
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
	mPartsField->SetText(caption);
}

// Update list of attachments, show/hide
void CMessageView::UpdatePartsList()
{
	mPartsTable->SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
	UpdatePartsCaption();
}

// Show sub-message window
void CMessageView::ShowSubMessage(CAttachment* attach)
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
		newWindow->GetWindow()->Show();
		
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
void CMessageView::SetSecretPane(const CMessageCryptoInfo& info)
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
					mSecureInfo->SetCurrentFontColor(mColorList.Add(green));
				}
				else
				{
					txt += "Signature: Bad";
					JRGB red(0xDDDD, 0x0000, 0x0000);
					mSecureInfo->SetCurrentFontColor(mColorList.Add(red));
				}
				mSecureInfo->InsertUTF8(txt);

				JRGB black(0x0000, 0x0000, 0x0000);
				mSecureInfo->SetCurrentFontColor(mColorList.Add(black));
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
					mSecureInfo->SetCurrentFontColor(mColorList.Add(red));
					
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
				mSecureInfo->SetCurrentFontColor(mColorList.Add(green));

				cdstring txt;
				txt += "Decrypted: OK";
				mSecureInfo->InsertUTF8(txt);

				JRGB black(0x0000, 0x0000, 0x0000);
				mSecureInfo->SetCurrentFontColor(mColorList.Add(black));
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
			mSecureInfo->SetCurrentFontColor(mColorList.Add(red));

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
				mTextFocus->AdjustSize(0, -moveby);
				mTextFocus->Move(0, moveby);
			}
			mSecurePane->AdjustSize(0, moveby);
		}
		else
		{
			mSecurePane->AdjustSize(0, -moveby);
			if (mShowSecure)
			{
				mTextFocus->AdjustSize(0, moveby);
				mTextFocus->Move(0, -moveby);
			}
		}

		mSecureMulti = multi_line;
	}
}

// Reset text traits from prefs
void CMessageView::ResetFont(const SFontInfo& list_font, const SFontInfo& display_font)
{
	mPartsTable->ResetFont(list_font);
	mPartsTitles->ResetFont(list_font);
	mPartsTitles->SyncTable(mPartsTable, true);
	mText->ResetFont(display_font, mFontScale);
}

// Temporarily add header summary for printing	cdstring caption;
void CMessageView::AddPrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader || !GetViewOptions().GetShowSummary()))
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
void CMessageView::RemovePrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader || !GetViewOptions().GetShowSummary()))
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

// Scroll the text if possible
bool CMessageView::SpacebarScroll(bool shift_key)
{
	// Check whether scrolled to the bottom or no scrollbar
	JXScrollbar* horiz;
	JXScrollbar* vert;
	mText->GetScrollbars(&horiz, &vert);
	if (vert->IsVisible())
	{
		if (shift_key)
		{
			if (!vert->IsAtMin())
			{
				mText->HandleChar(XK_Page_Up, JXKeyModifiers(GetDisplay()));
				return true;
			}
		}
		else
		{
			if (!vert->IsAtMax())
			{
				mText->HandleChar(XK_Page_Down, JXKeyModifiers(GetDisplay()));
				return true;
			}
		}
	}
	
	return false;
}

#pragma mark ____________________________Timer

// Called during idle
void CMessageView::Perform(const Time delta, Time* maxSleepTime)
{
	*maxSleepTime = 500;
	Time bogus;
	if (!TimeToPerform(delta, &bogus))
		return;

	// See if we are equal or greater than trigger
	if (::time(NULL) >= mSeenTriggerTime)
	{
		// Change the seen flag and stop the timer
		if (mItsMsg && mItsMsg->IsUnseen())
		{
			mItsMsg->ChangeFlags(NMessage::eSeen, true);
			
			// If seen state has changed do actions associated with that
			if (mItsMsg && mWasUnseen)
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
	CMulberryApp::sApp->InstallPermanentTask(this);
}

// Stop timer to trigger seen flag
void CMessageView::StopSeenTimer()
{
	// Just stop idle time processing
	CMulberryApp::sApp->RemovePermanentTask(this);
}

#pragma mark ____________________________________Command Updaters

// Common updaters
void CMessageView::OnUpdateMessageReadPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true) ||
					 mItsMsg->IsSubMessage() && mItsMsg->GetPrevDigest()));
}

void CMessageView::OnUpdateMessageReadNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() ||
						mItsMsg->IsSubMessage() && mItsMsg->GetNextDigest()));
}

void CMessageView::OnUpdateMessageCopyNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg != NULL);
}

void CMessageView::OnUpdateMessageReject(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowRejectCommand);
}

void CMessageView::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(mItsMsg && mItsMsg->GetEnvelope()->GetFrom()->size() &&
					CPreferences::TestSmartAddress(*mItsMsg->GetEnvelope()->GetFrom()->front()));
}

void CMessageView::OnUpdateMessageCopy(CCmdUI* pCmdUI)
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

void CMessageView::OnUpdateMessageDelete(CCmdUI* pCmdUI)
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

void CMessageView::OnUpdateMessageViewCurrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mCurrentPart != NULL) && (mParsing != eViewAsRaw));
}

void CMessageView::OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity());
}

void CMessageView::OnUpdateMessageFlagsTop(CCmdUI* pCmdUI)
{
	bool enabled = mItsMsg && !mItsMsg->IsSubMessage() &&
					mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eIMAPFlags);
	pCmdUI->Enable(enabled);
}

void CMessageView::OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eSeen);
}

void CMessageView::OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eFlagged);
}

void CMessageView::OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eAnswered);
}

void CMessageView::OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDeleted);
}

void CMessageView::OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDraft);
}

void CMessageView::OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->mCmd - CCommand::eFlagsLabel1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->mCmd - CCommand::eFlagsLabel1]->name);
}

void CMessageView::OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag)
{
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(flag));
	pCmdUI->SetCheck(mItsMsg && mItsMsg->HasFlag(flag));
}

void CMessageView::OnUpdateMessageShowHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(!mShowHeader ? IDS_SHOWHEADER : IDS_HIDEHEADER);
	
	pCmdUI->SetText(txt);
}

void CMessageView::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(!mPartsTwister->IsChecked() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	pCmdUI->SetText(txt);
}

#pragma mark ____________________________Commands

void CMessageView::OnFileNewDraft()
{
	DoNewLetter(GetDisplay()->GetLatestKeyModifiers().control());
}

void CMessageView::OnFilePageSetup()
{
	mText->SetPSPrinter(CPSPrinter::sPSPrinter.GetPSPrinter(GetWindow()));
	mText->HandlePSPageSetup();
}

void CMessageView::OnFilePrint()
{
	mText->SetPSPrinter(CPSPrinter::sPSPrinter.GetPSPrinter(GetWindow()));
	mText->PrintPS();
}

void CMessageView::OnMessageCopyRead(void)
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();
	CopyReadNextMessage(option_key);
}

void CMessageView::OnMessageReply(void)
{
	ReplyToThisMessage(replyReplyTo, GetDisplay()->GetLatestKeyModifiers().control());
}

void CMessageView::OnMessageReplySender(void)
{
	ReplyToThisMessage(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageView::OnMessageReplyFrom(void)
{
	ReplyToThisMessage(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageView::OnMessageReplyAll(void)
{
	ReplyToThisMessage(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

// Forward this message
void CMessageView::OnMessageForward(void)
{
	ForwardThisMessage(GetDisplay()->GetLatestKeyModifiers().control());
}

// Copy the message to chosen mailbox
void CMessageView::OnMessageCopyBtn()
{
	TryCopyMessage(GetDisplay()->GetLatestKeyModifiers().control());
}

// Copy the message to another mailbox
void CMessageView::OnMessageCopy(JXTextMenu* menu, JIndex nID)
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
void CMessageView::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMessageView::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
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

void CMessageView::OnTextFormatPopup(JIndex nID)
{
	mTextFormat->SetValue(nID);
	EView old_view = mParsing;
	EView new_view = static_cast<EView>(nID);
	OnViewAs(new_view);
	
	// Check for failure to change and reset popup to old value
	if (mParsing == old_view)
		mTextFormat->SetValue(mParsing);
	
	// Turn off quote depth if not formatted mode
	mQuoteDepthPopup->SetActive(JBoolean(mParsing == eViewFormatted));
}

void CMessageView::OnMessageFlagsSeen()
{
	OnMessageFlags(NMessage::eSeen);
}

void CMessageView::OnMessageFlagsAnswered()
{
	OnMessageFlags(NMessage::eAnswered);
}

void CMessageView::OnMessageFlagsImportant()
{
	OnMessageFlags(NMessage::eFlagged);
}

void CMessageView::OnMessageFlagsDraft()
{
	OnMessageFlags(NMessage::eDraft);
}

void CMessageView::OnMessageFlagsLabel(JIndex nID)
{
	OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << nID));
}

void CMessageView::OnMessageShowHeader(void)
{
	// Toggle header display
	mShowHeader = mHeaderBtn->IsChecked();

	// Reset this message
	ResetText();
}

void CMessageView::OnMessagePartsTwister(void)
{
	bool expanding = mPartsTwister->IsChecked();
	JCoordinate moveby = mPartsScroller->GetFrameHeight();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kFixedTop);

		// Hide the parts table pane in the splitter
		mSplitterView->ShowView(false, true);

		// Show/hide items
		mFlatHierarchyBtn->Hide();
		mPartsField->Show();

		// Only do this when in 1-pane mode or when parts is the current target
		if (!Is3Pane() || mPartsTable->IsTarget())
			mText->Focus();
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitterView->ShowView(true, true);

		// Turn on resize of parts area when compartment changes
		mPartsScroller->SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);
		
		// Show/hide items
		mFlatHierarchyBtn->Show();
		mPartsField->Hide();
		
		// Only do this when in 1-pane mode
		if (!Is3Pane())
			mPartsTable->Focus();
	}

	GetWindow()->Refresh();
}

void CMessageView::OnMessagePartsFlat()
{
	// Toggle flat state
	DoPartsFlat(!mPartsTable->GetFlat());
}

// Flatten parts table
void CMessageView::DoPartsFlat(bool flat)
{
	mPartsTable->SetFlat(flat);
	UpdatePartsCaption();
}

void CMessageView::OnZoomPane()
{
	C3PaneWindow::s3PaneWindow->ObeyCommand(CCommand::eToolbarZoomPreview, NULL);
}

#pragma mark ____________________________Window State

// Get current state of the view
void CMessageView::GetViewState(SMessageViewState& state) const
{
	mText->GetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->GetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	state.mShowHeader = mShowHeader;
	state.mPartsExpanded = mPartsTwister->IsChecked();
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
		mHeaderBtn->SetState(JBoolean(!mShowHeader));
		changed = true;
	}
	if (state.mPartsExpanded ^ mPartsTwister->IsChecked())
	{
		mPartsTwister->SetState(JBoolean(!mPartsTwister->IsChecked()));
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

	// Restore scroll and selection state
	// Do selection before scroll as selection change may cause scroll
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
}

// Show/hide address caption area
void CMessageView::ShowCaption(bool show)
{
	int moveby = mCaption1->GetFrameHeight();

	if (show)
	{
		// Shrink/move splitter
		mSplitterView->AdjustSize(0, -moveby);
		mSplitterView->Move(0, moveby);

		// Always move parts header even if not visible
		mHeader->Move(0, moveby);

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

		// Always move parts header even if not visible
		mHeader->Move(0, -moveby);

		// Expand/move splitter
		mSplitterView->AdjustSize(0, moveby);
		mSplitterView->Move(0, -moveby);
	}
}

// Show/hide parts area
void CMessageView::ShowParts(bool show)
{
	// Collapse parts if needed
	if (!show && mPartsTwister->IsChecked())
		mPartsTwister->SetState(kFalse);

	int moveby = mPartsMove->GetFrameHeight();

	if (show)
	{
		// Shrink/move splitter
		mSplitterView->AdjustSize(0, -moveby);
		mSplitterView->Move(0, moveby);

		// Increase header height
		mHeader->AdjustSize(0, moveby);

		// Show caption after all other changes
		mPartsMove->Show();
	}
	else
	{
		// Hide caption before other changes
		mPartsMove->Hide();

		// Decrease header height
		mHeader->AdjustSize(0, -moveby);

		// Expand/move splitter
		mSplitterView->AdjustSize(0, moveby);
		mSplitterView->Move(0, -moveby);
	}
}

// Show/hide parts area
void CMessageView::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	int moveby = mSecurePane->GetFrameHeight();

	if (show)
	{
		// Shrink/move text pane
		mTextFocus->AdjustSize(0, -moveby);
		mTextFocus->Move(0, moveby);

		// Show parts after all other changes
		mSecurePane->Show();
	}
	else
	{
		// Hide parts before other changes
		mSecurePane->Hide();

		// Expand/move splitter
		mTextFocus->AdjustSize(0, moveby);
		mTextFocus->Move(0, -moveby);
	}

	mShowSecure = show;
}

// Reset state from prefs
void CMessageView::ResetState(bool force)
{
	CMessageWindowState& state = (mOwnerWindow == C3PaneWindow::s3PaneWindow) ?
										CPreferences::sPrefs->mMessageView3Pane.Value() :
										CPreferences::sPrefs->mMessageView1Pane.Value();

	// Force twist down if required
	if (state.GetPartsTwisted())
		mPartsTwister->SetState(kTrue);
	mSplitterView->SetPixelSplitPos(state.GetSplitChange());
	mFlatHierarchyBtn->SetState(state.GetFlat() ? kFalse : kTrue);
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
	JRect bounds;

	// Add info to prefs
	CMessageWindowState state(NULL, &bounds, eWindowStateNormal,
								mSplitterView->GetPixelSplitPos(), mPartsTwister->IsChecked(), false, mPartsTable->GetFlat(), false);

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

void CMessageView::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mPartsTwister)
		{
			OnMessagePartsTwister();
			return;
		}
		else if (sender == mFlatHierarchyBtn)
		{
			OnMessagePartsFlat();
			return;
		}
		else if (sender == mHeaderBtn)
		{
			OnMessageShowHeader();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoomPane();
			return;
		}		
		else if (sender == mFontIncreaseBtn)
		{
			OnIncreaseFont();
			return;
		}		
		else if (sender == mFontDecreaseBtn)
		{
			OnDecreaseFont();
			return;
		}		
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mTextFormat)
		{
			OnTextFormatPopup(is->GetIndex());
			return;
		}
		else if (sender == mQuoteDepthPopup)
		{
			OnQuoteDepth(static_cast<EQuoteDepth>(is->GetIndex()));
			return;
		}
	}
	CBaseView::Receive(sender, message);
}

// Handle key presses
bool CMessageView::HandleChar(const int key, const JXKeyModifiers& modifiers)
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

	case 'z':
	case 'Z':
		cmd = CCommand::eToolbarZoomPreview;
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
	return CBaseView::HandleChar(key, modifiers);
}

void CMessageView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eFileSave:
	case CCommand::eFileSaveAs:
	case CCommand::eToolbarFileSaveBtn:
	case CCommand::eFilePageSetup:
	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
	case CCommand::eMessagesReply:
	case CCommand::eMessagesReplyToSender:
	case CCommand::eMessagesReplyToFrom:
	case CCommand::eMessagesReplyToAll:
	case CCommand::eMessagesForward:
	case CCommand::eMessagesBounce:
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
	case CCommand::eCommandMessageCopy:
	case CCommand::eCommandMessageMove:
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
		else if (cmdui->mMenu == mTextFormat)
		{
			OnUpdateAlways(cmdui);
			return;
		}
		else if (cmdui->mMenu == mQuoteDepthPopup)
		{
			OnUpdateAlways(cmdui);
			return;
		}
	}

	CCommander::UpdateCommand(cmd, cmdui);
}

//	Respond to commands
bool CMessageView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
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
		//SaveInNewFile();
		return true;

	case CCommand::eFilePageSetup:
		OnFilePageSetup();
		return true;

	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
		OnFilePrint();
		return true;

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
		if (menu && GetCopyBtn())
			GetCopyBtn()->SetValue(menu->mIndex);
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
		mHeaderBtn->ToggleState();
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

	case CCommand::eWindowsShowParts:
		mPartsTwister->ToggleState();
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

	return CCommander::ObeyCommand(cmd, menu);
}
