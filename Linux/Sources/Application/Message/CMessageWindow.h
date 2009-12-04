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

#include "CFileDocument.h"

#include "CMainMenu.h"
#include "CMessageFwd.h"
#include "CFormattedTextDisplay.h"
#include "ETag.h"

#include "cdstring.h"
#include "cdustring.h"
#include "cdmutexprotect.h"

#include <memory>

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
	eQuoteDepth_All = 1,
	eQuoteDepth_No,
	eQuoteDepth_1 = 3,		// Linux ignores separator
	eQuoteDepth_2,
	eQuoteDepth_3,
	eQuoteDepth_4
};

// Classes
class CAttachment;
class CAddressList;
class CBodyTable;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CMessageHeaderView;
class CMessageSimpleView;
class CSimpleTitleTable;
class CSplitterView;
class CTextDisplay;
class CTextBase;
class CToolbarView;
class CTwister;
class CSplitter;

class JXDecorRect;
class JXFlatRect;
class JXScrollbarSet;
class JXUpRect;

class CMessageWindow : public CFileDocument
{
	typedef CFileDocument super;

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
	typedef std::vector <CMessageWindow*> CMessageWindowList;
	static cdmutexprotect<CMessageWindowList> sMsgWindows;	// List of windows (protected for multi-thread access)

			CMessageWindow(JXDirector* supervisor);
	virtual ~CMessageWindow();

	static CMessageWindow* ManualCreate(bool hidden = false);

	virtual void OnCreate();

	virtual void Activate();

	void	DoSaveAs();

protected:
	virtual void Receive (JBroadcaster* sender, const Message& message);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual CCommander* GetTarget();

	virtual void WriteTextFile(std::ostream& output, const JBoolean safetySave) const;

public:
			CTextDisplay* GetText()
						{ return mText; }

	static  void	ServerState(bool logon);					// Server logs on/off

			void	SetTwistList(CTextBase* aField,
							     CTwister* aTwister,
							     JXDecorRect* aMover,
							     CAddressList* list);
			void	SetMessage(CMessage* aMsg);				// Set the message
			void	ClearMessage();							// Clear out the message

			CMessage*	GetMessage()						// Get its message
							{ return mItsMsg; }

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

			const unichar_t* GetSaveText();						// Get text to save
			void	ShowPart(CAttachment* attach);			// Show a particular part
			void	ShowSubMessage(CAttachment* attach);	// Display a sub-message
			void	ExtractPart(CAttachment* attach,		// Extract/view a particular part
									bool view);
			bool	ShowRawBody();							// Read in raw body

			void	AddPrintSummary();						// Temporarily add header summary for printing
			void	RemovePrintSummary();					// Remove temp header summary after printing

	static CMessageWindow* FindWindow(const CMessage* aMsg, bool owned_by = false); // Find the corresponding window
	static bool	WindowExists(const CMessageWindow* wnd);	// Check for window

			void	ResetFont(const SFontInfo& list_traits,
									const SFontInfo& display_traits);		// Reset text traits

	// Common updaters
	void	OnUpdateMessageReadPrev(CCmdUI* cmdui);
	void	OnUpdateMessageReadNext(CCmdUI* cmdui);
	void	OnUpdateMessageCopyNext(CCmdUI* cmdui);
	void	OnUpdateMessageReject(CCmdUI* pCmdUI);
	void	OnUpdateMessageSendAgain(CCmdUI* cmdui);
	void	OnUpdateMessageCopy(CCmdUI* cmdui);
	void	OnUpdateMessageDelete(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsTop(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsSeen(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsImportant(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsAnswered(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsDeleted(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsDraft(CCmdUI* cmdui);
	void	OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag);
	void	OnUpdateMessageViewCurrent(CCmdUI* pCmdUI);
	void	OnUpdateMessageVerifyDecrypt(CCmdUI* cmdui);
	void	OnUpdateMessageShowHeader(CCmdUI* cmdui);
	void	OnUpdateMenuExpandHeader(CCmdUI* cmdui);
	void	OnUpdateExpandHeader(CCmdUI* cmdui);
	void	OnUpdateWindowsShowParts(CCmdUI* cmdui);

	// Command handlers
	void	OnFileNewDraft();
	void	DoNewLetter(bool option_key);		// Display a new draft

	void	OnEditSpellCheck();
	void	OnMessageReadPrev();
	void	OnMessageReadNext();
	void	ReadNextMessage(bool delete_it, bool copy_it, bool option_key);				// Read next message
	void	OnMessageDeleteRead();
	void	OnMessageCopyRead();
	void	CopyReadNextMessage(bool option_key);

	bool	RedisplayMessage(CMessage* theMsg);					// Display message in existing window
	void	DoRollover(CMbox* current_mbox);					// Attempt to rollover to next unseen
	void	OnMessageReply();
	void	OnMessageReplySender();
	void	OnMessageReplyFrom();
	void	OnMessageReplyAll();
	void	ReplyToThisMessage(EReplyTo reply_to, bool option_key);	// Reply to this message

	void	OnMessageForward();
	void	ForwardThisMessage(bool option_key);
	void	OnMessageBounce();
	void	OnMessageReject();
	void	OnMessageSendAgain();
	void	OnMessageCopyBtn();					// Copy the message to chosen mailbox

	void	OnMessageCopy(JXTextMenu* menu, JIndex nID);			// Copy the message to another mailbox
	void	OnMessageCopyCmd();										// Explicit copy
	void	OnMessageMoveCmd();										// Explicit move
	bool	TryCopyMessage(bool option_key);						// Try to copy a message
	bool	CopyThisMessage(CMbox* mbox, bool option_key);			// Copy to specific mailbox

#ifdef NOTYET
	void	OnAddressCopy(UINT nID);			// Copy the message's addresses to an address book
#endif
	void	OnMessageFlagsSeen();
	void	OnMessageFlagsImportant();
	void	OnMessageFlagsAnswered();
	void	OnMessageFlagsDraft();
	void	OnMessageFlagsLabel(JIndex nID);
	void	OnMessageFlags(NMessage::EFlags flag);
	void	OnMessageDecodeBinHex();
	void	OnMessageDecodeUU();
	void	OnMessageViewCurrent();
	void	OnMessageVerifyDecrypt();

	void	OnMessageDelete();
	void	OnMessageShowHeader();

	void	OnMessageFromTwister();
	void	OnMessageToTwister();
	void	OnMessageCCTwister();
	void	DoTwist(CTextBase* aField,
					CTwister* aTwister,
					JXDecorRect* aMove); //Handle Twist

	void	LayoutHeader();								// Layout header component based on state

	void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
	void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

	void	OnTextFormatPopup(JIndex nID);
	void	OnViewAs(EView view);

	void	OnIncreaseFont();					// Increase displayed font size
	void	OnDecreaseFont();					// Decrease displayed font size

	void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

	void	OnMessagePartsTwister();

	void	HeaderExpand(bool expand);

	void	OnMessagePartsFlat();
	void	DoPartsFlat(bool flat);

	virtual void	ResetState(bool force = false);			// Reset window state
	virtual void	SaveDefaultState(void);					// Save current state as default

	void	OnWindowsExpandHeader();

private:
	CMessage*				mItsMsg;					// The message for this window
	bool					mWasUnseen;					// Was it unseen before being displayed
	bool					mItsMsgError;				// Flag for cleared out message
	CMessageList*			mMsgs;						// Used for multi-save operation
	const char*				mShowText;					// Text to show
	std::auto_ptr<unichar_t>		mUTF16Text;					// UTF16 text
	std::auto_ptr<unichar_t>		mRawUTF16Text;				// Raw UTF16 text
	bool					mShowHeader;				// Show header
	bool					mShowSecure;				// Show verify/decrypt area
	bool					mSecureMulti;				// Multi-line verify/decrypt area

	CToolbarView*			mToolbarView;
	CMessageSimpleView*		mView;						// The view that controls the main toolbar
	CSplitterView*			mSplitterView;
	CMessageHeaderView*		mHeader;
	JXScrollbarSet*			mPartsScroller;				// Parts scroller
	CSimpleTitleTable*		mPartsTitles;				// Parts titles
	CBodyTable*				mPartsTable;				// List of parts

    JXFlatRect*         	mBottomPane;

    JXUpRect*         		mSecurePane;
    CTextDisplay*         	mSecureInfo;
	JColorList*				mColorList;

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

			CBodyTable*		GetPartsTable()
				{ return mPartsTable; }

			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();						// Reset message text
			void	ResetFontScale();					// Reset font scale text
			void	UpdatePartsCaption();				// Update parts caption
			void	UpdatePartsList();					// Update list of parts, show/hide
};

#endif
