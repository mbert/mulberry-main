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
#include "CBalloonDialog.h"
#include "CEnvelope.h"
#include "CHelpAttach.h"
#include "CLetterDoc.h"
#include "CLetterWindow.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"

#include "MyCFString.h"

#include <LPopupButton.h>
#include <LRadioButton.h>
#include <LTableMultiGeometry.h>


// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CReplyChooseDialog::CReplyChooseDialog()
{
	mItsTable = NULL;
	mAddToDraft = NULL;
}

// Constructor from stream
CReplyChooseDialog::CReplyChooseDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mItsTable = NULL;
	mAddToDraft = NULL;
}

// Default destructor
CReplyChooseDialog::~CReplyChooseDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CReplyChooseDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Info
	mItsTable = (CReplyChooseTable*) FindPaneByID(paneid_ReplyChooseTable);

	// Don't get the buttons until we know now that this is a reply not an address operation
	mQuoteSelection = NULL;
	mQuoteAll = NULL;
	mQuoteNone = NULL;

	// Init the add to popup - only present if using the address version of the dialog
	InitAddToDraft();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, (GetPaneID() == paneid_ReplyChooseDialog) ? RidL_CReplyChooseDialogBtns : RidL_CAddressChooseDialogBtns);
}

void CReplyChooseDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_SelectReplyToBtn:
		mItsTable->ChangeSelection(replyReplyTo);
		break;
	case msg_SelectReplyAllBtn:
		mItsTable->ChangeSelection(replyAll);
		break;
	case msg_SelectReplyFromBtn:
		mItsTable->ChangeSelection(replyFrom);
		break;
	case msg_SelectReplySenderBtn:
		mItsTable->ChangeSelection(replySender);
		break;
	case msg_SelectReplyNoneBtn:
		mItsTable->ChangeSelection(replyNone);
		break;
	case msg_SelectAddressAllToBtn:
		mItsTable->ChangeAddressSelection(replyTo);
		break;
	case msg_SelectAddressAllCcBtn:
		mItsTable->ChangeAddressSelection(replyCC);
		break;
	case msg_SelectAddressAllBccBtn:
		mItsTable->ChangeAddressSelection(replyBCC);
		break;
	case msg_SelectAddressAllNoneBtn:
		mItsTable->ChangeAddressSelection(replyNone);
		break;
	default:;
	}
}

void CReplyChooseDialog::SetReplyQuote(EReplyQuote& quote, bool has_selection)
{
	// Get the buttons at this point since we know now that this is a reply not an address operation
	mQuoteSelection = (LRadioButton*) FindPaneByID(paneid_ReplyChooseQuoteSelection);
	mQuoteAll = (LRadioButton*) FindPaneByID(paneid_ReplyChooseQuoteAll);
	mQuoteNone = (LRadioButton*) FindPaneByID(paneid_ReplyChooseQuoteNone);
	
	// Disable selection option if none available
	if (!has_selection)
		mQuoteSelection->Disable();

	// Set it up
	switch(quote)
	{
	case eQuoteSelection:
		(has_selection ? mQuoteSelection : mQuoteAll)->SetValue(1);
		break;
	case eQuoteAll:
		mQuoteAll->SetValue(1);
		break;
	case eQuoteNone:
		mQuoteNone->SetValue(1);
		break;
	}
}

void CReplyChooseDialog::GetReplyQuote(EReplyQuote& quote)
{
	// Get the proper setting
	if (mQuoteSelection->GetValue())
		quote = eQuoteSelection;
	else if (mQuoteAll->GetValue())
		quote = eQuoteAll;
	else if (mQuoteNone->GetValue())
		quote = eQuoteNone;
}

void CReplyChooseDialog::InitAddToDraft()
{
	// Try to get add to draft popup
	mAddToDraft = (LPopupButton*) FindPaneByID(paneid_ReplyChooseAddToDraft);
	if (!mAddToDraft)
		return;

	// Delete previous items
	MenuHandle menuH = mAddToDraft->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i > eAddTo_Separator; i--)
		::DeleteMenuItem(menuH, i);

	// Now add titles of any open drafts
	{
		short menu_pos = eAddTo_First;
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++, menu_pos++)
		{
			MyCFString origText((*iter)->CopyCFDescriptor());
			cdstring title = origText.GetString();
			::AppendItemToMenu(menuH, menu_pos, title);
		}
	}

	// Force max/min update
	mAddToDraft->SetMenuMinMax();
}

// Get selected add to window
void CReplyChooseDialog::GetAddToWindow(CLetterWindow*& add_to)
{
	// Start off NULL
	add_to = NULL;

	// Only if popup exists
	if (!mAddToDraft)
		return;
	
	// Get current value
	short value = mAddToDraft->GetValue();
	
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
	CLetterDoc* newDoc = NULL;
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
		newDoc = new CLetterDoc(CMulberryApp::sApp, NULL);
		CLetterWindow* newWindow = (CLetterWindow*) newDoc->GetWindow();

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

		delete newDoc;
		delete to_list;
		delete cc_list;
		delete bcc_list;
	}
}

void CReplyChooseDialog::ProcessChoice(CMessageList* msgs, EReplyTo reply_to, bool use_dialog)
{
	CLetterDoc* newDoc = NULL;
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
		newDoc = new CLetterDoc(CMulberryApp::sApp, NULL);
		CLetterWindow* newWindow = (CLetterWindow*) newDoc->GetWindow();

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
			newWindow->IncludeFromReply(false, false);

		// Show window
		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete newDoc;
		delete msgs;
		delete to_list;
		delete cc_list;
		delete bcc_list;
	}
}

void CReplyChooseDialog::ProcessChoice(CAddressList* aList, bool use_dialog)
{
	CLetterDoc* newDoc = NULL;
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
			newDoc = new CLetterDoc(CMulberryApp::sApp, NULL);
			CLetterWindow* newWindow = (CLetterWindow*) newDoc->GetWindow();

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

			delete newDoc;
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
	// Create the dialog
	CBalloonDialog theHandler(paneid_ReplyChooseDialog, CMulberryApp::sApp);

	CReplyChooseDialog* dlog = (CReplyChooseDialog*) theHandler.GetDialog();
	dlog->SetMessage(aMsg);
	dlog->SetReplyQuote(quote, has_selection);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		switch(hitMessage)
		{
		case msg_OK:
		case msg_CreateNewBtn:
			dlog->GetAddressLists(reply_to, reply_cc, reply_bcc);
			dlog->GetReplyQuote(quote);
			return (hitMessage == msg_OK) ? 1 : 2;
		case msg_Cancel:
			return 0;
		}
	}
}

long CReplyChooseDialog::PoseDialog(CMessageList* msgs,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								EReplyQuote& quote)
{
	// Create the dialog
	CBalloonDialog theHandler(paneid_ReplyChooseDialog, CMulberryApp::sApp);

	CReplyChooseDialog* dlog = (CReplyChooseDialog*) theHandler.GetDialog();
	dlog->SetMessageList(msgs);
	dlog->SetReplyQuote(quote, false);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		switch(hitMessage)
		{
		case msg_OK:
		case msg_CreateNewBtn:
			dlog->GetAddressLists(reply_to, reply_cc, reply_bcc);
			dlog->GetReplyQuote(quote);
			return (hitMessage == msg_OK) ? 1 : 2;
		case msg_Cancel:
			return 0;
		}
	}
}

long CReplyChooseDialog::PoseDialog(CAddressList* aList,
								CAddressList** to,
								CAddressList** cc,
								CAddressList** bcc,
								CLetterWindow*& add_to)
{
	// Create the dialog
	CBalloonDialog theHandler(paneid_AddressChooseDialog, CMulberryApp::sApp);

	CReplyChooseDialog* dlog = (CReplyChooseDialog*) theHandler.GetDialog();
	dlog->SetList(aList);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		switch(hitMessage)
		{
		case msg_OK:
		case msg_CreateNewBtn:
			dlog->GetAddressLists(to, cc, bcc);
			dlog->GetAddToWindow(add_to);
			return (hitMessage == msg_OK) ? 1 : 2;
		case msg_Cancel:
			return 0;
		}
	}
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CReplyChooseTable::CReplyChooseTable(LStream *inStream)
		: CTableDrag(inStream)
{
	mItsMsg = NULL;
	mItsList = NULL;
	mTextTraits = NULL;
	mTableGeometry = new LTableMultiGeometry(this, mFrameSize.width, 16);
}

// Default destructor
CReplyChooseTable::~CReplyChooseTable()
{
	// Forget traits
	DISPOSE_HANDLE(mTextTraits);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CReplyChooseTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Get text traits resource - detatch & lock
	mTextTraits = UTextTraits::LoadTextTraits(261);
	ThrowIfResFail_(mTextTraits);
	::DetachResource((Handle) mTextTraits);
	::MoveHHi((Handle) mTextTraits);
	::HLock((Handle) mTextTraits);

	// No drag and drop unless exlicitly specified
	SetAllowDrag(false);
}

void CReplyChooseTable::InitTable(bool originals)
{
	// Create columns
	SDimension16 frame;
	GetFrameSize(frame);

	InsertCols(originals ? 5 : 4, 1, NULL, 0, false);
	SetColWidth(20, 1, 1);
	SetColWidth(20, 2, 2);
	SetColWidth(20, 3, 3);
	if (originals)
	{
		SetColWidth(230, 4, 4);
		SetColWidth(frame.width - 290, 5, 5);
	}
	else
		SetColWidth(frame.width - 60, 4, 4);
}

// Draw a cell
void CReplyChooseTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(*mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	// Get data
	const SReplyElement& elem = mStorage.at(inCell.row - 1);

	switch(inCell.col)
	{

	case 1:
	case 2:
	case 3:
		Rect	iconRect;
		iconRect.left = inLocalRect.left + 3;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom;
		iconRect.top = iconRect.bottom - 16;
		// Check for tick
		if (((inCell.col==1) && (elem.reply_type == replyTo)) ||
			((inCell.col==2) && (elem.reply_type == replyCC)) ||
			((inCell.col==3) && (elem.reply_type == replyBCC)))
			::Ploticns(&iconRect, atNone, ttNone, ICNx_DiamondTicked);
		else
			::Ploticns(&iconRect, atNone, ttNone, ICNx_Diamond);
		break;

	case 4:
		{
			// Write address
			cdstring str(elem.reply_addr.GetFullAddress());
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 4);
			::DrawClippedStringUTF8(str, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	case 5:
		if (elem.original_type != replyNone)
		{
			cdstring theTxt;
			switch(elem.original_type)
			{
			case replyReplyTo:
				theTxt.FromResource("UI::ReplyChoose::ReplyTo");
				break;

			case replyFrom:
				theTxt.FromResource("UI::ReplyChoose::From");
				break;

			case replySender:
				theTxt.FromResource("UI::ReplyChoose::Sender");
				break;

			case replyTo:
				theTxt.FromResource("UI::ReplyChoose::To");
				break;

			case replyCC:
				theTxt.FromResource("UI::ReplyChoose::Cc");
				break;
			default:;
			}
			::TextFace(bold);
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 4);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	default:
		break;
	}

}

// Click in the cell
void CReplyChooseTable::ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
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

	if (new_reply_type != elem.reply_type)
	{
		// Force refresh of previous tick
		if (elem.reply_type != replyNone)
		{
			STableCell refresh(inCell);
			switch(elem.reply_type)
			{
			case replyTo:
				refresh.col = 1;
				break;

			case replyCC:
				refresh.col = 2;
				break;

			case replyBCC:
				refresh.col = 3;
				break;
			default:;
			}
			RefreshCell(refresh);
		}
	}
	else
		new_reply_type = replyNone;

	// Change
	elem.reply_type = new_reply_type;
	RefreshCell(inCell);
}

// Set message to reply to (called by msg window)
void CReplyChooseTable::SetMessage(CMessage* aMsg)
{
	mItsMsg = aMsg;

	CEnvelope* env = mItsMsg->GetEnvelope();
	ThrowIfNil_(env);

	// Init columns
	InitTable(true);

	// Add reply to
	AddAddrList(env->GetReplyTo(), replyReplyTo, replyTo);

	// Add from (use from in the reply if the original reply-to is empty)
	AddAddrList(env->GetFrom(), replyFrom, (env->GetReplyTo()->size() > 0) ? replyNone : replyTo);

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
	// Init columns
	InitTable(true);

	// Add reply to
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetReplyTo(), replyReplyTo, replyTo);

	// Add from (use from in the reply if the original reply-to is empty)
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		AddAddrList((*iter)->GetEnvelope()->GetFrom(), replyFrom, ((*iter)->GetEnvelope()->GetReplyTo()->size() > 0) ? replyNone : replyTo);

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

	// Init columns
	InitTable(false);

	// Add reply to
	AddAddrList(mItsList, replyNone, replyTo);
}

// Add items from an address list to the table
void CReplyChooseTable::AddAddrList(CAddressList* aList,
									EReplyTo original_type,
									EReplyTo reply_type)
{
	// Count inserted items
	SInt32 count = 0;

	// Loop over all addresses in input list
	for(CAddressList::iterator iter1 = aList->begin(); iter1 < aList->end(); iter1++)
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
		InsertRows(count, 1, NULL, 0, false);
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
			elem.reply_type = (elem.original_type == replyReplyTo) && !smart ? replyTo : replyNone;
			break;
		case replyAll:
			switch(elem.original_type)
			{
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
