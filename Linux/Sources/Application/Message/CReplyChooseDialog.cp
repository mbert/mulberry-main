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


// Source for CReplyChooseDialog class


#include "CReplyChooseDialog.h"

#include "CAddressList.h"
#include "CEnvelope.h"
#include "CIconLoader.h"
#include "CLetterWindow.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CSimpleTitleTable.h"

#include <UNX_LTableMultiGeometry.h>

#include "StPenState.h"

#include "TPopupMenu.h"

#include <JXImage.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CReplyChooseDialog dialog


CReplyChooseDialog::CReplyChooseDialog(JXDirector* supervisor, bool reply)
	: CDialogDirector(supervisor)
{
	mItsMsg = NULL;
	mMsgs = NULL;
	mItsList = NULL;
	mIsReply = reply;
	mAddToDraft = NULL;

	mCreateNewBtn = NULL;
	mReplyToBtn = NULL;
	mReplyAllBtn = NULL;
	mReplyFromBtn = NULL;
	mReplySenderBtn = NULL;
	mReplyNoneBtn = NULL;
	mAddressToBtn = NULL;
	mAddressCcBtn = NULL;
	mAddressBccBtn = NULL;
	mAddressNoneBtn = NULL;
}

CReplyChooseDialog::~CReplyChooseDialog()
{
}

const int cTitleHeight = 16;

void CReplyChooseDialog::OnCreate()
{
    JXWindow* window = new JXWindow(this, 400, mIsReply ? 310 : 285, "");
    assert( window != NULL );
    SetWindow(window);

	JXScrollbarSet* my_sbs = NULL;

	if (mIsReply)
	{
// begin JXLayout1

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 400,310);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Select Addresses to be used in the Reply:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 255,20);
    assert( obj2 != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 380,150);
    assert( sbs != NULL );

    mOKBtn =
        new JXTextButton("Create Reply", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 290,275, 90,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 70,275, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mCreateNewBtn =
        new JXTextButton("Create New", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,275, 90,25);
    assert( mCreateNewBtn != NULL );

    mReplyToBtn =
        new JXTextButton("Reply-To", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,185, 70,20);
    assert( mReplyToBtn != NULL );
    mReplyToBtn->SetFontSize(10);

    mReplyAllBtn =
        new JXTextButton("Reply All", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,185, 70,20);
    assert( mReplyAllBtn != NULL );
    mReplyAllBtn->SetFontSize(10);

    mReplyFromBtn =
        new JXTextButton("From", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 165,185, 70,20);
    assert( mReplyFromBtn != NULL );
    mReplyFromBtn->SetFontSize(10);

    mReplySenderBtn =
        new JXTextButton("Sender", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,185, 70,20);
    assert( mReplySenderBtn != NULL );
    mReplySenderBtn->SetFontSize(10);

    mReplyNoneBtn =
        new JXTextButton("None", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,185, 70,20);
    assert( mReplyNoneBtn != NULL );
    mReplyNoneBtn->SetFontSize(10);

    mQuote =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,225, 375,40);
    assert( mQuote != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Quote:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,210, 45,20);
    assert( obj3 != NULL );

    mQuoteSelection =
        new JXTextRadioButton(eQuoteSelection, "Selection", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 95,20);
    assert( mQuoteSelection != NULL );

    mQuoteAll =
        new JXTextRadioButton(eQuoteAll, "Entire Message", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,10, 120,20);
    assert( mQuoteAll != NULL );

    mQuoteNone =
        new JXTextRadioButton(eQuoteNone, "None", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,10, 105,20);
    assert( mQuoteNone != NULL );

// end JXLayout1

	my_sbs = sbs;

	}
	else
	{
// begin JXLayout2

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 400,285);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Select Addresses to be used in the Draft:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 255,20);
    assert( obj2 != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 380,150);
    assert( sbs != NULL );

    mOKBtn2 =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 300,245, 80,25);
    assert( mOKBtn2 != NULL );
    mOKBtn2->SetShortcuts("^M");

    mCancelBtn2 =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 215,245, 70,25);
    assert( mCancelBtn2 != NULL );
    mCancelBtn2->SetShortcuts("^[");

    mAddressToBtn =
        new JXTextButton("All To", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,185, 70,20);
    assert( mAddressToBtn != NULL );
    mAddressToBtn->SetFontSize(10);

    mAddressCcBtn =
        new JXTextButton("All Cc", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,185, 70,20);
    assert( mAddressCcBtn != NULL );
    mAddressCcBtn->SetFontSize(10);

    mAddressBccBtn =
        new JXTextButton("All Bcc", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,185, 70,20);
    assert( mAddressBccBtn != NULL );
    mAddressBccBtn->SetFontSize(10);

    mAddressNoneBtn =
        new JXTextButton("None", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,185, 70,20);
    assert( mAddressNoneBtn != NULL );
    mAddressNoneBtn->SetFontSize(10);

    JXStaticText* obj3 =
        new JXStaticText("Add to Draft:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,218, 80,20);
    assert( obj3 != NULL );

    mAddToDraft =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,215, 245,20);
    assert( mAddToDraft != NULL );

// end JXLayout2

	// Init the add to popup - only present if using the address version of the dialog
	InitAddToDraft();

	my_sbs = sbs;

	}

	mTitles = new CSimpleTitleTable(my_sbs, my_sbs->GetScrollEnclosure(),
																	 JXWidget::kHElastic,
																	 JXWidget::kFixedTop,
																	 0, 0, 550, cTitleHeight);

	mItsTable = new CReplyChooseTable(my_sbs,my_sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,cTitleHeight, 550, 100);

	mItsTable->OnCreate();
	mTitles->OnCreate();

	mTitles->LoadTitles("UI::Titles::ReplyChoose", 5);

	window->SetTitle(mIsReply ? "Reply To" : "Create Draft To");
	SetButtons(mIsReply ? mOKBtn : mOKBtn2, mIsReply ? mCancelBtn : mCancelBtn2);

	if (mIsReply)
	{
		ListenTo(mCreateNewBtn);
		ListenTo(mReplyToBtn);
		ListenTo(mReplyAllBtn);
		ListenTo(mReplyFromBtn);
		ListenTo(mReplySenderBtn);
		ListenTo(mReplyNoneBtn);
	}
	else
	{
		ListenTo(mAddressToBtn);
		ListenTo(mAddressCcBtn);
		ListenTo(mAddressBccBtn);
		ListenTo(mAddressNoneBtn);
	}
}

void CReplyChooseDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed) && mIsReply)
	{
		if (sender == mCreateNewBtn)
		{
			OnCreateNewBtn();
			return;
		}
		else if (sender == mReplyToBtn)
		{
			OnSelectReplyToBtn();
			return;
		}
		else if (sender == mReplyAllBtn)
		{
			OnSelectReplyAllBtn();
			return;
		}
		else if (sender == mReplyFromBtn)
		{
			OnSelectReplyFromBtn();
			return;
		}
		else if (sender == mReplySenderBtn)
		{
			OnSelectReplySenderBtn();
			return;
		}
		else if (sender == mReplyNoneBtn)
		{
			OnSelectReplyNoneBtn();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed) && !mIsReply)
	{
		if (sender == mAddressToBtn)
		{
			OnSelectAddressToBtn();
			return;
		}
		else if (sender == mAddressCcBtn)
		{
			OnSelectAddressCcBtn();
			return;
		}
		else if (sender == mAddressBccBtn)
		{
			OnSelectAddressBccBtn();
			return;
		}
		else if (sender == mAddressNoneBtn)
		{
			OnSelectAddressNoneBtn();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CReplyChooseDialog::SetReplyQuote(EReplyQuote& quote, bool has_selection)
{
	// Set it up
	if (quote == eQuoteSelection)
		mQuote->SelectItem(has_selection ? eQuoteSelection : eQuoteAll);
	else
		mQuote->SelectItem(quote);

	// Disable selection option if none available
	if (!has_selection)
		mQuoteSelection->Deactivate();
}

void CReplyChooseDialog::GetReplyQuote(EReplyQuote& quote)
{
	// Get the proper setting
	quote = (EReplyQuote) mQuote->GetSelectedItem();
}

void CReplyChooseDialog::InitAddToDraft()
{
	// Only if it exists
	if (!mAddToDraft)
		return;

	// Remove any existing items
	mAddToDraft->RemoveAllItems();

	// Now add current items
	mAddToDraft->SetMenuItems("New Draft %r %l"); 
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			mAddToDraft->AppendItem((*iter)->GetName(), kFalse, kTrue);
	}

	// Set to first item
	mAddToDraft->SetUpdateAction(JXMenu::kDisableNone);
	mAddToDraft->SetToPopupChoice(kTrue, eAddTo_New);
}

// Get selected add to window
void CReplyChooseDialog::GetAddToWindow(CLetterWindow*& add_to)
{
	// Start off NULL
	add_to = NULL;

	// Only if it exists
	if (!mAddToDraft)
		return;

	// Get current value
	JIndex value = mAddToDraft->GetValue();
	
	// If its a new draft just return - or iif index is higher than window count
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		if ((value == eAddTo_New) || (value - eAddTo_First >= CLetterWindow::sLetterWindows->size()))
			return;
		
		// Just get the corresponding window
		add_to = CLetterWindow::sLetterWindows->at(value - eAddTo_First);
	}
}

#pragma mark ____________________________Static Processing

void CReplyChooseDialog::ProcessChoice(CMessage* aMsg, const cdstring& quotetxt, EContentSubType subtype, bool is_flowed, EReplyTo reply_to, bool use_dialog)
{
	CLetterWindow* newWindow = NULL;
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	EReplyQuote quote;
	if (quotetxt.length() && CPreferences::sPrefs->mQuoteSelection.GetValue())
		quote = eQuoteSelection;
	else
		quote = CPreferences::sPrefs->mAlwaysQuote.GetValue() ? eQuoteAll : eQuoteNone;
	long dlog_result = 1;

	if (use_dialog && !(dlog_result = CReplyChooseDialog::PoseDialog(aMsg, &to_list, &cc_list, &bcc_list, quote, quotetxt.length())))
		return;

	try
	{
		// When creating a new message rather than a reply we must explicitly set the tied identity from
		// the original message as the draft has no knowledge of the source of the addresses
		const CIdentity* id = NULL;

		// Create the letter window and give it the message
		newWindow = CLetterWindow::ManualCreate();

		// Use address lists if dlog
		if (use_dialog)
		{
			if (dlog_result == 1)
				newWindow->SetReplyMessage(aMsg, to_list, cc_list, bcc_list);
			else
			{
				// Get addresses to insert into new draft
				newWindow->AddAddressLists(to_list, cc_list, bcc_list);
				
				// Determine tied identity from original message
				CMessageList temp;
				temp.SetOwnership(false);
				temp.push_back(aMsg);
				id = CPreferences::sPrefs->GetTiedIdentity(&temp);
			}
			delete to_list;
			delete cc_list;
			delete bcc_list;
			to_list = NULL;
			cc_list = NULL;
			bcc_list = NULL;
		}
		else
			newWindow->SetReplyMessage(aMsg, reply_to);

		// Quote whole message if requested
		if (dlog_result == 1)
		{
			if (quote == eQuoteSelection)
				newWindow->IncludeMessageTxt(aMsg, NULL, quotetxt, false, subtype, is_flowed);
			else if (quote == eQuoteAll)
				newWindow->IncludeFromReply(false, false);
		}

		// Show window
		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_WINDOW(newWindow)
		delete to_list;
		delete cc_list;
		delete bcc_list;
	}
}

void CReplyChooseDialog::ProcessChoice(CMessageList* msgs, EReplyTo reply_to, bool use_dialog)
{
	CLetterWindow* newWindow = NULL;
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	EReplyQuote quote = CPreferences::sPrefs->mAlwaysQuote.GetValue() ? eQuoteAll : eQuoteNone;
	long dlog_result = 1;

	if (use_dialog && !(dlog_result = CReplyChooseDialog::PoseDialog(msgs, &to_list, &cc_list, &bcc_list, quote)))
		return;

	try
	{
		// When creating a new message rather than a reply we must explicitly set the tied identity from
		// the original message as the draft has no knowledge of the source of the addresses
		const CIdentity* id = NULL;

		// Create the letter window and give it the message
		newWindow = CLetterWindow::ManualCreate();

		// Use address lists if dlog
		if (use_dialog)
		{
			if (dlog_result == 1)
			{
				newWindow->SetReplyMessages(msgs, to_list, cc_list, bcc_list);
				msgs = NULL;
			}
			else
			{
				// Get addresses to insert into new draft
				newWindow->AddAddressLists(to_list, cc_list, bcc_list);

				// Determine tied identity from original message
				id = CPreferences::sPrefs->GetTiedIdentity(msgs);

				// Draft will not take ownership of the message list so we delete it here
				delete msgs;
				msgs = NULL;
			}
			delete to_list;
			delete cc_list;
			delete bcc_list;
			to_list = NULL;
			cc_list = NULL;
			bcc_list = NULL;
		}
		else
		{
			newWindow->SetReplyMessages(msgs, reply_to);
			msgs = NULL;
		}

		// Quote whole message if requested
		if ((dlog_result == 1) && (quote == eQuoteAll))
			newWindow->OnDraftInclude();

		// Show window
		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_WINDOW(newWindow)
		delete msgs;
		delete to_list;
		delete cc_list;
		delete bcc_list;
	}
}

void CReplyChooseDialog::ProcessChoice(CAddressList* aList, bool use_dialog)
{
	CLetterWindow* newWindow = NULL;
	CLetterWindow* add_to = NULL;
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	long dlog_result;

	if (use_dialog && !(dlog_result = CReplyChooseDialog::PoseDialog(aList, &to_list, &cc_list, &bcc_list, add_to)))
		return;

	// Check for new window or merge to existing
	if (add_to)
	{
		// Add them, then delete them
		add_to->AddAddressLists(to_list, cc_list, bcc_list);
		delete to_list;
		delete cc_list;
		delete bcc_list;
		to_list = NULL;
		cc_list = NULL;
		bcc_list = NULL;
		
		// Always force destination draft window to top
		FRAMEWORK_WINDOW_TO_TOP(add_to)
	}
	else
	{
		try
		{
			// Create the letter window and give it the message
			newWindow = CLetterWindow::ManualCreate();

			// Use address lists if dlog
			if (use_dialog)
			{
				newWindow->AddAddressLists(to_list, cc_list, bcc_list);
				delete to_list;
				delete cc_list;
				delete bcc_list;
				to_list = NULL;
				cc_list = NULL;
				bcc_list = NULL;
			}
			else
				newWindow->AddAddressLists(aList, NULL, NULL);

			// Show window
			newWindow->PostSetMessage();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Window failed to be created
			if (newWindow)
				FRAMEWORK_DELETE_WINDOW(newWindow)
			delete to_list;
			delete cc_list;
			delete bcc_list;
		}
	}
}

long CReplyChooseDialog::PoseDialog(CMessage* aMsg,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								EReplyQuote& quote,
								bool has_selection)
{
	// Create the dialog and give it the message
	CReplyChooseDialog* dlog = new CReplyChooseDialog(JXGetApplication(), true);
	dlog->OnCreate();
	dlog->mItsTable->InitTable(true);
	dlog->mTitles->SyncTable(dlog->mItsTable, true);
	dlog->mItsTable->SetMessage(aMsg);
	dlog->SetReplyQuote(quote, has_selection);
	
	// Let DialogHandler process events
	int result = dlog->DoModal(false);
	switch(result)
	{
	case kDialogClosed_OK:
	case kDialogClosed_Btn3:
	{
		dlog->mItsTable->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog->GetReplyQuote(quote);
		dlog->Close();
		return (result == kDialogClosed_OK) ? 1 : 2;
	}
	default:
		return 0;
	}
}

long CReplyChooseDialog::PoseDialog(CMessageList* msgs,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								EReplyQuote& quote)
{
	// Create the dialog and give it the message
	CReplyChooseDialog* dlog = new CReplyChooseDialog(JXGetApplication(), true);
	dlog->OnCreate();
	dlog->mItsTable->InitTable(true);
	dlog->mTitles->SyncTable(dlog->mItsTable, true);
	dlog->mItsTable->SetMessageList(msgs);
	dlog->SetReplyQuote(quote, false);

	// Let DialogHandler process events
	int result = dlog->DoModal(false);
	switch(result)
	{
	case kDialogClosed_OK:
	case kDialogClosed_Btn3:
	{
		dlog->mItsTable->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog->GetReplyQuote(quote);
		dlog->Close();
		return (result == kDialogClosed_OK) ? 1 : 2;
	}
	default:
		return 0;
	}
}

long CReplyChooseDialog::PoseDialog(CAddressList* aList,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								CLetterWindow*& add_to)
{
	// Create the dialog and give it the message
	CReplyChooseDialog* dlog = new CReplyChooseDialog(JXGetApplication(), false);
	dlog->OnCreate();
	dlog->mItsTable->InitTable(false);
	dlog->mTitles->SyncTable(dlog->mItsTable, true);
	dlog->mItsTable->SetList(aList);

	// Let DialogHandler process events
	int result = dlog->DoModal(false);
	switch(result)
	{
	case kDialogClosed_OK:
	case kDialogClosed_Btn3:
	{
		dlog->mItsTable->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog->GetAddToWindow(add_to);
		dlog->Close();
		return (result == kDialogClosed_OK) ? 1 : 2;
	}
	default:
		return 0;
	}
}

void CReplyChooseDialog::OnCreateNewBtn()
{
	EndDialog(kDialogClosed_Btn3);
}

void CReplyChooseDialog::OnSelectReplyToBtn()
{
	mItsTable->ChangeSelection(replyReplyTo);
}

void CReplyChooseDialog::OnSelectReplyAllBtn()
{
	mItsTable->ChangeSelection(replyAll);
}

void CReplyChooseDialog::OnSelectReplyFromBtn()
{
	mItsTable->ChangeSelection(replyFrom);
}

void CReplyChooseDialog::OnSelectReplySenderBtn()
{
	mItsTable->ChangeSelection(replySender);
}

void CReplyChooseDialog::OnSelectReplyNoneBtn()
{
	mItsTable->ChangeSelection(replyNone);
}

void CReplyChooseDialog::OnSelectAddressToBtn()
{
	mItsTable->ChangeAddressSelection(replyTo);
}

void CReplyChooseDialog::OnSelectAddressCcBtn()
{
	mItsTable->ChangeAddressSelection(replyCC);
}

void CReplyChooseDialog::OnSelectAddressBccBtn()
{
	mItsTable->ChangeAddressSelection(replyBCC);
}

void CReplyChooseDialog::OnSelectAddressNoneBtn()
{
	mItsTable->ChangeAddressSelection(replyNone);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CReplyChooseTable::CReplyChooseTable(
								 JXScrollbarSet* scrollbarSet,
								 JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
	: CTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	mItsMsg = NULL;
	mItsList = NULL;
	
	mUseFromInsteadOfReplyto = false;
	
	mTableGeometry = new LTableMultiGeometry(this, 128, 16);
}

// Default destructor
CReplyChooseTable::~CReplyChooseTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CReplyChooseTable::InitTable(bool originals)
{
	// Create columns and adjust flag rect
	InsertCols(originals ? 5 : 4, 1);

	SetColWidth(36, 1, 1);
	SetColWidth(36, 2, 2);
	SetColWidth(36, 3, 3);
	if (originals)
	{
		SetColWidth(100, 5, 5);
		SetColWidth(GetApertureWidth() - 16 - 208, 4, 4);
	}
	else
		SetColWidth(GetApertureWidth() - 16 - 108, 4, 4);

	// Load strings
	if (originals)
	{
		cdstring s;
		mOriginalItems.push_back(s.FromResource("UI::ReplyChoose::ReplyTo"));
		mOriginalItems.push_back(s.FromResource("UI::ReplyChoose::From"));
		mOriginalItems.push_back(s.FromResource("UI::ReplyChoose::Sender"));
		mOriginalItems.push_back(s.FromResource("UI::ReplyChoose::To"));
		mOriginalItems.push_back(s.FromResource("UI::ReplyChoose::Cc"));
	}
}

// Draw a cell
void CReplyChooseTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Don't call HilightIfSelected

	StPenState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get data
	const SReplyElement& elem = mStorage.at(inCell.row - 1);

	switch(inCell.col)
	{

	case 1:
	case 2:
	case 3:
	{
		// Check for tick
		JXImage* icon = NULL;
		if (((inCell.col==1) && (elem.reply_type == replyTo)) ||
			((inCell.col==2) && (elem.reply_type == replyCC)) ||
			((inCell.col==3) && (elem.reply_type == replyBCC)))
			icon = CIconLoader::GetIcon(IDI_DIAMONDTICKED, this, 16, 0x00FFFFFF);
		else
			icon = CIconLoader::GetIcon(IDI_DIAMOND, this, 16, 0x00FFFFFF);

		pDC->Image(*icon, icon->GetBounds(), inLocalRect.left + 6, inLocalRect.top);
		break;
	}

	case 4:
		// Write address
		::DrawClippedStringUTF8(pDC, elem.reply_addr.GetFullAddress(), JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 5:	// Will not get this if no original column
		if (elem.original_type != replyNone)
		{
			short index = 1;
			switch(elem.original_type)
			{
			case replyReplyTo:
				index = eOriginal_ReplyTo;
				break;

			case replyFrom:
				index = eOriginal_From;
				break;

			case replySender:
				index = eOriginal_Sender;
				break;

			case replyTo:
				index = eOriginal_To;
				break;

			case replyCC:
				index = eOriginal_CC;
				break;

			default:;
			}
			JFontStyle style = pDC->GetFontStyle();
			style.bold = kTrue;
			pDC->SetFontStyle(style);
			::DrawClippedStringUTF8(pDC, mOriginalItems[index], JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		}
		break;

	default:;
	}

}

// Click in the cell
void CReplyChooseTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	// Get data
	SReplyElement& elem = mStorage.at(inCell.row - 1);
	EReplyTo new_reply_type;

	switch(inCell.col)
	{

	case 1:
		new_reply_type = replyTo;
		break;

	case 2:
		new_reply_type = replyCC;
		break;

	case 3:
		new_reply_type = replyBCC;
		break;

	default:
		// Do nothing
		return;
	}

	// Toggle off if clicking current selection
	if (new_reply_type == elem.reply_type)
		new_reply_type = replyNone;

	// Change
	elem.reply_type = new_reply_type;
	RefreshRow(inCell.row);
}

// Set message to reply to (called by msg window)
void CReplyChooseTable::SetMessage(CMessage* aMsg)
{
	mItsMsg = aMsg;

	CEnvelope* env = mItsMsg->GetEnvelope();

	// Add reply to
	AddAddrList(env->GetReplyTo(), replyReplyTo, replyTo);

	// Add from (use from in the reply if the original reply-to is empty)
	mUseFromInsteadOfReplyto = (env->GetReplyTo()->size() == 0);
	AddAddrList(env->GetFrom(), replyFrom, mUseFromInsteadOfReplyto ? replyTo : replyNone);

	// Add sender
	AddAddrList(env->GetSender(), replySender, replyNone);

	// Add to
	AddAddrList(env->GetTo(), replyTo, replyTo);

	// Add cc
	AddAddrList(env->GetCC(), replyCC, replyCC);

}

// Set message to reply to (called by msg window)
void CReplyChooseTable::SetMessageList(CMessageList* msgs)
{
	// Add reply to
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetReplyTo(), replyReplyTo, replyTo);
		
	// Add from (use from in the reply if the original reply-to is empty)
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		mUseFromInsteadOfReplyto = ((*iter)->GetEnvelope()->GetReplyTo()->size() == 0);
		AddAddrList((*iter)->GetEnvelope()->GetFrom(), replyFrom, mUseFromInsteadOfReplyto ? replyTo : replyNone);
	}

	// Add sender
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetSender(), replySender, replyNone);
		
	// Add to
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetTo(), replyTo, replyTo);
		
	// Add cc
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetCC(), replyCC, replyCC);
		
}

// Set message to reply to (called by Adbk)
void CReplyChooseTable::SetList(CAddressList* aList)
{
	mItsList = aList;

	// Add reply to
	AddAddrList(mItsList, replyNone, replyTo);

}

// Add items from an address list to the table
void CReplyChooseTable::AddAddrList(CAddressList* aList,
									EReplyTo original_type,
									EReplyTo reply_type)
{
	// Count inserted items
	int count = 0;

	// Loop over all addresses in input list
	for(CAddressList::iterator iter1 = aList->begin(); iter1 != aList->end(); iter1++)
	{
		bool include;
		include = true;

		// Loop over all addresses in reply list
		for(CReplyList::const_iterator iter2 = mStorage.begin(); iter2 != mStorage.end(); iter2++)
		{
			// Compare with existing
			if (**iter1 == (*iter2).reply_addr)
			{
				include = false;
				break;
			}
		}

		// Add it if required
		if (include)
		{
			// Set reply info
			SReplyElement elem;
			elem.original_type = original_type;
			elem.reply_type = reply_type;

			// Always force smart address into off state
			if (CPreferences::TestSmartAddress(**iter1))
				elem.reply_type = replyNone;

			elem.reply_addr = **iter1;
			mStorage.push_back(elem);
			count++;

		}
	}

	// Add rows to table if data added to storage
	if (count)
		InsertRows(count, 1);
}

// Get selected lists
void CReplyChooseTable::GetAddressLists(CAddressList** reply_to,
									CAddressList** reply_cc,
									CAddressList** reply_bcc)
{
	*reply_to = NULL;
	*reply_cc = NULL;
	*reply_bcc = NULL;

	// Add all addresses in row
	for(CReplyList::const_iterator iter = mStorage.begin(); iter != mStorage.end(); iter++)
	{
		const SReplyElement& elem = *iter;
		switch(elem.reply_type)
		{

		case replyTo:
			// Create list if required
			if (!*reply_to)
				*reply_to = new CAddressList();

			// Add to list
			(*reply_to)->push_back(new CAddress(elem.reply_addr));
			break;

		case replyCC:
			// Create list if required
			if (!*reply_cc)
				*reply_cc = new CAddressList();

			// Add to list
			(*reply_cc)->push_back(new CAddress(elem.reply_addr));
			break;

		case replyBCC:
			// Create list if required
			if (!*reply_bcc)
				*reply_bcc = new CAddressList();

			// Copy and add to list
			(*reply_bcc)->push_back(new CAddress(elem.reply_addr));
			break;

		default:
			// ignore
			break;
		}
	}

}

void CReplyChooseTable::ChangeSelection(EReplyTo select)
{
	// Iterator over each element
	// Add all addresses in row
	for(CReplyList::iterator iter = mStorage.begin(); iter != mStorage.end(); iter++)
	{
		SReplyElement& elem = *iter;

		// Check for smart address
		bool smart = CPreferences::TestSmartAddress(elem.reply_addr);

		switch(select)
		{
		case replyReplyTo:
			elem.reply_type = (elem.original_type == (mUseFromInsteadOfReplyto ? replyFrom : replyReplyTo)) && !smart ? replyTo : replyNone;
			break;
		case replyAll:
			switch(elem.original_type)
			{
			case replyFrom:
				elem.reply_type = (smart || !mUseFromInsteadOfReplyto) ? replyNone : replyTo;
				break;
			case replyReplyTo:
				elem.reply_type = smart ? replyNone : replyTo;
				break;
			case replyTo:
				elem.reply_type = smart ? replyNone : replyTo;
				break;
			case replyCC:
				elem.reply_type = smart ? replyNone : replyCC;
				break;
			default:
				elem.reply_type = replyNone;
			}
			break;
		case replyFrom:
			elem.reply_type = (elem.original_type == replyFrom) && !smart ? replyTo : replyNone;
			break;
		case replySender:
			elem.reply_type = (elem.original_type == replySender) && !smart ? replyTo : replyNone;
			break;
		case replyNone:
			elem.reply_type = replyNone;
			break;
		default:;
		}
	}
	
	// Force redraw
	Refresh();
}

void CReplyChooseTable::ChangeAddressSelection(EReplyTo select)
{
	// Iterator over each element
	for(CReplyList::iterator iter = mStorage.begin(); iter != mStorage.end(); iter++)
	{
		SReplyElement& elem = *iter;
		elem.reply_type = select;
	}
	
	// Force redraw
	Refresh();
}
