/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#ifndef __CMESSAGEVIEW__MULBERRY__
#define __CMESSAGEVIEW__MULBERRY__

#include "CBaseView.h"
#include "ETag.h"

#include "cdstring.h"
#include "cdmutexprotect.h"

#include "CMessageWindow.h"
#include "C3PaneOptions.h"

#include "CBodyTable.h"
#include "CCharsetCodes.h"
#include "CCmdEdit.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CIconWnd.h"
#include "CMailboxPopup.h"
#include "CMessageFwd.h"
#include "CNumberEdit.h"
#include "CPopupButton.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CToolbarButton.h"
#include "CTwister.h"

// Classes
class CAttachment;
class CBodyTable;
class CDisplayFormatter;
class CMailboxPopup;
class CMailboxView;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CSplitterView;

class CMessageView : public CBaseView
{

	friend class CSpaceBarAttachment;

public:
	struct SMessageViewState
	{
		long		mScrollHpos;
		long		mScrollVpos;
		long		mTextSelectionStart;
		long		mTextSelectionEnd;
		bool		mShowHeader;
		bool		mPartsExpanded;
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
			  mParsing = eViewFormatted;
			  mFontScale = 0; }
		void _copy(const SMessageViewState& copy)
			{ mScrollHpos = copy.mScrollHpos;
			  mScrollVpos = copy.mScrollVpos;
			  mTextSelectionStart = copy.mTextSelectionStart;
			  mTextSelectionEnd = copy.mTextSelectionEnd;
			  mShowHeader = copy.mShowHeader;
			  mPartsExpanded = copy.mPartsExpanded;
			  mParsing = copy.mParsing;
			  mFontScale = copy.mFontScale;  }
	};

	typedef std::vector<CMessageView*>	CMessageViewList;
	static cdmutexprotect<CMessageViewList> sMsgViews;	// List of windows (protected for multi-thread access)

public:
					CMessageView();
	virtual 		~CMessageView();

	static	CMessageView*	FindView(const CMessage* theMsg,	// Find window belonging to message
										bool owned_by = false);
	static bool	ViewExists(const CMessageView* wnd);			// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	virtual void	DoClose();

	virtual void	MakeToolbars(CToolbarView* parent);

	CMailboxView*	GetOwnerView() const
		{ return mMailboxView; }
	void	SetOwnerView(CMailboxView* view)
		{ mMailboxView = view; }

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual CCmdEditView* GetText()
						{ return mText; }

	virtual CDocument* GetDocument()
						{ return mText->GetDocument(); }

	static  void	ServerState(bool logon);					// Server logs on/off

			void	SetMessage(CMessage* aMsg,					// Set the message
								bool restore = false);
			void	RestoreMessage(CMessage* theMsg,			// Set the message and restore state
									const SMessageViewState& state);
			CMessage*	GetMessage() const						// Get its message
							{ return mItsMsg; }
			void	ClearMessage();								// Clear out the message

			void	PostSetMessage(bool restore = false);		// Do post processing of message
			void	MessageChanged();							// Someone else changed this message
			void	ResetOptions();								// Mail view options changed

			const char* GetShowText()							// Get text showing
						{ return mShowText; }
	
			const CAttachment* GetCurrentPart() const
		{ return mCurrentPart; }
			EView GetViewAs() const								// Parse
				{ return mParsing;	}

			const unichar_t* GetSaveText();							// Get text to save
			void	ShowPart(CAttachment* attach, bool restore = false);	// Show a particular part
			void	ShowSubMessage(CAttachment* attach);		// Display a sub-message
			void	ExtractPart(CAttachment* attach,			// Extract/view a particular part
									bool view);
			bool	ShowRawBody();								// Read in raw body

			void	AddPrintSummary();						// Temporarily add header summary for printing
			void	RemovePrintSummary();					// Remove temp header summary after printing

	virtual void	ResetFont(CFont* list_font,
								CFont* display_font);			// Reset fonts

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

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
	afx_msg void	OnMessageReadNextNew();
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
	afx_msg void	OnMessageCopyBtn();					// Copy the message to chosen mailbox
	afx_msg void	OnMessageCopyCmd();									// Copy the message to chosen mailbox
	afx_msg void	OnMessageMoveCmd();									// Move the message to chosen mailbox
	afx_msg void	OnMessageCopy(UINT nID);			// Copy the message to another mailbox
			bool	TryCopyMessage(bool option_key);	// Try to copy a message
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

	afx_msg void	OnViewAs(EView view);
	afx_msg void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

	afx_msg void	OnIncreaseFont();					// Increase displayed font size
	afx_msg void	OnDecreaseFont();					// Decrease displayed font size

			const CMailViewOptions&	GetViewOptions() const;		// Get options for this view

			void	ShowCaption(bool show);				// Show/hide address caption area
			void	ShowParts(bool show);				// Show/hide parts area
			void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
			void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

	afx_msg void	OnMessagePartsTwister();

	afx_msg void	OnMessagePartsFlat();
			void	DoPartsFlat(bool hierarchic);

	afx_msg void	OnSaveDefaultState();			// Save state in prefs
	afx_msg void	OnResetDefaultState();

	afx_msg void    OnTextFormatPopup(UINT nID);

	afx_msg void    OnQuoteDepthPopup(UINT nID);

			bool	SpacebarScroll(bool shift_key);				// Scroll the text if possible

			void	GetViewState(SMessageViewState& state) const;	// Get current state of the view
			void	SetViewState(const SMessageViewState& state);	// Change view state to one saved earlier

	virtual void	ResetState(bool force = false);		// Reset state from prefs
	virtual void	SaveState();						// Save state
	virtual void	SaveDefaultState();					// Save state as default

private:
	static cdstring			sLastCopyTo;				// Cache last selected copy to
	CMessage*				mItsMsg;					// The message for this window
	CMailboxView*			mMailboxView;				// Associated mailbox view
	bool					mWasUnseen;					// Was it unseen before being displayed
	bool					mItsMsgError;				// Flag for cleared out message
	const char*				mShowText;					// Text to show direct from attachment in local encoding
	std::auto_ptr<unichar_t>		mUTF16Text;					// UTF16 text
	std::auto_ptr<unichar_t>		mRawUTF16Text;				// Raw UTF16 text
	bool					mShowHeader;				// Show header
	bool					mShowAddressCaption;		// Show address caption area
	bool					mShowParts;					// Show parts area
	bool					mShowSecure;				// Show verify/decrypt area
	bool					mSecureMulti;				// Multi-line verify/decrypt area
	EView					mParsing;					// Parse
	long					mFontScale;					// Font scale factor
	long					mQuoteDepth;				// Quote depth

	CGrayBackground			mHeader;					// Header for caption/parts controls
	CCmdEdit				mCaption1;					// Caption details
	CCmdEdit				mCaption2;					// Caption details
	CToolbarButton			mZoom;

	CGrayBackground			mPartsMove;					// Header for parts controls
	CStatic					mDivider;					// Divider line
	CStatic					mPartsTitle;				// Parts title
	CTwister				mPartsTwister;				// Parts twist button
	CStatic					mPartsField;				// Number of parts
	CIconButton				mFlatHierarchyBtn;			// Flat/hierarchy button
	CIconWnd				mAttachments;				// Indicate attachments
	CIconWnd				mAlternative;				// Indicate alternatives
	CIconButton				mHeaderBtn;					// Show header button
	CPopupButton			mTextFormat;				// Text format	 popup menu
	CIconButton				mFontIncreaseBtn;			// Increase font size
	CIconButton				mFontDecreaseBtn;			// Decrease font size
	CStatic					mFontScaleField;			// Font scale value
	CPopupButton			mQuoteDepthPopup;			// Quote depth popup menu
	CMailboxToolbarPopup*	mCopyToPopup;				// Copy to popup menu

	CSplitterView			mSplitter;

	CGrayBackground			mPartsFocus;				// Focus for parts table
	CSimpleTitleTable		mPartsTitles;				// Parts titles
	CBodyTable				mPartsTable;				// List of parts
	CWndAlignment*			mPartsTableAlign;			// Aligner for parts table

	CGrayBackground			mSplitter2Pane;				// Pane for bottom splitter

	CGrayBackground			mSecurePane;				// Header for verify/decrypt info
	CCmdEdit				mSecureInfo;				// Verify/decrypt details

	CGrayBackground			mTextFocus;					// Focus for text
	CFormattedTextDisplay*	mText;

	bool					mRedisplayBlock;			// Prevent window from being redisplayed
	bool					mResetTextBlock;			// Prevent ResetText execution
	bool					mAllowDeleted;				// Allow window close on message delete
	CAttachment*			mCurrentPart;
	bool					mDidExpandParts;			// Expand parts was done previously
	
	UINT					mTimerID;					// ID for seen timer

	CCommanderProtect		mCmdProtect;				// Protect commands
	
			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();						// Reset message text
			void	ResetFontScale();					// Reset font scale text
			void	ResetCaption();						// Reset caption text

			void	UpdatePartsCaption();				// Update parts caption
			void	UpdatePartsList();					// Update list of parts, show/hide

			void	StartSeenTimer(unsigned long secs);			// Start timer to trigger seen flag
			void	StopSeenTimer();							// Stop timer to trigger seen flag

	CBodyTable*		GetPartsTable()
		{ return &mPartsTable; }

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg void OnZoomPane();

	DECLARE_MESSAGE_MAP()
};

#endif
