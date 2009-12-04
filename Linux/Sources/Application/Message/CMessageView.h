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


// Header for CMessageView class

#ifndef __CMessageView__MULBERRY__
#define __CMessageView__MULBERRY__

#include "CBaseView.h"
#include <JXIdleTask.h>

#include "ETag.h"

#include "cdstring.h"
#include "cdmutexprotect.h"

#include "CCharsetCodes.h"
#include "CMessageFwd.h"
#include "CMessageWindow.h"
#include "C3PaneOptions.h"

#include "HPopupMenu.h"

#include <time.h>

#include <memory>

// Classes
class CAddressText;
class CAttachment;
class CBodyTable;
class CDivider;
class CFocusBorder;
class CMailboxToolbarPopup;
class CMailboxView;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CSimpleTitleTable;
class CSplitterView;
class CToolbarButton;
class CFormattedTextDisplay;

class JXFlatRect;
class JXImageWidget;
class JXMultiImageButton;
class JXMultiImageCheckbox;
class JXScrollbarSet;
class JXStaticText;
class JXUpRect;

class CMessageView : public CBaseView,
						public JXIdleTask
{
public:
	struct SMessageViewState
	{
		long		mScrollHpos;
		long		mScrollVpos;
		JIndex		mTextSelectionStart;
		JIndex		mTextSelectionEnd;
		bool		mShowHeader;
		bool		mPartsExpanded;
		bool		mDidExpandParts;
		EView		mParsing;
		long		mFontScale;
		
		SMessageViewState()
			{ _init(); }
		SMessageViewState(const SMessageViewState& copy)
			{ _copy(copy); }

		SMessageViewState& operator=(const SMessageViewState& copy)			// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }

	private:
		void _init()
			{ mScrollHpos = -1;
			  mScrollVpos = -1;
			  mTextSelectionStart = 0;
			  mTextSelectionEnd = 0;
			  mShowHeader = false;
			  mPartsExpanded = false;
			  mDidExpandParts = false;
			  mParsing = eViewFormatted;
			  mFontScale = 0; }
		void _copy(const SMessageViewState& copy)
			{ mScrollHpos = copy.mScrollHpos;
			  mScrollVpos = copy.mScrollVpos;
			  mTextSelectionStart = copy.mTextSelectionStart;
			  mTextSelectionEnd = copy.mTextSelectionEnd;
			  mShowHeader = copy.mShowHeader;
			  mPartsExpanded = copy.mPartsExpanded;
			  mDidExpandParts = copy.mDidExpandParts;
			  mParsing = copy.mParsing;
			  mFontScale = copy.mFontScale;  }
	};

	typedef std::vector<CMessageView*>	CMessageViewList;
	static cdmutexprotect<CMessageViewList> sMsgViews;	// List of windows (protected for multi-thread access)

private:
	static cdstring	sLastCopyTo;						// Cache last selected copy to
	CMessage*		mItsMsg;							// The message for this window
	CMailboxView*	mMailboxView;						// Associated mailbox view
	bool			mWasUnseen;							// Was it unseen before being displayed
	bool			mItsMsgError;						// Flag for cleared out message
	const char*		mShowText;							// Text to show
	std::auto_ptr<unichar_t>		mUTF16Text;					// UTF16 text
	std::auto_ptr<unichar_t>		mRawUTF16Text;				// Raw UTF16 text
	bool			mShowHeader;						// Show header
	bool			mShowAddressCaption;				// Show address caption area
	bool			mShowParts;							// Show parts area
	bool			mShowSecure;						// Show verify/decrypt area
	bool			mSecureMulti;						// Multi-line verify/decrypt area
	EView			mParsing;							// Parse
	long			mFontScale;							// Font scale factor
	long			mQuoteDepth;						// Quote depth

// begin JXLayout1

    JXUpRect*             mHeader;
    CAddressText*         mCaption1;
    CAddressText*         mCaption2;
    CToolbarButton*       mZoomBtn;
    JXFlatRect*           mPartsMove;
    CDivider*             mSeparator;
    JXStaticText*         mPartsTitle;
    CTwister*             mPartsTwister;
    JXStaticText*         mPartsField;
    JXMultiImageCheckbox* mFlatHierarchyBtn;
    JXImageWidget*        mAttachments;
    JXImageWidget*        mAlternative;
    JXMultiImageCheckbox* mHeaderBtn;
    HPopupMenu*           mTextFormat;
    JXMultiImageButton*   mFontIncreaseBtn;
    JXMultiImageButton*   mFontDecreaseBtn;
    JXStaticText*         mFontScaleField;
    HPopupMenu*           mQuoteDepthPopup;

// end JXLayout1

	CSplitterView*			mSplitterView;				// Splitter

    CFocusBorder*        	mPartsFocus;
    JXScrollbarSet*        	mPartsScroller;
    CSimpleTitleTable*		mPartsTitles;
	CBodyTable*				mPartsTable;				// List of parts

    JXFlatRect*         	mBottomPane;

    JXUpRect*         		mSecurePane;
    CTextDisplay*         	mSecureInfo;
	JColorList				mColorList;

    CFocusBorder*        	mTextFocus;
	CFormattedTextDisplay*	mText;

	bool			mRedisplayBlock;					// Prevent redisplay
	bool			mResetTextBlock;					// Prevent ResetText execution
	bool			mAllowDeleted;						// Allow deleted while constructing
	CAttachment*	mCurrentPart;
	bool			mDidExpandParts;					// Expand parts was done previously

	time_t			mSeenTriggerTime;					// Time at which to trigger seen flag change

public:
					CMessageView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CMessageView();

	virtual void	OnCreate();

	virtual void ListenTo_Message(long msg, void* param);

			void Close()
				{ DoClose(); }
	virtual void DoClose(void);

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void ApertureResized(const JCoordinate dw, const JCoordinate dh);

	virtual	void	Perform(const Time delta, Time* maxSleepTime);

	static  void	ServerState(bool logon);					// Server logs on/off

			void	SetMessage(CMessage* theMsg,				// Set the message
								bool restore = false);
			void	RestoreMessage(CMessage* theMsg,			// Set the message and restore state
									const SMessageViewState& state);
			CMessage* GetMessage()								// Get message
		{ return mItsMsg; };
			void	ClearMessage();								// Clear out the message
			void	PostSetMessage(bool restore = false);		// Processing after message and display update
			void	MessageChanged();							// Someone else changed this message
			void	ShowPart(CAttachment* attach, bool restore = false);				// Show a particular part
			void	ShowSubMessage(CAttachment* attach);		// Display a sub-message
			void	ExtractPart(CAttachment* part,				// Extract/view a particular part
									bool view);
			bool	ShowRawBody();								// Read in raw body

			void	AddPrintSummary();						// Temporarily add header summary for printing
			void	RemovePrintSummary();					// Remove temp header summary after printing

			const CAttachment* GetCurrentPart() const
		{ return mCurrentPart; }

			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();							// Reset message text
			void	ResetFontScale();						// Reset font scale text
			void	ResetCaption();							// Reset message caption
			void	ResetOptions();							// Mail view options changed


	static	CMessageView*	FindView(const CMessage* theMsg,	// Find window belonging to message
										bool owned_by = false);
	static bool	ViewExists(const CMessageView* wnd);			// Check for view

	virtual void	MakeToolbars(CToolbarView* parent);

	CMailboxView*	GetOwnerView() const
		{ return mMailboxView; }
	void	SetOwnerView(CMailboxView* view)
		{ mMailboxView = view; }

	virtual bool	HasFocus() const;
	virtual void	Focus();

			EView GetViewAs() const							// Parse
				{ return mParsing;	}

			void	DoSaveAs();
		const unichar_t* GetSaveText();									// Get text to save

			bool	SpacebarScroll(bool shift_key);								// Scroll the text if possible

			void	GetViewState(SMessageViewState& state) const;	// Get current state of the view
			void	SetViewState(const SMessageViewState& state);	// Change view state to one saved earlier
			
	// Common updaters
	void	OnUpdateMessageReadPrev(CCmdUI* pCmdUI);
	void	OnUpdateMessageReadNext(CCmdUI* pCmdUI);
	void	OnUpdateMessageCopyNext(CCmdUI* pCmdUI);
	void	OnUpdateMessageReject(CCmdUI* pCmdUI);
	void	OnUpdateMessageSendAgain(CCmdUI* pCmdUI);
	void	OnUpdateMessageCopy(CCmdUI* pCmdUI);
	void	OnUpdateMessageDelete(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsTop(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI);
	void	OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag);
	void	OnUpdateMessageViewCurrent(CCmdUI* pCmdUI);
	void	OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI);
	void	OnUpdateMessageShowHeader(CCmdUI* pCmdUI);
	void	OnUpdateWindowsShowParts(CCmdUI* pCmdUI);

	void	OnFileNewDraft();
	void	DoNewLetter(bool option_key);		// Display a new draft
	void	OnFilePageSetup();
	void	OnFilePrint();

	void	OnEditSpellCheck();
	void	OnMessageReadPrev();
	void	OnMessageReadNext();
	void	OnMessageReadNextNew();
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

	void    OnTextFormatPopup(JIndex nID);
	void	OnViewAs(EView view);

	void	OnIncreaseFont();						// Increase displayed font size
	void	OnDecreaseFont();						// Decrease displayed font size

	void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

	void	OnMessagePartsTwister();

	void	OnMessagePartsFlat();
	void	DoPartsFlat(bool flat);		// Handle flatten of parts

	void	OnZoomPane();

	const CMailViewOptions&	GetViewOptions() const;		// Get options for this view


public:
			void	ResetFont(const SFontInfo& list_traits,
									const SFontInfo& display_traits);		// Reset text traits

	virtual void	ResetState(bool force = false);		// Reset state from prefs
	virtual void	SaveState();						// Save state
	virtual void	SaveDefaultState();					// Save state as default

private:
			CMailboxToolbarPopup* GetCopyBtn() const;

			CBodyTable*		GetPartsTable()
				{ return mPartsTable; }

			void	InitMessageView();

			void	UpdatePartsCaption();			// Update parts caption
			void	UpdatePartsList();				// Update list of parts, show/hide

			void	ShowCaption(bool show);				// Show/hide address caption area
			void	ShowParts(bool show);				// Show/hide parts area
			void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
			void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

			void	StartSeenTimer(unsigned long secs);			// Start timer to trigger seen flag
			void	StopSeenTimer();							// Stop timer to trigger seen flag
};

#endif
