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


// Header for CMessageWindow class

#ifndef __CMESSAGEWINDOW__MULBERRY__
#define __CMESSAGEWINDOW__MULBERRY__

#include "CWindowStatus.h"
#include "CCommander.h"

#include "CBodyTable.h"
#include "CCmdEdit.h"
#include "CDisplayFormatter.h"
#include "CFormattedTextDisplay.h"
#include "CGrayBackground.h"
#include "CMessageFwd.h"
#include "CMessageSimpleView.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CToolbarView.h"
#include "ETag.h"

#include "cdstring.h"
#include "cdustring.h"
#include "cdmutexprotect.h"

enum EReplyTo
{
	replyNone = 0,
	replyReplyTo,
	replySender,
	replyFrom,
	replyTo,
	replyCC,
	replyBCC,
	replyAll
};

enum EReplyQuote
{
	eQuoteSelection = 0,
	eQuoteAll,
	eQuoteNone
};

enum EForwardOptions
{
	eForwardQuote = 		1L << 0,
	eForwardHeaders =		1L << 1,
	eForwardAttachment =	1L << 2,
	eForwardRFC822 =		1L << 3
};

enum EQuoteDepth
{
	eQuoteDepth_All = IDM_QUOTEDEPTH_ALL,
	eQuoteDepth_No,
	eQuoteDepth_1,
	eQuoteDepth_2,
	eQuoteDepth_3,
	eQuoteDepth_4
};

// Classes
class CAddressList;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CMessageHeaderView;
class CTwister;
class CAttachment;
class CDisplayFormatter;

class CMessageWindow : public CWnd,
						public CCommander,
						public CWindowStatus
{

	DECLARE_DYNCREATE(CMessageWindow)

	friend class CMessageHeaderView;

	struct SHeaderState
	{
		bool	mExpanded;
		bool	mFromVisible;
		bool	mFromExpanded;
		bool	mToVisible;
		bool	mToExpanded;
		bool	mCcVisible;
		bool	mCcExpanded;
		bool	mSubjectVisible;
	};

public:
	typedef vector<CMessageWindow*>	CMessageWindowList;
	static cdmutexprotect<CMessageWindowList> sMsgWindows;	// List of windows (protected for multi-thread access)
	static CMultiDocTemplate* sMessageDocTemplate;

					CMessageWindow();
	virtual 		~CMessageWindow();
	static CMessageWindow* ManualCreate(bool hidden = false);
	static void		UpdateUIPos();

	virtual void	OnDraw(CDC* pDC) {}

	virtual CCmdEditView* GetText()
						{ return mText; }

	virtual CDocument* GetDocument()
						{ return mText->GetDocument(); }

	static  void	ServerState(bool logon);					// Server logs on/off

			void	SetTwistList(CCmdEdit* aField,
									CTwister* aTwister,
									CWnd* aMover,
									CAddressList* list);	// Add address list to caption/field
			void	SetMessage(CMessage* aMsg);				// Set the message
			CMessage*	GetMessage()						// Get its message
							{ return mItsMsg; }
			void	ClearMessage();							// Clear out the message
			void	SetMessageList(CMessageList* msgs);		// Set the message list
			CMessageList*	GetMessageList()				// Get its message
							{ return mMsgs; }
			void	PostSetMessage();						// Do post processing of message
			void	MessageChanged();						// Someone else changed this message

			const char* GetShowText()						// Get text showing
						{ return mShowText; }
	
			const CAttachment* GetCurrentPart() const
		{ return mCurrentPart; }
			EView GetViewAs() const							// Parse
				{ return mParsing;	}

			const unichar_t* GetSaveText();					// Get text to save
			void	ShowPart(CAttachment* attach);			// Show a particular part
			void	ShowSubMessage(CAttachment* attach);	// Display a sub-message
			void	ExtractPart(CAttachment* attach,		// Extract/view a particular part
									bool view);
			bool	ShowRawBody();							// Read in raw body

			void	AddPrintSummary();						// Temporarily add header summary for printing
			void	RemovePrintSummary();					// Remove temp header summary after printing

	static CMessageWindow*	FindWindow(const CMessage* aMsg,	// Find the corresponding window
										bool owned_by = false);
	static bool	WindowExists(const CMessageWindow* wnd);	// Check for window

	virtual void	ResetFont(CFont* list_font,
								CFont* display_font);		// Reset fonts
	virtual void	ResetState();							// Reset state from prefs

	// Common updaters
	afx_msg void	OnUpdateAlways(CCmdUI* pCmdUI);

	afx_msg void 	OnUpdateEditSpeak(CCmdUI* pCmdUI);

	afx_msg void	OnUpdateMessageReadPrev(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageReadNext(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageCopyNext(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageReject(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageSendAgain(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageCopy(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageDelete(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI);
			void	OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag);
	afx_msg void	OnUpdateMessageViewCurrent(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageShowHeader(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMenuExpandHeader(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateExpandHeader(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateWindowsShowParts(CCmdUI* pCmdUI);

	// Command handlers
	virtual bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void	OnFileNewDraft();
			void	DoNewLetter(bool option_key);		// Display a new draft
	afx_msg void	OnFilePrint();
	afx_msg void	OnFilePrintPreview();

	afx_msg void	OnEditSpeak();
			void	SpeakMessage();
	afx_msg void	OnEditSpellCheck();

	afx_msg void	OnMessageReadPrev();
	afx_msg void	OnMessageReadNext();
			void	ReadNextMessage(bool delete_it, bool copy_it, bool option_key);				// Read next message
	afx_msg void	OnMessageDeleteRead();
	afx_msg void	OnMessageCopyRead();
			void	CopyReadNextMessage(bool option_key);
			bool	RedisplayMessage(CMessage* theMsg);					// Display message in existing window
			void	DoRollover(CMbox* current_mbox);					// Attempt to rollover to next unseen

	afx_msg void	OnMessageReply();
	afx_msg void	OnMessageReplySender();
	afx_msg void	OnMessageReplyFrom();
	afx_msg void	OnMessageReplyAll();
			void	ReplyToThisMessage(EReplyTo reply_to, bool option_key);	// Reply to this message
	afx_msg void	OnMessageForward();
			void	ForwardThisMessage(bool option_key);
	afx_msg void	OnMessageBounce();
	afx_msg void	OnMessageReject();
	afx_msg void	OnMessageSendAgain();
	afx_msg void	OnMessageCopyBtn();									// Copy the message to chosen mailbox
	afx_msg void	OnMessageCopyCmd();									// Copy the message to chosen mailbox
	afx_msg void	OnMessageMoveCmd();									// Move the message to chosen mailbox
	afx_msg void	OnMessageCopyPopup(UINT nID);						// Copy to popup change
	afx_msg void	OnMessageCopy(UINT nID);							// Copy the message to another mailbox
			bool	TryCopyMessage(bool option_key);					// Try to copy a message
			bool	CopyThisMessage(CMbox* mbox, bool option_key);		// Copy to specific mailbox
	afx_msg void	OnMessageFlagsSeen();
	afx_msg void	OnMessageFlagsImportant();
	afx_msg void	OnMessageFlagsAnswered();
	afx_msg void	OnMessageFlagsDraft();
	afx_msg void	OnMessageFlagsLabel(UINT nID);
			void	OnMessageFlags(NMessage::EFlags flag);
	afx_msg void	OnMessageViewCurrent();
	afx_msg void	OnMessageDecodeBinHex();
	afx_msg void	OnMessageDecodeUU();
	afx_msg void	OnMessageVerifyDecrypt();
	afx_msg void	OnMessageDelete();
	afx_msg void	OnMessageShowHeader();

	afx_msg void	OnWindowsExpandHeader();

	afx_msg void	OnMessageFromTwister();
	afx_msg void	OnMessageToTwister();
	afx_msg void	OnMessageCCTwister();
	virtual	void	DoTwist(CWnd* aMover,
							CCmdEdit* aField,
							CTwister* aTwister);				// Handle twist

			void	LayoutHeader();								// Layout header component based on state

			void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
			void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

	afx_msg void	OnViewAs(EView view);
	afx_msg void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

	afx_msg void	OnIncreaseFont();					// Increase displayed font size
	afx_msg void	OnDecreaseFont();					// Decrease displayed font size

	afx_msg void	OnMessagePartsTwister();

	afx_msg void	OnMessagePartsFlat();
			void	DoPartsFlat(bool hierarchic);

	afx_msg void	OnSaveDefaultState();				// Save state in prefs
	afx_msg void	OnResetDefaultState();

	afx_msg void    OnTextFormatPopup(UINT nID);

	afx_msg void    OnQuoteDepthPopup(UINT nID);

private:
	CMessage*				mItsMsg;					// The message for this window
	bool					mWasUnseen;					// Was it unseen before being displayed
	bool					mItsMsgError;				// Flag for cleared out message
	CMessageList*			mMsgs;						// Used for multi-save operation
	const char*				mShowText;					// Text to show
	auto_ptr<unichar_t>		mUTF16Text;					// UTF16 text
	auto_ptr<unichar_t>		mRawUTF16Text;				// Raw UTF16 text
	bool					mShowHeader;				// Show header
	bool					mShowSecure;				// Show verify/decrypt area
	bool					mSecureMulti;				// Multi-line verify/decrypt area

	CToolbarView			mToolbarView;
	CMessageSimpleView		mView;						// The view that controls the main toolbar

	CSplitterView			mSplitterView;
	CMessageHeaderView*		mHeader;

	CGrayBackground			mPartsPane;					// Container for parts
	CSimpleTitleTable		mPartsTitles;				// Parts titles
	CBodyTable				mPartsTable;				// List of parts
	CWndAlignment*			mPartsTableAlign;			// Aligner for parts table

	CGrayBackground			mSplitter2Pane;				// Pane for bottom splitter

	CGrayBackground			mSecurePane;				// Header for verify/decrypt info
	CCmdEdit				mSecureInfo;				// Verify/decrypt details

	CGrayBackground			mTextArea;
	CFormattedTextDisplay*	mText;

	bool					mRedisplayBlock;			// Prevent window from being redisplayed
	bool					mAllowDeleted;				// Allow window close on message delete
	bool					mDidExpandParts;			// Expand parts was done previously
	static cdstring			sLastCopyTo;				// Cache last selected copy to

	SHeaderState			mHeaderState;				// State of items in header

	EView					mParsing;					// Parse
	long					mFontScale;					// Font scale factor
	long					mQuoteDepth;				// Quote depth

	CAttachment*			mCurrentPart;
	
	static cdstring		sNumberString;					// sprintf string for message number
	static cdstring		sNumberStringBig;				// sprintf string for message number >= 10000
	static cdstring		sSubMessageString;				// sprintf string for sub-message
	static cdstring		sDigestString;					// sprintf string for digestive message

			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();						// Reset message text
			void	ResetFontScale();					// Reset font scale text
			void	UpdatePartsCaption();				// Update parts caption
			void	UpdatePartsList();					// Update list of parts, show/hide

	CBodyTable*		GetPartsTable()
		{ return &mPartsTable; }

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	DECLARE_MESSAGE_MAP()
};

#endif
