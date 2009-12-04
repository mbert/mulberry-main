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


// Header for CLetterWindow class

#ifndef __CLETTERWINDOW__MULBERRY__
#define __CLETTERWINDOW__MULBERRY__

#include "LWindow.h"
#include "CWindowStatus.h"
#include "CListener.h"

#include "CDSN.h"
#include "CIdentity.h"
#include "CLetterDoc.h"
#include "CMessageFwd.h"
#include "CMessageWindow.h"
#include "CAttachmentList.h"

#include "CMIMESupport.h"

#include "cdmutexprotect.h"

// Constants

// Panes
const	ClassIDT	class_LetterWindow ='MaWi';
const	PaneIDT		paneid_LetterWindow = 3000;
const	PaneIDT		paneid_LetterToolbarView = 'TBar';
const	PaneIDT		paneid_LetterView = 'VIEW';
const	PaneIDT		paneid_LetterHeader = 'MHDR';
const	PaneIDT		paneid_LetterFromMove = 'IDMV';
const	PaneIDT		paneid_LetterFromView = 'FROM';
const	PaneIDT		paneid_LetterIdentities = 'IDTS';
const	PaneIDT		paneid_LetterIdentityEdit = 'IEDT';
const	PaneIDT		paneid_LetterRecipientView = 'RECI';
const	PaneIDT		paneid_LetterRecipientText = 'IRCP';
const	PaneIDT		paneid_LetterToMove = 'TOMV';
const	PaneIDT		paneid_LetterToTwister = 'TOTW';
const	PaneIDT		paneid_LetterToX = 'TOST';
const	PaneIDT		paneid_LetterCCMove = 'CCMV';
const	PaneIDT		paneid_LetterCCTwister = 'CCTW';
const	PaneIDT		paneid_LetterCCX = 'CCST';
const	PaneIDT		paneid_LetterBCCMove = 'BCMV';
const	PaneIDT		paneid_LetterBCCTwister = 'BCTW';
const	PaneIDT		paneid_LetterBCCX = 'BCST';
const	PaneIDT		paneid_LetterSubjectMove = 'SUMV';
const	PaneIDT		paneid_LetterSubject = 'SUBJ';
const	PaneIDT		paneid_LetterSentIcon = 'SENT';
const	PaneIDT		paneid_LetterAppendList = 'LIST';
const	PaneIDT		paneid_LetterPartsMove = 'PARV';
const	PaneIDT		paneid_LetterPartsCaptionP = 'ATTP';
const	PaneIDT		paneid_LetterPartsCaptionA = 'ATTA';
const	PaneIDT		paneid_LetterPartsNumber = 'ANUM';
const	PaneIDT		paneid_LetterPartsTwister = 'ATTW';
const	PaneIDT		paneid_LetterPartsIcon = 'PAR1';
const	PaneIDT		paneid_LetterPartsHide = 'ATHD';
const	PaneIDT		paneid_LetterPartsBackground = 'AHDR';
const	PaneIDT		paneid_LetterPartsScroller = 'SATT';
const	PaneIDT		paneid_LetterPartsTitles = 'TITL';
const	PaneIDT		paneid_LetterPartsTable = 'ATTM';
const	PaneIDT		paneid_LetterSplit = 'SPLT';
const	PaneIDT		paneid_LetterTextArea = 'VTXT';
const	PaneIDT		paneid_LetterEnrichedTools = 'RICH';
const	PaneIDT		paneid_LetterScroller = 'STXT';
const	PaneIDT		paneid_LetterText = 'MTXT';

const 	PaneIDT		paneid_LetterEnriched = 'RICH';

// Resources
const	ResIDT		RidL_CLetterBtns = 3000;
const	ResIDT		Txtr_DefaultLetter = 1000;

const	ResIDT		cctb_LetterControls = 3000;

// Messages
const	MessageT	msg_IdentityEdit = 'IEDT';
const	MessageT	msg_AppendTo = 3003;
const	MessageT	msg_TwistTo = 3004;
const	MessageT	msg_TwistCC = 3005;
const	MessageT	msg_TwistBCC = 3006;
const	MessageT	msg_TwistAttach = 3007;
const	MessageT	msg_Identities = 'IDTS';
const	MessageT	msg_LtrHeaderExpand = 'PLUS';
const	MessageT	msg_LtrHeaderCollapse = 'MINS';

// Others
const	SInt16		cToTwistMove = 38;
const	SInt16		cCCTwistMove = 38;
const	SInt16		cBCCTwistMove = 38;

// Types
struct SLetterTraits {
	Rect			bounds;
};

// Classes
class CFileTable;
class LCheckBox;
class LDisclosureTriangle;
class LBevelButton;
class LIconControl;
class LPictureControl;
class LPopupButton;

class CAddressDisplay;
class CAddressList;
class CDataAttachment;
class CIdentityPopup;
class CLetterView;
class CMailboxPopup;
class CMboxProtocol;
class CMessage;
class CMessageList;
class CSplitDivider;
class CStaticText;
class CTextFieldX;
class CTextDisplay;
class CEditFormattedTextDisplay;
class CBackView;
class CSimpleTitleTable;
class CSplitterView;
class CStyleToolbar;
class CToolbarView;

class	CLetterWindow : public LWindow,
						 public LListener,
						 public LDragAndDrop,
						 public CWindowStatus,
						 public CListener
{
	friend class CLetterDoc;
	friend class CLetterTextDisplay;
	friend class CPasteQuoteAction;
	friend class CIncludeReplyAction;
	friend class CFileTable;

	struct SHeaderState
	{
		bool	mExpanded;
		bool	mToVisible;
		bool	mToExpanded;
		bool	mCcVisible;
		bool	mCcExpanded;
		bool	mBccVisible;
		bool	mBccExpanded;
		bool	mSubjectVisible;
	};

public:
	typedef std::vector<CLetterWindow*>	CLetterWindowList;
	static cdmutexprotect<CLetterWindowList> sLetterWindows;	// List of windows (protected for multi-thread access)
	static short		sLetterWindowCount;						// Total number of letters so far
private:
	static cdstring		sLastAppendTo;						// Cache last selected append to
	CMessageList*		mMsgs;								// The associated message list for this window

	cdstring			mIdentity;							// Identity to use when sending mail
	bool				mCustomIdentity;					// Use custom identity from popup
	bool				mDoSign;							// Crypto sign this message
	bool				mDoEncrypt;							// Crypto encrypt this message
	CDSN				mDSN;								// DSN to use when sending mail

	CToolbarView*		mToolbarView;						// The toolbar view
	CLetterView*		mView;								// The view that controls the main toolbar

	LView*				mHeaderMove;						// Pane contain all header controls
	LView*				mFromMove;							// Pane enclosing identities
	LView*				mFromView;							// Pane enclosing from
	CIdentityPopup*		mIdentities;						// Identities popup
	LBevelButton*		mIdentityEditBtn;					// The identity edit button
	LView*				mRecipientView;						// Pane enclosing recipients
	CStaticText*		mRecipientText;						// Summary of recipients
	
	LView*				mToMove;							// Pane enclosing To
	CAddressDisplay*	mToEdit;							// Multiple addresses of To
	LDisclosureTriangle*	mToTwister;						// Twister for To field
	LView*				mCCMove;							// Pane enclosing CC
	CAddressDisplay*	mCCEdit;							// Multiple addresses of CC
	LDisclosureTriangle*	mCCTwister;						// Twister for CC field
	LView*				mBCCMove;							// Pane e3nclosing BCC
	CAddressDisplay*	mBCCEdit;							// Multiple addresses of BCC
	LDisclosureTriangle*	mBCCTwister;					// Twister for BCC field
	LView*				mSubjectMove;						// Pane enclosing subject
	CTextFieldX*		mSubjectField;						// Subject
	LIconControl*		mSentIcon;							// Indicates that message has been sent
	CMailboxPopup*		mAppendList;						// List of mailboxes for append
	LView*				mPartsMove;							// Pane to show when attachments exist
	LDisclosureTriangle*	mPartsTwister;					// Twister for attachment table
	CTextFieldX*		mPartsNumber;						// Number of attachments
	LIconControl*		mPartsIcon;							// Indicates that message has parts

	CSplitterView*		mSplitter;							// Splitter window

	LView*				mPartsArea;							// Pane to hide when twisted up
	CSimpleTitleTable*	mPartsTitles;						// List of attachments
	CFileTable*			mPartsTable;						// List of attachments

	LView*				mTextArea;							// The entire text area
	CEditFormattedTextDisplay*		mText;					// The text
	CAttachment*		mCurrentPart;						// Text part on display
	CAttachment*		mBody;								// Parts list for message

	SHeaderState		mHeaderState;						// State of items in header

	bool				mSentOnce;							// Has been sent
	bool				mDirty;								// Window is dirty
	bool				mReply;								// Message is being replied
	cdstring			mReplySubject;						// Original subject of reply
	bool				mForward;							// Message is being forwarded
	bool				mForwardQuote;						// Quote as forward
	bool				mBounce;							// Message is being bounced
	bool				mReject;							// Message is being rejected
	bool				mSendAgain;							// Message is being sent again
	bool				mOriginalEncrypted;					// Original content was encrypted
	bool				mMarkSaved;							// Has been saved at least once
	char*				mBounceHeader;						// Bouncing message's header
	cdstring			mSavedAppendMbox;					// Name of append mailbox while server logged out
	cdstring			mSignature;							// Last signature inserted
	long				mSignatureLength;					// Length of last signature inserted
	bool				mCopyOriginalAllowed;				// Copy original allowed
	bool				mCopyOriginal;						// Copy original state
	cdstring			mOriginalTitle;						// Original document title

	CStyleToolbar*		mToolbar;							// Panel of Style commands

public:
	enum { class_ID = class_LetterWindow };

					CLetterWindow();
					CLetterWindow(LStream *inStream);
	virtual 		~CLetterWindow();

	static CLetterWindow*	ManualCreate();

	static bool	WindowExists(const CLetterWindow* wnd);	// Check for window

private:
			void	InitLetterWindow(void);					// Common init

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to clicks in the icon buttons
	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

			void	ServerState(CMboxProtocol* proto, bool logon);					// Server logs on/off
			void	MailboxState(const CMbox* mbox);								// Mailbox closed

	virtual void	Activate();
	virtual void	Deactivate();

	virtual void	SetDirty(bool dirty);					// Set dirty state
	virtual bool	IsDirty(void);								// Get dirty state

	CLetterDoc*		GetDocument()
		{ return static_cast<CLetterDoc*>(GetSuperCommander()); }

			bool	TestOwnership(const CMbox* mbox, bool replies = true) const;		// Check whether mbox in use in draft
			void	MessageRemoved(const CMessage* msg);				// Message removed

			void	SetTwistList(CAddressDisplay* aEdit,
									LDisclosureTriangle* aTwister,
									const CAddressList* list);			// Add address list to caption/field

			void	SetAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Set address lists
			void	GetAddressLists(CAddressList*& to,
									CAddressList*& cc,
									CAddressList*& bcc);				// Set address lists
			void	AddAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Add address lists
			void	RemoveAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Remove address lists

			void	SetReplyMessage(CMessage* msg,
									EReplyTo reply_to);			// Set the reply to message
			void	SetReplyMessages(CMessageList* msgs,
									EReplyTo reply_to);			// Set the reply to messages
			void	SetReplyMessage(CMessage* msg,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc);	// Set the reply to messages
			void	SetReplyMessages(CMessageList* msgs,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc);	// Set the reply to messages
			void	SetForwardMessage(CMessage* msg,				// Set the forwarded message
										EForwardOptions forward);
			void	SetForwardMessages(CMessageList* msgs,			// Set the forwarded messages
										EForwardOptions forward);
			void	SetBounceMessage(CMessage* msg);			// Set the bounced message
			void	SetBounceMessages(CMessageList* msgs);		// Set the bounced messages
			void	SetRejectMessage(CMessage* msg,				// Set the rejected message
										bool return_msg);
			void	SetRejectMessages(CMessageList* msgs,		// Set the rejected messages
										bool return_msg);
			void	SetSendAgainMessage(CMessage* msg);			// Set the send again message
			void	SetSendAgainMessages(CMessageList* msgs);	// Set the send again messages
			void	SetDigestMessage(CMessage* msg);			// Set the digest message
			void	SetDigestMessages(CMessageList* msgs);		// Set the digest messages
			void	PostSetMessage(const CIdentity* id = NULL);	// Do common bits after setting a message

			void 	SetSubject(const cdstring& theSubject);		// Set the subject
			void 	GetSubject(cdstring& theSubject) const;		// Get the subject
			void 	SetFiles(const cdstrvect& files);			// Set file attachments
			void 	SetiTIP(const cdstring& subject,
							const cdstring& description,
							CDataAttachment* attach,
							CMessage* msg);						// Set iTIP attachment

			void	SetBody(CAttachment* attach,				// Set body content
								bool converting = false);
			CAttachment* GetBody() const
			{
				return mBody;
			}

			CAttachment* GetCurrentPart() const
			{
				return mCurrentPart;
			}
			void	SetCurrentPart(CAttachment* part);			// Show a particular part
			void	SyncPart(void);								// Sync text in current part
			void	SyncPartSize(void);							// Sync size of text in current part

			void	DoStyleChangePopup(long index);				// Handle style change popup command

			void	InitIdentity(const CIdentity* id);			// Init identity
			void	SyncIdentities(const CIdentityList& ids);	// Handle change of identities
			void	SetIdentityPopup(unsigned long pos);		// Set identity popup value
			void	DoIdentitiesPopup(long index);				// Handle identities popup command
			void	OnEditCustom();								// Edit custom identity
			const CIdentity* GetIdentity() const;				// Get current identity
			void	SetIdentity(const CIdentity* identity,		// Change identity
								bool custom = false,
								bool add_signature = true);
			void	ReplaceSignature(const cdstring& old_sig, const cdstring& new_sig);
			void	NormaliseSignature(cdstring& old_sig);
			void	InsertSignature(const cdstring& new_sig);	// Insert current signature

			void	OnDraftCopyOriginal();
			void	OnDraftSign();
			void	OnDraftEncrypt();

			void	OnDraftMDN();
			void	OnDraftDSNSuccess();
			void	OnDraftDSNFailure();
			void	OnDraftDSNDelay();
			void	OnDraftDSNFull();

protected:
			void			RefreshToolbar();
			CMailboxPopup*	GetAppendPopup()
				{ return mAppendList; }
			CFileTable*		GetPartsTable()
				{ return mPartsTable; }
			void ShowSentIcon();
			bool DoAppendReplied() const;
			bool IsSigned() const;
			bool IsEncrypted() const;

private:
			void	OnDraftSendMail(void);						// Send mail
			CMessage*	CreateMessage(bool send,				// Create message from contents
										 bool bcc_only = false);
			bool	ValidAddressList(const CAddressList* addrs,		// Check that all addresses are valid format
										const char* str_id1, const char* strid2) const;

			void	CopyNow(bool option_key);					// Copy it to mailbox now
			void	CopyNow(CMbox* mbox, bool option_key);		// Copy it to mailbox now

			void	DraftSaved();								// Draft was saved in some fashion
			
			bool	MissingAttachments();						// Check for missing attachments
			bool	UnencryptedSend();							// Check for unencrypted send of originally encrypted content

public:
			bool	DoSpellCheck(bool sending);					// Spell check message

			void	SpeakMessage(void);							// Speak whole message

			void	SetText(const char *theText, bool plain = false);				// Display this text
			void	GetTaggedText(cdstring& txt);				// Get the tagged text for saving....
			CEditFormattedTextDisplay*	GetTextDisplay(void)
						{ return mText; }						// Get text display

			void	SetEnvelope(Handle theText, long length);	// Copy envelope
			void	GetEnvelope(Handle& theText, long& length);	// Get envelope

			void	SetTraits(SLetterTraits& traits);			// Set display traits
			void	GetTraits(SLetterTraits& traits);			// Get display traits

			void	IncludeMessage(CMessage* theMsg,		// Include message selection indented
									bool forward,
									bool header);
			void	IncludeMessageTxt(CMessage* theMsg,
									const char* header,
									const char* txt,
									bool forward,
									EContentSubType type = eContentSubPlain,
									bool is_flowed = false);	// Include message selection indented
			void	IncludeScrap(void);						// Include some text from scrap
			void	IncludeFromReply(bool forward,			// Include repling to mail
										bool header);

private:
			void	ChangedSubject();

			void	IncludeText(const char* theText,
								bool forward,
								bool adding = false);			// Include some text
			const char*	QuoteText(const char* theText,			// Quote some text
									bool forward, bool header = false, bool is_flowed = false);
			void	DoPartsTwist();							// Handle twist of attachments
			bool	IsPartsTwist() const;
			void	DoPartsTwist(bool expand, bool change_focus);
public:
			void	UpdatePartsCaption(void);				// Update attachments caption
private:
			void	OnHeaderExpand();

			void	OnToTwist();							// To twister clicked
			void	OnCcTwist();							// Cc twister clicked
			void	OnBccTwist();							// Bcc twister clicked
			void	DoTwist(LView* mover,					// Handle twist
							CTextDisplay* aEdit,
							LDisclosureTriangle* aTwister);
			void	OnAddCc();								// Add Cc button clicked
			void	OnAddBcc();								// Add Bcc button clicked

			void	LayoutHeader();							// Layout header component based on state
			void	UpdateRecipients();						// Update recipients caption

public:
			void	ResetTextTraits(const TextTraitsRecord& list_traits,
									const TextTraitsRecord& display_traits);		// Reset text traits
private:
	virtual void	ResetState(void);					// Reset state from prefs
	virtual void	SaveDefaultState(void);				// Save state as default

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
	virtual void	HiliteDropArea(DragReference inDragRef);	// Hilite region border

public:
	void SetToolbar(const CMIMEContent& mime);
	void SetToolbar(EContentSubType type);
	void ShowToolbar(EContentSubType type, bool override=false);
	void HideToolbar(bool override=false);
};

#endif
