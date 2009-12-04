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
#include "CDrawUtils.h"
#include "CEnvelope.h"
#include "CIconLoader.h"
#include "CLetterDoc.h"
#include "CLetterWindow.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

#include <WIN_LTableMultiGeometry.h>

/////////////////////////////////////////////////////////////////////////////
// CReplyChooseDialog dialog


CReplyChooseDialog::CReplyChooseDialog(UINT nID, CWnd* pParent /*=NULL*/)
	: CHelpDialog(nID, pParent)
{
	//{{AFX_DATA_INIT(CReplyChooseDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	mItsMsg = NULL;
	mMsgs = NULL;
	mItsList = NULL;
}

CReplyChooseDialog::~CReplyChooseDialog()
{
	// Must unsubclass table
	mTable.Detach();
	mTitles.Detach();
}


void CReplyChooseDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplyChooseDialog)
	if (!mItsList)
		DDX_Radio(pDX, IDC_REPLYCHOOSE_QUOTESELECTION, mQuote);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReplyChooseDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CReplyChooseDialog)
	ON_COMMAND(IDC_REPLYCHOOSE_CREATENEW, OnCreateNewBtn)
	ON_COMMAND(IDC_REPLYCHOOSE_SELECTREPLYTO, OnSelectReplyToBtn)
	ON_COMMAND(IDC_REPLYCHOOSE_SELECTREPLYALL, OnSelectReplyAllBtn)
	ON_COMMAND(IDC_REPLYCHOOSE_SELECTREPLYFROM, OnSelectReplyFromBtn)
	ON_COMMAND(IDC_REPLYCHOOSE_SELECTREPLYSENDER, OnSelectReplySenderBtn)
	ON_COMMAND(IDC_REPLYCHOOSE_SELECTREPLYNONE, OnSelectReplyNoneBtn)
	ON_COMMAND(IDC_ADDRESSCHOICE_SELECTTO, OnSelectAddressToBtn)
	ON_COMMAND(IDC_ADDRESSCHOICE_SELECTCC, OnSelectAddressCcBtn)
	ON_COMMAND(IDC_ADDRESSCHOICE_SELECTBCC, OnSelectAddressBccBtn)
	ON_COMMAND(IDC_ADDRESSCHOICE_SELECTNONE, OnSelectAddressNoneBtn)
	ON_COMMAND_RANGE(IDM_AddressAddTo_NewDraft, IDM_AddressAddTo_End, OnChangeAddTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplyChooseDialog message handlers

BOOL CReplyChooseDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mTable.SubclassDlgItem(IDC_ADDRESSCHOICE, this);
	if (mItsMsg)
	{
		mTable.InitTable(true);
		mTable.SetMessage(mItsMsg);
	}
	else if (mMsgs)
	{
		mTable.InitTable(true);
		mTable.SetMessageList(mMsgs);
	}
	else if (mItsList)
	{
		mTable.InitTable(false);
		mTable.SetList(mItsList);
	}

	// Subclass titles
	mTitles.SubclassDlgItem(IDC_ADDRESSCHOICETITLES, this);
	mTitles.SyncTable(&mTable, true);

	mTitles.LoadTitles("UI::Titles::ReplyChoose", 5);

	// Set quote options
	if (!mHasSelection && !mItsList)
		GetDlgItem(IDC_REPLYCHOOSE_QUOTESELECTION)->EnableWindow(false);

	// Init the add to popup - only present if using the address version of the dialog
	InitAddToDraft();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReplyChooseDialog::SetReplyQuote(EReplyQuote& quote, bool has_selection)
{
	// Set it up
	if (quote == eQuoteSelection)
		mQuote = (has_selection ? eQuoteSelection : eQuoteAll);
	else
		mQuote = quote;

	// Disable selection option if none available
	mHasSelection = has_selection;

}

void CReplyChooseDialog::GetReplyQuote(EReplyQuote& quote)
{
	// Get the proper setting
	quote = (EReplyQuote) mQuote;
}

void CReplyChooseDialog::InitAddToDraft()
{
	// Only if it exists
	if (!GetDlgItem(IDC_REPLYCHOOSE_ADDTOPOPUP))
		return;

	// Try to get add to draft popup
	mAddToDraft.SubclassDlgItem(IDC_REPLYCHOOSE_ADDTOPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAddToDraft.SetMenu(IDR_POPUP_ADDRESS_ADDTO);

	// Delete previous items
	CMenu* pPopup = mAddToDraft.GetPopupMenu();
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = num_menu - 1; i > eAddTo_Separator; i--)
		pPopup->RemoveMenu(i, MF_BYPOSITION);

	// Now add titles of any open drafts
	{
		int menu_id = IDM_AddressAddTo_NewDraft + eAddTo_First;
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
		{
			cdstring title((*iter)->GetDocument()->GetTitle());
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, title);
		}
	}

	// Set to first item
	mAddToDraft.SetValue(IDM_AddressAddTo_NewDraft);
}

// Get selected add to window
void CReplyChooseDialog::GetAddToWindow(CLetterWindow*& add_to)
{
	// Start off NULL
	add_to = NULL;

	// Get current value
	short value = mAddToDraft.GetValue() - IDM_AddressAddTo_NewDraft;
	
	// If its a new draft just return - or iif index is higher than window count
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		if ((value == eAddTo_New) || (value - eAddTo_First >= CLetterWindow::sLetterWindows->size()))
			return;
		
		// Just get the corresponding window
		add_to = CLetterWindow::sLetterWindows->at(value - eAddTo_First);
	}
}

void CReplyChooseDialog::OnChangeAddTo(UINT nID) 
{
	mAddToDraft.SetValue(nID);
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
	CReplyChooseDialog dlog(IDD_REPLYCHOOSE, CSDIFrame::GetAppTopWindow());
	dlog.SetMessage(aMsg);
	dlog.SetReplyQuote(quote, has_selection);

	// Let DialogHandler process events
	UINT result = dlog.DoModal();
	switch(result)
	{
	case IDOK:
	case IDC_REPLYCHOOSE_CREATENEW:
	{
		dlog.GetTable()->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog.GetReplyQuote(quote);
		return (result == IDOK) ? 1 : 2;
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
	CReplyChooseDialog dlog(IDD_REPLYCHOOSE, CSDIFrame::GetAppTopWindow());
	dlog.SetMessageList(msgs);
	dlog.SetReplyQuote(quote, false);

	// Let DialogHandler process events
	UINT result = dlog.DoModal();
	switch(result)
	{
	case IDOK:
	case IDC_REPLYCHOOSE_CREATENEW:
	{
		dlog.GetTable()->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog.GetReplyQuote(quote);
		return (result == IDOK) ? 1 : 2;
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
	CReplyChooseDialog dlog(IDD_ADDRESSCHOOSE, CSDIFrame::GetAppTopWindow());
	dlog.SetList(aList);

	// Let DialogHandler process events
	UINT result = dlog.DoModal();
	switch(result)
	{
	case IDOK:
	case IDC_REPLYCHOOSE_CREATENEW:
	{
		dlog.GetTable()->GetAddressLists(reply_to, reply_cc, reply_bcc);
		dlog.GetAddToWindow(add_to);
		return (result == IDOK) ? 1 : 2;
	}
	default:
		return 0;
	}
}

void CReplyChooseDialog::OnCreateNewBtn()
{
	EndDialog(IDC_REPLYCHOOSE_CREATENEW);
}

void CReplyChooseDialog::OnSelectReplyToBtn()
{
	GetTable()->ChangeSelection(replyReplyTo);
}

void CReplyChooseDialog::OnSelectReplyAllBtn()
{
	GetTable()->ChangeSelection(replyAll);
}

void CReplyChooseDialog::OnSelectReplyFromBtn()
{
	GetTable()->ChangeSelection(replyFrom);
}

void CReplyChooseDialog::OnSelectReplySenderBtn()
{
	GetTable()->ChangeSelection(replySender);
}

void CReplyChooseDialog::OnSelectReplyNoneBtn()
{
	GetTable()->ChangeSelection(replyNone);
}

void CReplyChooseDialog::OnSelectAddressToBtn()
{
	GetTable()->ChangeAddressSelection(replyTo);
}

void CReplyChooseDialog::OnSelectAddressCcBtn()
{
	GetTable()->ChangeAddressSelection(replyCC);
}

void CReplyChooseDialog::OnSelectAddressBccBtn()
{
	GetTable()->ChangeAddressSelection(replyBCC);
}

void CReplyChooseDialog::OnSelectAddressNoneBtn()
{
	GetTable()->ChangeAddressSelection(replyNone);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CReplyChooseTable::CReplyChooseTable()
{
	mItsMsg = NULL;
	mItsList = NULL;
	
	mTableGeometry = new LTableMultiGeometry(this, 128, 16);
}

// Default destructor
CReplyChooseTable::~CReplyChooseTable()
{
}

BEGIN_MESSAGE_MAP(CReplyChooseTable, CTable)
	//{{AFX_MSG_MAP(CReplyChooseTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CReplyChooseTable::InitTable(bool originals)
{
	// Create columns
	CRect frame;
	GetClientRect(frame);

	InsertCols(originals ? 5 : 4, 1);
	SetColWidth(32, 1, 1);
	SetColWidth(32, 2, 2);
	SetColWidth(36, 3, 3);
	if (originals)
	{
		SetColWidth(frame.Width() - 16 - 200, 4, 4);
		SetColWidth(100, 5, 5);
	}
	else
		SetColWidth(frame.Width() - 16 - 100, 4, 4);

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
void CReplyChooseTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

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
			CIconLoader::DrawIcon(pDC, inLocalRect.left + 6, inLocalRect.top, IDI_DIAMONDTICKED, 16);
		else
			CIconLoader::DrawIcon(pDC, inLocalRect.left + 6, inLocalRect.top, IDI_DIAMOND, 16);
		break;

	case 4:
		// Write address
		::DrawClippedStringUTF8(pDC, elem.reply_addr.GetFullAddress(), CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
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
			}
			pDC->SelectObject(CMulberryApp::sAppFontBold);
			::DrawClippedStringUTF8(pDC, mOriginalItems[index], CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		}
		break;

	default:
		break;
	}

}

// Click in the cell
void CReplyChooseTable::LClickCell(const STableCell& inCell, UINT nFlags)
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
		}
	}
	
	// Force redraw
	RedrawWindow();
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
	RedrawWindow();
}
