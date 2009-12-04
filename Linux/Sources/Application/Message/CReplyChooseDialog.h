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

#include "CAddress.h"
#include "CDialogDirector.h"
#include "CTable.h"
#include "CMessageWindow.h"
#include "CMIMETypes.h"
#include <vector>

#include "HPopupMenu.h"

// Constants

// Classes

class CAddressList;
class CMessage;
class CMessageList;
class cdstring;

struct SReplyElement
{
	EReplyTo	original_type;
	CAddress	reply_addr;
	EReplyTo	reply_type;
};

class JXImage;
typedef std::vector<SReplyElement> CReplyList;

class CReplyChooseTable : public CTable
{
	enum
	{
		eDiamond = 0,
		eTicked = 1
	};

	enum
	{
		eOriginal_ReplyTo = 0,
		eOriginal_From,
		eOriginal_Sender,
		eOriginal_To,
		eOriginal_CC
	};

public:
	CReplyChooseTable(JXScrollbarSet* scrollbarSet,
						 JXContainer* enclosure,
						 const HSizingOption hSizing,
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h);
	virtual ~CReplyChooseTable();

			void	InitTable(bool originals);
			void	SetMessage(CMessage* aMsg);
			void	SetMessageList(CMessageList* msgs);
			void	SetList(CAddressList* aList);				// Set addresses to show

			void	GetAddressLists(CAddressList** reply_to,
									CAddressList** reply_cc,
									CAddressList** reply_bcc);	// Get selected lists

			void	ChangeSelection(EReplyTo select);
			void	ChangeAddressSelection(EReplyTo select);

	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,
								const JRect& inLocalRect);				// Draw the items

protected:
	virtual	void	LClickCell(const STableCell& inCell, 
			   					const JXKeyModifiers& modifiers);		// Clicked somewhere

private:
	CMessage*		mItsMsg;
	CAddressList*	mItsList;
	CReplyList		mStorage;
	cdstrvect		mOriginalItems;
	bool			mUseFromInsteadOfReplyto;

			void	AddAddrList(CAddressList* aList,
								EReplyTo original_type,
								EReplyTo reply_type);
};

/////////////////////////////////////////////////////////////////////////////
// CReplyChooseDialog dialog

class CLetterWindow;
class CSimpleTitleTable;
class JXRadioGroup;
class JXTextButton;
class JXTextRadioButton;

class CReplyChooseDialog : public CDialogDirector
{
// Construction
public:
	CReplyChooseDialog(JXDirector* supervisor, bool reply);
	~CReplyChooseDialog();   					// standard destructor


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

// Implementation
protected:
// begin JXLayout1

    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mCreateNewBtn;
    JXTextButton*      mReplyToBtn;
    JXTextButton*      mReplyAllBtn;
    JXTextButton*      mReplyFromBtn;
    JXTextButton*      mReplySenderBtn;
    JXTextButton*      mReplyNoneBtn;
    JXRadioGroup*      mQuote;
    JXTextRadioButton* mQuoteSelection;
    JXTextRadioButton* mQuoteAll;
    JXTextRadioButton* mQuoteNone;

// end JXLayout1
// begin JXLayout2

    JXTextButton* mOKBtn2;
    JXTextButton* mCancelBtn2;
    JXTextButton* mAddressToBtn;
    JXTextButton* mAddressCcBtn;
    JXTextButton* mAddressBccBtn;
    JXTextButton* mAddressNoneBtn;
    HPopupMenu*   mAddToDraft;

// end JXLayout2
	CSimpleTitleTable*	mTitles;
	CReplyChooseTable*	mItsTable;
	bool mIsReply;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void OnCreateNewBtn();
	void OnSelectReplyToBtn();
	void OnSelectReplyAllBtn();
	void OnSelectReplyFromBtn();
	void OnSelectReplySenderBtn();
	void OnSelectReplyNoneBtn();
	void OnSelectAddressToBtn();
	void OnSelectAddressCcBtn();
	void OnSelectAddressBccBtn();
	void OnSelectAddressNoneBtn();

	void SetMessage(CMessage* aMsg)
		{ mItsMsg = aMsg; }
	void SetMessageList(CMessageList* msgs)
		{ mMsgs = msgs; }
	void SetReplyQuote(EReplyQuote& quote, bool has_selection);
	void GetReplyQuote(EReplyQuote& quote);
				
	void SetList(CAddressList* aList)
		{ mItsList = aList; }

	void GetAddToWindow(CLetterWindow*& add_to);		// Get selected add to window

private:
	enum EAddToDraft
	{
		eAddTo_New = 1,
		//eAddTo_Separator,
		eAddTo_First
	};

	CMessage* mItsMsg;
	CMessageList* mMsgs;
	CAddressList* mItsList;
	
	void InitAddToDraft();
};

#endif
