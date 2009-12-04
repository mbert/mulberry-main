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


// Header for CReplyChooseDialog class

#ifndef __CREPLYCHOOSEDIALOG__MULBERRY__
#define __CREPLYCHOOSEDIALOG__MULBERRY__

#include <LDialogBox.h>
#include "CTableDrag.h"

#include "CAddress.h"
#include "CMessageWindow.h"
#include "CMIMETypes.h"

// Constants

// Panes
const	PaneIDT		paneid_ReplyChooseDialog = 2010;
const	PaneIDT		paneid_AddressChooseDialog = 9010;
const	PaneIDT		paneid_ReplyChooseTable = 'TABL';
const	PaneIDT		paneid_ReplyChooseQuoteSelection = 'QSEL';
const	PaneIDT		paneid_ReplyChooseQuoteAll = 'QALL';
const	PaneIDT		paneid_ReplyChooseQuoteNone = 'QNON';
const	PaneIDT		paneid_ReplyChooseAddToDraft = 'ADD2';

// Resources
const	ResIDT		RidL_CReplyChooseDialogBtns = 2010;
const	ResIDT		RidL_CAddressChooseDialogBtns = 9010;
const	ResIDT		STRx_ReplyChooseHelp = 2011;

// Messages
const MessageT		msg_CreateNewBtn = 'NEWB';
const MessageT		msg_SelectReplyToBtn = 'SRPL';
const MessageT		msg_SelectReplyAllBtn = 'SALL';
const MessageT		msg_SelectReplyFromBtn = 'SFRM';
const MessageT		msg_SelectReplySenderBtn = 'SSND';
const MessageT		msg_SelectReplyNoneBtn = 'SNON';
const MessageT		msg_SelectAddressAllToBtn = 'ATO ';
const MessageT		msg_SelectAddressAllCcBtn = 'ACC ';
const MessageT		msg_SelectAddressAllBccBtn = 'ABCC';
const MessageT		msg_SelectAddressAllNoneBtn = 'ANON';

// Classes

class CAddressList;
class CMessage;
class CMessageList;

struct SReplyElement
{
	EReplyTo	original_type;
	CAddress	reply_addr;
	EReplyTo	reply_type;
};

typedef std::vector<SReplyElement> CReplyList;

class	CReplyChooseTable : public CTableDrag {

public:
	enum { class_ID = 'RTAB' };

					CReplyChooseTable(LStream *inStream);
	virtual 		~CReplyChooseTable();

	void	InitTable(bool originals);

	void	SetMessage(CMessage* aMsg);
	void	SetMessageList(CMessageList* msgs);
	void	SetList(CAddressList* aList);			// Set addresses to show
						
	void	GetAddressLists(CAddressList** reply_to,
									CAddressList** reply_cc,
									CAddressList** reply_bcc);	// Get selected lists

	void	ChangeSelection(EReplyTo select);
	void	ChangeAddressSelection(EReplyTo select);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);

	virtual void	ClickCell(const STableCell &inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell

private:
	CMessage*		mItsMsg;
	CAddressList*	mItsList;
	CReplyList		mStorage;
	TextTraitsH		mTextTraits;
	
	void	AddAddrList(CAddressList* aList,
								EReplyTo original_type,
								EReplyTo reply_type);
};

class LPopupButton;
class LRadioButton;
class CLetterWindow;

class	CReplyChooseDialog : public LDialogBox {

public:
	enum { class_ID = 'REPL' };

					CReplyChooseDialog();
					CReplyChooseDialog(LStream *inStream);
	virtual 		~CReplyChooseDialog();

	static	void	ProcessChoice(CMessage* aMsg, const cdstring& quotetxt, EContentSubType subtype, bool is_flowed, EReplyTo reply_to, bool use_dialog);
	static	void	ProcessChoice(CMessageList* msgs, EReplyTo reply_to, bool use_dialog);
	static	void	ProcessChoice(CAddressList* aList, bool use_dialog);

	static  long	PoseDialog(CMessage* aMsg,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								EReplyQuote& quote,
								bool has_selection);
	static  long	PoseDialog(CMessageList* msgs,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								EReplyQuote& quote);
	static  long	PoseDialog(CAddressList* aList,
								CAddressList** reply_to,
								CAddressList** reply_cc,
								CAddressList** reply_bcc,
								CLetterWindow*& add_to);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

			void	SetMessage(CMessage* aMsg)				// Set message to reply to
						{ mItsTable->SetMessage(aMsg); }
			void	SetMessageList(CMessageList* msgs)
						{ mItsTable->SetMessageList(msgs); }
			void	SetReplyQuote(EReplyQuote& quote, bool has_selection);
			void	GetReplyQuote(EReplyQuote& quote);
						
			void	SetList(CAddressList* aList)			// Set addresses to show
						{ mItsTable->SetList(aList); }
						
			void	GetAddressLists(CAddressList** reply_to,
									CAddressList** reply_cc,
									CAddressList** reply_bcc)	// Get selected lists
						{ mItsTable->GetAddressLists(reply_to, reply_cc, reply_bcc); }

			void	GetAddToWindow(CLetterWindow*& add_to);		// Get selected add to window
	
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	enum EAddToDraft
	{
		eAddTo_New = 1,
		eAddTo_Separator,
		eAddTo_First
	};

	CReplyChooseTable*	mItsTable;
	LRadioButton*		mQuoteSelection;
	LRadioButton*		mQuoteAll;
	LRadioButton*		mQuoteNone;
	LPopupButton*		mAddToDraft;
	
			void		InitAddToDraft();
};

#endif
