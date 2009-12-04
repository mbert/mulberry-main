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

#include "CHelpDialog.h"

#include "CSimpleTitleTable.h"
#include "CTable.h"

#include "CAddress.h"
#include "CMessageWindow.h"
#include "CPopupButton.h"

// Constants

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

typedef vector<SReplyElement> CReplyList;

class CReplyChooseTable : public CTable
{
	enum
	{
		eOriginal_ReplyTo = 0,
		eOriginal_From,
		eOriginal_Sender,
		eOriginal_To,
		eOriginal_CC
	};

public:
					CReplyChooseTable();
	virtual 		~CReplyChooseTable();

	virtual	void	InitTable(bool originals);
	virtual	void	SetMessage(CMessage* aMsg);
	virtual	void	SetMessageList(CMessageList* msgs);

	virtual void	SetList(CAddressList* aList);				// Set addresses to show

	virtual void	GetAddressLists(CAddressList** reply_to,
									CAddressList** reply_cc,
									CAddressList** reply_bcc);	// Get selected lists

	virtual void	ChangeSelection(EReplyTo select);
	virtual void	ChangeAddressSelection(EReplyTo select);

protected:
	virtual	void	LClickCell(const STableCell& inCell, UINT nFlags);				// Clicked somewhere

private:
	CMessage*		mItsMsg;
	CAddressList*	mItsList;
	CReplyList		mStorage;
	cdstrvect		mOriginalItems;

	virtual void	AddAddrList(CAddressList* aList,
								EReplyTo original_type,
								EReplyTo reply_type);
	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CReplyChooseDialog dialog

class CLetterWindow;

class CReplyChooseDialog : public CHelpDialog
{
// Construction
public:
	CReplyChooseDialog(UINT nID, CWnd* pParent = NULL);   // standard constructor
	~CReplyChooseDialog();   					// standard destructor

// Dialog Data
	//{{AFX_DATA(CReplyChooseDialog)
	CSimpleTitleTable mTitles;
	CReplyChooseTable mTable;
	//}}AFX_DATA

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

			void	SetMessage(CMessage* aMsg)
						{ mItsMsg = aMsg; }
			void	SetMessageList(CMessageList* msgs)
						{ mMsgs = msgs; }
			void	SetReplyQuote(EReplyQuote& quote, bool has_selection);
			void	GetReplyQuote(EReplyQuote& quote);
						
			void	SetList(CAddressList* aList)
						{ mItsList = aList; }

	CReplyChooseTable* GetTable()
			{ return &mTable; }

			void	GetAddToWindow(CLetterWindow*& add_to);		// Get selected add to window

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplyChooseDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	enum EAddToDraft
	{
		eAddTo_New = 0,
		eAddTo_Separator,
		eAddTo_First
	};

	CMessage* mItsMsg;
	CMessageList* mMsgs;
	CAddressList* mItsList;
	CPopupButton mAddToDraft;
	int		mQuote;
	bool	mHasSelection;
	
	void InitAddToDraft();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplyChooseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnCreateNewBtn();
	afx_msg void OnSelectReplyToBtn();
	afx_msg void OnSelectReplyAllBtn();
	afx_msg void OnSelectReplyFromBtn();
	afx_msg void OnSelectReplySenderBtn();
	afx_msg void OnSelectReplyNoneBtn();
	afx_msg void OnSelectAddressToBtn();
	afx_msg void OnSelectAddressCcBtn();
	afx_msg void OnSelectAddressBccBtn();
	afx_msg void OnSelectAddressNoneBtn();
	afx_msg void OnChangeAddTo(UINT nID);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
