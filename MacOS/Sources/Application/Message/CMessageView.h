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
#include "CCharsetCodes.h"
#include "ETag.h"

#include "cdstring.h"
#include "cdmutexprotect.h"

#include <UPrinting.h>

#include "CMessageFwd.h"
#include "CMessageWindow.h"
#include "C3PaneOptions.h"

#include <time.h>

// Constants
const	PaneIDT		paneid_MessageViewSplitter = 'SPLT';
const	PaneIDT		paneid_MessageViewTop = 'TOPP';
const	PaneIDT		paneid_MessageViewCaption1 = 'CAP1';
const	PaneIDT		paneid_MessageViewCaption2 = 'CAP2';
const	PaneIDT		paneid_MessageViewSeparator = 'SEP1';
const	PaneIDT		paneid_MessageViewParts = 'PART';
const	PaneIDT		paneid_MessageViewTextFocus = 'MSGT';
const	PaneIDT		paneid_MessageViewZoom = 'ZOOM';

// Panes

// Resources

// Other STR#

// Messages

// Others

// Classes
class LDisclosureTriangle;
class LBevelButton;
class LIconControl;
class LPane;
class LPopupButton;
class CStaticText;

class CAttachment;
class CBodyTable;
class CMailboxToolbarPopup;
class CMailboxView;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CSplitterView;
class CTextDisplay;
class CFormattedTextDisplay;
class CWebKitControl;

class CMessageView : public CBaseView,
						public LListener,
						public LPeriodical
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
	const char*		mShowText;							// Text to show direct from attachment in local encoding
	std::auto_ptr<unichar_t>	mUTF16Text;						// UTF16 text
	std::auto_ptr<unichar_t>	mRawUTF16Text;					// Raw UTF16 text
	LPrintSpec*		mPrintSpec;
	LPrintSpec		mMyPrintSpec;
	bool			mShowHeader;						// Show header
	bool			mShowAddressCaption;				// Show address caption area
	bool			mShowParts;							// Show parts area
	bool			mShowSecure;						// Show verify/decrypt area
	bool			mSecureMulti;						// Multi-line verify/decrypt area
	EView			mParsing;							// Parse
	long			mFontScale;							// Font scale factor
	long			mQuoteDepth;						// Quote depth

	CSplitterView*	mSplitter;							// Splitter

	LView*			mHeader;							// Header area
	LBevelButton*	mZoomBtn;							// Zoom button

	CTextDisplay*	mCaption1;							// Caption details
	CTextDisplay*	mCaption2;							// Caption details
	LView*			mParts;								// Parts area
	LPane*			mSeparator;							// Horiz separator
	LDisclosureTriangle*	mAttachTwister;				// Twister for attachment table
	CStaticText*	mAttachNumberField;					// Number of attachments
	LBevelButton*	mFlatHierarchyBtn;					// Flat/hierarchy button
	LIconControl*	mAttachmentsIcon;					// Indicates message has attachments
	LIconControl*	mAlternativeIcon;					// Indicates message is text alternative only
	LView*			mAttachHide;						// Pane to hide when twisted up
	CBodyTable*		mAttachTable;						// List of attachments
	LBevelButton*	mHeaderBtn;							// The header toggle button

	LView*			mBottomPane;						// Pane containing bottom splitter

	LView*			mSecurePane;						// Pane contain verify/decrypt details
	CTextDisplay*	mSecureInfo;						// Verify/decrypt details

	LView*			mTextPane;							// Pane containing bottom splitter
	CFormattedTextDisplay*	mText;						// The text
	CWebKitControl*	mHTML;								// HTML text

	LPopupButton*	mParseLevel;						// User selected parse level justin
	LBevelButton*	mFontIncreaseBtn;					// Font increase button
	LBevelButton*	mFontDecreaseBtn;					// Font decrease button
	CStaticText*	mFontScaleField;					// Font scaling size
	LPopupButton*	mQuoteDepthPopup;					// User selected quote depth

	bool			mRedisplayBlock;					// Prevent redisplay
	bool			mResetTextBlock;					// Prevent ResetText execution
	bool			mAllowDeleted;						// Allow deleted while constructing
	CAttachment*	mCurrentPart;
	bool			mDidExpandParts;					// Expand parts was done previously

	time_t			mSeenTriggerTime;					// Time at which to trigger seen flag change

public:
	enum { class_ID = 'MeVi' };

					CMessageView();
					CMessageView(LStream *inStream);
	virtual 		~CMessageView();

	virtual void ListenTo_Message(long msg, void* param);

protected:
	virtual void	FinishCreateSelf();					// Do odds & ends

public:
	virtual void		DoClose(void);

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);		// Respond to clicks in the icon buttons
	virtual void	ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh);

	virtual	void	SpendTime(const EventRecord &inMacEvent);	// Called during idle

	static  void	ServerState(bool logon);					// Server logs on/off

			void	SetMessage(CMessage* theMsg,				// Set the message
								bool restore = false);
			void	RestoreMessage(CMessage* theMsg,			// Set the message and restore state
									const SMessageViewState& state);
			CMessage* GetMessage()								// Get message
		{ return mItsMsg; };
			void	ClearMessage();									// Clear out the message
			void	PostSetMessage(bool restore = false);			// Processing after message and display update
			void	MessageChanged();								// Someone else changed this message
			void	ShowPart(CAttachment* attach, bool restore = false);	// Show a particular part
			void	ShowSubMessage(CAttachment* attach,
							Rect zoom_from);					// Display a sub-message
			void	ExtractPart(CAttachment* part,				// Extract/view a particular part
									bool view);
			bool	ShowRawBody();								// Read in raw body

			const CAttachment* GetCurrentPart() const
		{ return mCurrentPart; }

	virtual void	SetPrintSpec(LPrintSpec* printSpec);	// Set a print spec
			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();							// Reset message text
			void	ResetFontScale();						// Reset font scale text
			void	ResetCaption();							// Reset message caption
			void	ResetOptions();							// Mail view options changed

	static	CMessageView*	FindView(const CMessage* theMsg,	// Find window belonging to message
										bool owned_by = false);
	static bool	ViewExists(const CMessageView* wnd);			// Check for view

	virtual void	MakeToolbars(CToolbarView* parent);
	CMailboxToolbarPopup* GetCopyBtn() const;

	CMailboxView*	GetOwnerView() const
		{ return mMailboxView; }
	void	SetOwnerView(CMailboxView* view)
		{ mMailboxView = view; }

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	SaveThisMessage();								// Save this message to file
	virtual void	DoSave(PPx::FSObject &inFileSpec,
								OSType inFileType);					// Save the message from filespec
	const unichar_t* GetSaveText();									// Get text to save

	LPrintSpec*		GetPrintSpec()
		{ return mPrintSpec ? mPrintSpec : &mMyPrintSpec; }

	virtual void	DoPrint();										// Print the message

			void	OnEditSpellCheck();								// Spell check message

			void	SpeakMessage();									// Speak whole message

			bool	SpacebarScroll(bool shift_key);					// Scroll the text if possible

			void	GetViewState(SMessageViewState& state) const;	// Get current state of the view
			void	SetViewState(const SMessageViewState& state);	// Change view state to one saved earlier
			
private:
			CBodyTable*		GetPartsTable() const
				{ return mAttachTable; }

			void	DoNewLetter(bool option_key);					// Display a new draft
			void	OnMessageReadPrev();							// Read previous message
			void	OnMessageReadNext();							// Read next message
			void	OnMessageReadNextNew();
			void	ReadNextMessage(bool delete_it, bool copy_it, bool option_key);				// Read next message
			void	OnMessageDeleteRead();							// Delete then read next message
			void	CopyReadNextMessage(bool option_key);			// Copy then read next message
			bool	RedisplayMessage(CMessage* theMsg);				// Display message in existing window
			void	DoRollover(CMbox* current_mbox);				// Attempt to rollover to next unseen

			void	ReplyToThisMessage(EReplyTo reply_to,
								bool option_key);			// Reply to this message
			void	ForwardThisMessage(bool option_key);	// Forward this message
			void	OnMessageBounce();						// Bounce this message
			void	OnMessageReject();						// Reject this message
			void	OnMessageSendAgain();					// Send this message again

			void	OnMessageViewCurrent();					// View current part

			void	OnMessageDecodeBinHex();				// Decode message using BinHex
			void	OnMessageDecodeUU();					// Decode message using UU

			void	OnMessageVerifyDecrypt();				// Verify/decrypt message

			bool	TryCopyMessage(bool option_key);		// Try to copy a message
			bool	CopyThisMessage(CMbox* mbox,
								bool option_key);			// Copy the message to another mailbox

			void	FindFlagCommandStatus(Boolean &outEnabled,
									Boolean &outUsesMark,
									UInt16 &outMark,
									NMessage::EFlags flag);

			void	OnMessageFlags(NMessage::EFlags flags);
			void	OnMessageDelete();				// Delete the message and then the window

			void	DoPartsTwist();					// Handle twist of parts
			void	UpdatePartsCaption();			// Update parts caption
			void	UpdatePartsList();				// Update list of parts, show/hide

			void	DoPartsFlat(bool hierarchic);		// Handle flatten of parts

			void    OnTextFormatPopup(long nID);
			void	OnViewAs(EView view);

			void	OnIncreaseFont();						// Increase displayed font size
			void	OnDecreaseFont();						// Decrease displayed font size

			void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

			const CMailViewOptions&	GetViewOptions() const;		// Get options for this view

			void	ShowCaption(bool show);						// Show/hide address caption area
			void	ShowParts(bool show);						// Show/hide parts area
			void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
			void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

			void	StartSeenTimer(unsigned long secs);			// Start timer to trigger seen flag
			void	StopSeenTimer();							// Stop timer to trigger seen flag

public:
	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits,
									const TextTraitsRecord& display_traits);	// Reset text traits

	virtual void	ResetState(bool force = false);		// Reset state from prefs
	virtual void	SaveState();						// Save state
	virtual void	SaveDefaultState();					// Save state as default

private:
			void	InitMessageView();
};

#endif
