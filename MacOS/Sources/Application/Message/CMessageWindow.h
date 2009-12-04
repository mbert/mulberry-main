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

#include "LWindow.h"
#include "CWindowStatus.h"
#include "CMessageFwd.h"
#include "ETag.h"

#include "cdstring.h"
#include "cdustring.h"
#include "cdmutexprotect.h"

#include <UPrinting.h>

// Constants
const	OSType		kMessageFileType = 'TEXT';				// Save messages as text

// Panes
const	ClassIDT	class_MessageWindow ='MeWi';
const	PaneIDT		paneid_MessageWindow = 2000;
const	PaneIDT		paneid_MessageToolbarView = 'TBar';
const	PaneIDT		paneid_MessageView = 'VIEW';
const	PaneIDT		paneid_MessageHeader = 'MHDR';
const	PaneIDT		paneid_MessageFromMove = 'FRMV';
const	PaneIDT		paneid_MessageFromTwister = 'FRTW';
const	PaneIDT		paneid_MessageFromX = 'FRST';
const	PaneIDT		paneid_MessageToMove = 'TOMV';
const	PaneIDT		paneid_MessageToTwister = 'TOTW';
const	PaneIDT		paneid_MessageToX = 'TOST';
const	PaneIDT		paneid_MessageCCMove = 'CCMV';
const	PaneIDT		paneid_MessageCCTwister = 'CCTW';
const	PaneIDT		paneid_MessageCCX = 'CCST';
const	PaneIDT		paneid_MessageSubjectMove = 'SUMV';
const	PaneIDT		paneid_MessageSubjectTitle = 'SUTT';
const	PaneIDT		paneid_MessageSubject = 'SUBJ';
const	PaneIDT		paneid_MessageDateTitle = 'DATT';
const	PaneIDT		paneid_MessageDate = 'DATE';
const	PaneIDT		paneid_MessageNumber = 'MNUM';
const	PaneIDT		paneid_MessageNumberX = 'LNUM';
const	PaneIDT		paneid_MessageAttachMove = 'PARV';
const	PaneIDT		paneid_MessageAttachNumber = 'ANUM';
const	PaneIDT		paneid_MessageFlatHierarchy = 'HIER';
const	PaneIDT		paneid_MessageAttachTwister = 'ATTW';
const	PaneIDT		paneid_MessageAttachmentsIcon= 'PAR1';
const	PaneIDT		paneid_MessageAlternativeIcon= 'PAR2';
const	PaneIDT		paneid_MessageBtnHeader = 'BTN3';
const 	PaneIDT		paneid_MessageParseLevel = 'PARS';
const	PaneIDT		paneid_MessageFontIncrease = 'FNTU';
const	PaneIDT		paneid_MessageFontDecrease = 'FNTD';
const	PaneIDT		paneid_MessageFontScale = 'FNUM';
const	PaneIDT		paneid_MessageQuoteDepth = 'QUOT';
const	PaneIDT		paneid_MessageAttachHide = 'ATHD';
const	PaneIDT		paneid_MessageAttachBackground = 'AHDR';
const	PaneIDT		paneid_MessageAttachContentTitle = 'ATCT';
const	PaneIDT		paneid_MessageAttachScroller = 'SATT';
const	PaneIDT		paneid_MessageAttachTable = 'ATTM';
const	PaneIDT		paneid_MessageBottom = 'ITXT';
const	PaneIDT		paneid_MessageSecure = 'SECP';
const	PaneIDT		paneid_MessageSecureInfo = 'SECR';
const	PaneIDT		paneid_MessageScroller = 'STXT';
const	PaneIDT		paneid_MessageText = 'MTXT';
const	PaneIDT		paneid_MessageHTML = 'HTML';
const	PaneIDT		paneid_MessageSplit = 'SPLT';

const	PaneIDT		paneid_MessagePrintout = 2500;
const	PaneIDT		paneid_MessagePrintHead = 'HEAD';
const	PaneIDT		paneid_MessagePrintArea = 'TBOX';
const	PaneIDT		paneid_MessagePrintFooter = 'FOOT';
const	short		cFooterSize = 16;

// Resources
const	ResIDT		RidL_CMessageBtns = 2000;
const	ResIDT		Txtr_DefaultMessageText = 1000;

const	ResIDT		MENU_Flags = 155;
const	SInt16		menu_Seen = 1;
const	SInt16		menu_Important = 2;
const	SInt16		menu_Answered = 3;
const	SInt16		menu_Deleted = 4;

// Messages
const	MessageT	msg_HeaderMessage = 2002;
const	MessageT	msg_MTwistFrom = 2003;
const	MessageT	msg_MTwistTo = 2004;
const	MessageT	msg_MTwistCC = 2005;
const	MessageT	msg_MTwistAttach = 2006;
const	PaneIDT		msg_MFlatHierarchy = 'HIER';
const 	MessageT	msg_View = 'PARS';
const	MessageT	msg_FontUp = 'FNTU';
const	MessageT	msg_FontDown = 'FNTD';
const	MessageT	msg_QuoteDepth = 'QUOT';

// Others
const	short		kMaxTitleLength = 64;				// Maximum number of chars in window title
const	SInt16		cTwistMove = 46;

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
	eQuoteDepth_1 = 4,
	eQuoteDepth_2,
	eQuoteDepth_3,
	eQuoteDepth_4
};

// Classes
class LDisclosureTriangle;
class LBevelButton;
class LIconControl;
class LPane;
class LPopupButton;
class CStaticText;

class CAddressList;
class CAttachment;
class CBodyTable;
class CFormattedTextDisplay;
class CWebKitControl;
class CMbox;
class CMessage;
class CMessageCryptoInfo;
class CMessageList;
class CMessageSimpleView;
class CSplitterView;
class CAddressText;
class CTextDisplay;
class CTextFieldX;
class CToolbarView;

class	CMessageWindow : public LWindow,
						 public LListener,
						 public LDragAndDrop,
						 public CWindowStatus
{
	friend class CSpaceBarAttachment;

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
	typedef std::vector<CMessageWindow*> CMessageWindowList;
	static cdmutexprotect<CMessageWindowList> sMsgWindows;	// List of windows (protected for multi-thread access)

private:
	static cdstring	sLastCopyTo;						// Cache last selected copy to
	CMessage*		mItsMsg;							// The message for this window
	bool			mWasUnseen;							// Was it unseen before being displayed
	bool			mItsMsgError;						// Flag for cleared out message
	CMessageList*	mMsgs;								// Used for multi-save operation
	const char*			mShowText;							// Text to show
	std::auto_ptr<unichar_t>	mUTF16Text;						// UTF16 text
	std::auto_ptr<unichar_t>	mRawUTF16Text;					// Raw UTF16 text
	LPrintSpec*		mPrintSpec;
	LPrintSpec		mMyPrintSpec;
	bool			mShowHeader;						// Show header
	bool			mShowSecure;						// Show verify/decrypt area
	bool			mSecureMulti;						// Multi-line verify/decrypt area
	EView			mParsing;							// Parse
	long			mFontScale;							// Font scale factor
	long			mQuoteDepth;						// Quote depth

	CToolbarView*		mToolbarView;					// The toolbar view
	CMessageSimpleView*	mView;							// The view controlling the main toolbar

	LView*			mHeaderMove;						// Pane contain all controls
	LView*			mFromMove;							// Pane enclosing from
	CAddressText*	mFromEdit;							// Multiple addresses of From
	LDisclosureTriangle*	mFromTwister;				// Twister for From field
	LView*			mToMove;							// Pane enclsoing to
	CAddressText*	mToEdit;							// Multiple addresses of To
	LDisclosureTriangle*	mToTwister;					// Twister for To field
	LView*			mCCMove;							// Pane enclsoing cc
	CAddressText*	mCCEdit;							// Multiple addresses of CC
	LDisclosureTriangle*	mCCTwister;					// Twister for CC field
	LView*			mSubjectMove;						// Pane enclsoing subject & date
	CTextFieldX*	mSubjectField;						// Message subject
	CTextFieldX*	mDateField;							// Message date
	CStaticText*	mNumberField;						// Message number
	LView*			mAttachMove;						// Pane to show when attachments exist
	LDisclosureTriangle*	mAttachTwister;				// Twister for attachment table
	CStaticText*	mAttachNumberField;					// Number of attachments
	LBevelButton*	mFlatHierarchyBtn;					// Flat/hierarchy button
	LIconControl*	mAttachmentsIcon;					// Indicates message has attachments
	LIconControl*	mAlternativeIcon;					// Indicates message is text alternative only
	LBevelButton*	mHeaderBtn;							// The header toggle button
	LPopupButton*	mParseLevel;						// User selected parse level justin
	LBevelButton*	mFontIncreaseBtn;					// Font increase button
	LBevelButton*	mFontDecreaseBtn;					// Font decrease button
	CStaticText*	mFontScaleField;					// Font scaling size
	LPopupButton*	mQuoteDepthPopup;					// User selected quote depth

	CSplitterView*	mSplitter;							// Splitter window

	LView*			mAttachArea;						// Pane to hide when twisted up
	CBodyTable*		mAttachTable;						// List of attachments

	LView*			mBottomPane;						// Pane containing bottom splitter

	LView*			mSecurePane;						// Pane contain verify/decrypt details
	CTextDisplay*	mSecureInfo;						// Verify/decrypt details

	LView*			mTextPane;							// Pane containing bottom splitter
	CFormattedTextDisplay*	mText;						// The text
	CWebKitControl*	mHTML;								// HTML text

	bool			mRedisplayBlock;					// Prevent redisplay
	bool			mAllowDeleted;						// Allow deleted while constructing
	CAttachment*	mCurrentPart;
	bool			mDidExpandParts;					// Expand parts was done previously
	bool			mDeleteBtnActive;					// Protect against delete self

	SHeaderState	mHeaderState;						// State of items in header
	
	static cdstring	sNumberString;						// sprintf string for message number
	static cdstring	sNumberStringBig;					// sprintf string for message number >= 10000
	static cdstring	sSubMessageString;					// sprintf string for sub-message
	static cdstring	sDigestString;						// sprintf string for digest

public:
	enum { class_ID = class_MessageWindow };

					CMessageWindow();
					CMessageWindow(LStream *inStream);
	virtual 		~CMessageWindow();


protected:
	virtual void	FinishCreateSelf();					// Do odds & ends

public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the icon buttons

	static  void	ServerState(bool logon);					// Server logs on/off

	virtual void	Activate();

			void	SetTwistList(CAddressText* aEdit,
									LDisclosureTriangle* aTwister,
									CAddressList* list);	// Add address list to caption/field
			void	SetMessage(CMessage* theMsg);			// Set the message
			void	ClearMessage();							// Clear out the message
			void	SetMessageList(CMessageList* msgs)		// Set the message list
						{ mMsgs = msgs; }
			void	PostSetMessage();						// Processing after message and display update
			void	MessageChanged();						// Someone else changed this message
			void	ShowPart(CAttachment* attach);			// Show a particular part
			void	ShowSubMessage(CAttachment* attach,
							Rect zoom_from);				// Display a sub-message
			void	ExtractPart(CAttachment* part,			// Extract/view a particular part
									bool view);
			bool	ShowRawBody();							// Read in raw body

	virtual void	SetPrintSpec(LPrintSpec* printSpec);	// Set a print spec
			void	Text2UTF16(i18n::ECharsetCode charset);	// Convert text to utf16
			void	ResetText();							// Reset message text
			void	ResetFontScale();						// Reset font scale text
public:
			CMessage* GetMessage()							// Get message
		{ return mItsMsg; };
			const CAttachment* GetCurrentPart() const
		{ return mCurrentPart; }


	static	CMessageWindow*	FindWindow(const CMessage* theMsg,	// Find window belonging to message
										bool owned_by = false);
	static bool	WindowExists(const CMessageWindow* wnd);	// Check for window

	virtual void	SaveThisMessage();				// Save this message to file
	virtual void	DoSave(PPx::FSObject &inFileSpec,
								OSType inFileType);		// Save the message from filespec
	const unichar_t* GetSaveText(void);					// Get text to save

	LPrintSpec*		GetPrintSpec()
		{ return mPrintSpec ? mPrintSpec : &mMyPrintSpec; }

	virtual void	DoPrint();						// Print the message

			void	OnEditSpellCheck();				// Spell check message

			void	SpeakMessage();					// Speak whole message

			bool	SpacebarScroll(bool shift_key);					// Scroll the text if possible

private:
			CBodyTable*		GetPartsTable() const
				{ return mAttachTable; }

			void	DoNewLetter(bool option_key);			// Display a new draft
			void	OnMessageReadPrev();					// Read previous message
			void	OnMessageReadNext();					// Read next message
			void	ReadNextMessage(bool delete_it, bool copy_it, bool option_key);				// Read next message
			void	OnMessageDeleteRead();					// Delete then read next message
			void	CopyReadNextMessage(bool option_key);	// Copy then read next message
			bool	RedisplayMessage(CMessage* theMsg);		// Display message in existing window
			void	DoRollover(CMbox* current_mbox);		// Attempt to rollover to next unseen

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

			void	OnHeaderExpand();

			void	OnFromTwist();							// From twister clicked
			void	OnToTwist();							// To twister clicked
			void	OnCcTwist();							// Cc twister clicked
			void	DoTwist(LView* mover,					// Handle twist
							CAddressText* aEdit,
							LDisclosureTriangle* aTwister);

			void	LayoutHeader();							// Layout header component based on state

			void	ShowSecretPane(bool show);					// Show/hide verify/decrypt area
			void	SetSecretPane(const CMessageCryptoInfo& info);		// Set details for decrypt/verify

			void	OnViewAs(EView view);

			void	OnIncreaseFont();						// Increase displayed font size
			void	OnDecreaseFont();						// Decrease displayed font size

			void	OnQuoteDepth(EQuoteDepth quote);		// Change visible quote depth

public:
	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits,
									const TextTraitsRecord& display_traits);	// Reset text traits

private:
			void	InitMessageWindow();
	virtual void	ResetState();					// Reset state from prefs
	virtual void	SaveDefaultState();				// Save state as default

// Drag methods
private:
	virtual Boolean CheckIfViewIsAlsoSender(DragReference inDragRef);	// Check to see whether drop from another window

protected:
	virtual Boolean	ItemIsAcceptable(DragReference inDragRef,
								ItemReference inItemRef);	// Check its a message

	virtual void	ReceiveDragItem(DragReference inDragRef,
								DragAttributes inDragAttrs,
								ItemReference inItemRef,
								Rect &inItemBounds);		// Receive message
	virtual void	HiliteDropArea(DragReference inDragRef);	// Hilite/unhilite region border

};

#endif
