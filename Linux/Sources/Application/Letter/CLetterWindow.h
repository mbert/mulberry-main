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

#include "CFileDocument.h"

#include "CListener.h"
#include <JXIdleTask.h>

#include "CAttachmentList.h"

#include "CMessageWindow.h"

#include "cdstring.h"
#include "cdmutexprotect.h"

#include "UNX_LUndoer.h"

#include "CIdentity.h"
#include "CMIMESupport.h"

// Classes
class CAddressDisplay;
class CAddressList;
class CDataAttachment;
class CFileTable;
class CLetterHeaderView;
class CLetterTextEditView;
class CLetterView;
class CMailboxPopup;
class CMbox;
class CMboxProtocol;
class CMessage;
class CMessageList;
class CSimpleTitleTable;
class CSplitterView;
class CStyleToolbar;
class CToolbarView;
class CTwister;

class JXWidgetSet;
class JSimpleProcess;

class CLetterWindow : public CFileDocument,
						public JXIdleTask,
						public CListener

{
	typedef CFileDocument super;
	typedef CLetterWindow CLetterDoc;
	friend class CLetterHeaderView;
	friend class CLetterTextEditView;
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
	static cdmutexprotect<CLetterWindowList> sLetterWindows;				// List of windows (protected for multi-thread access)

	CLetterWindow(JXDirector* owner);
	virtual ~CLetterWindow();
	static CLetterWindow* ManualCreate();
	static bool OpenDraft();
	static bool OpenDraft(const char* fpath);
	static bool	WindowExists(const CLetterWindow* wnd);	// Check for window

	virtual void			SaveTemporary();
	static  unsigned long	SaveAllTemporary();
	virtual void			DeleteTemporary();
	static	void			ReadTemporary();
	static	void			MakeTemporary(const cdstring& path, unsigned long ctr);
	virtual	void			Perform(const Time delta, Time* maxSleepTime);
	void			ResetAutoSaveTimer();
	void			PauseAutoSaveTimer(bool pause);


	CLetterDoc* GetDocument()
		{ return this; }
	virtual void OnCreate();

	virtual void Activate();
	virtual void WindowFocussed(JBoolean focussed);

	virtual CLetterTextEditView* GetTextDisplay()
						{ return mText; }
	virtual LUndoer* GetUndoer()
						{ return &mUndoer; }

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes
			void	ServerState(CMboxProtocol* proto, bool logon);					// Server logs on/off
			void	MailboxState(const CMbox* mbox);										// Mailbox closed

	bool	TestOwnership(const CMbox* mbox, bool replies = true) const;		// Check whether mbox in use in draft
	void	MessageRemoved(const CMessage* msg);			// Message removed

	void	SetTwistList(CAddressDisplay* aField,						// Add address list to caption/field
							 CTwister* aTwister,
							 const CAddressList* list);

	void	SetAddressLists(const CAddressList* to,						// Add address lists
								const CAddressList* cc,
								const CAddressList* bcc);
	void	GetAddressLists(CAddressList*& to,							// Set address lists
								CAddressList*& cc,
								CAddressList*& bcc);
	void	AddAddressLists(const CAddressList* to,						// Add address lists
								const CAddressList* cc,
								const CAddressList* bcc);
	void	RemoveAddressLists(const CAddressList* to,					// Remove address lists
								 const CAddressList* cc,
								 const CAddressList* bcc);
	
	void	SetReplyMessage(CMessage* theMsg, EReplyTo reply_to);			// Set the reply to message
	void	SetReplyMessages(CMessageList* msgs, EReplyTo reply_to);		// Set the reply to message
	void	SetReplyMessage(CMessage* theMsg,								// Set the reply to message
								const CAddressList* reply_to,
								const CAddressList* reply_cc,
								const CAddressList* reply_bcc);
	void	SetReplyMessages(CMessageList* msgs,							// Set the reply to message
								 const CAddressList* reply_to,
								 const CAddressList* reply_cc,
								 const CAddressList* reply_bcc);
	void	SetForwardMessage(CMessage* theMsg,			// Set the forwarded message
								EForwardOptions forward);
	void	SetForwardMessages(CMessageList* msgs,		// Set the forwarded message
								EForwardOptions forward);
	void	SetBounceMessage(CMessage* theMsg);			// Set the bounced message
	void	SetBounceMessages(CMessageList* msgs);		// Set the bounced message
	void	SetRejectMessage(CMessage* msg,				// Set the rejected message
								bool return_msg);
	void	SetRejectMessages(CMessageList* msgs,		// Set the rejected messages
								bool return_msg);
	void	SetSendAgainMessage(CMessage* theMsg);		// Set the send again message
	void	SetSendAgainMessages(CMessageList* msgs);	// Set the send again message
	void	SetDigestMessage(CMessage* theMsg);			// Set the digest message
	void	SetDigestMessages(CMessageList* msgs);		// Set the digest message
	void	PostSetMessage(const CIdentity* id = NULL);	// Do common bits after setting a message
	
	const char* QuoteText(const char* theText,
							bool forward, bool header = false, bool is_flowed = false);
	void	IncludeMessage(CMessage* theMsg,				// Include message selection indented
							 bool forward,
							 int start = 0,
							 int length = 0,
							 bool showHeader = false);
	
	void 	IncludeMessageTxt(CMessage* theMsg, 
								const char* header,
								const char* txt,
								bool forward,
								EContentSubType type = eContentSubPlain,
								bool is_flowed = false);
	void	IncludeFromReply(bool forward,			// Include repling to mail
								bool header);
	
	void	SetText(const char* text, bool plain = false);		// Set text
	void 	SetSubject(const cdstring& text);					// Set Subject
	void 	GetSubject(cdstring& text) const;					// Get Subject
	void 	SetFiles(const cdstrvect& files);					// Set files
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
	void	SyncPart();											// Sync text in current part
	void	SyncPartSize();										// Sync size of text in current part
		
	void	InitIdentity(const CIdentity* id);					// Init identity
	void	SyncIdentities(const CIdentityList& ids);			// Handle change of identities
	void	SetIdentityPopup(unsigned long index);				// Change identity popup
	void	OnChangeIdentity(JIndex nID);						// Handle identities popup command
	void	OnEditCustom();										// Edit custom identity
	const CIdentity* GetIdentity() const;						// Get current identity
	void	SetIdentity(const CIdentity* identity,				// Change identity
						bool custom = false,
						bool add_signature = true);
	void ResetFont(const SFontInfo& list_traits, const SFontInfo& display_traits);	
	
	virtual bool	IsDirty();								// Get dirty state
	virtual void	SetDirty(bool dirty);					// Set dirty state
	
	void ReadTextFile(std::istream& input);
	virtual void WriteTextFile(std::ostream& output, const JBoolean safetySave) const;
	virtual void	SafetySave(const JXDocumentManager::SafetySaveReason reason);
	virtual bool	AbortSafetySave(const char* path);
	
	// Common updaters
	void	OnUpdateNotReject(CCmdUI* pCmdUI);
	void	OnUpdateFileImport(CCmdUI* pCmdUI);
	void	OnUpdateEditUndo(CCmdUI* pCmdUI);
	void	OnUpdatePasteIndented(CCmdUI* pCmdUI);
	void	OnUpdateAllowStyled(CCmdUI* pCmdUI);
	void	OnUpdateDraftInclude(CCmdUI* pCmdUI);
	void	OnUpdateDraftAddCc(CCmdUI* pCmdUI);
	void	OnUpdateDraftAddBcc(CCmdUI* pCmdUI);
	void	OnUpdateDraftNotReject(CCmdUI* pCmdUI);
	void	OnUpdateTextStyle(CCmdUI* pCmdUI);
	void	OnUpdateDraftTextActive(CCmdUI* pCmdUI);
	void	OnUpdateDraftAttachFile(CCmdUI* pCmdUI);
	void	OnUpdateDraftSendMail(CCmdUI* pCmdUI);
	void	OnUpdateDraftAppend(CCmdUI* pCmdUI);
	void	OnUpdateDraftCopyOriginal(CCmdUI* pCmdUI);

	void	OnUpdateDraftSign(CCmdUI* pCmdUI);
	void	OnUpdateDraftEncrypt(CCmdUI* pCmdUI);

	void	OnUpdateDraftExternalEdit(CCmdUI* pCmdUI);

	void	OnUpdateDraftMDN(CCmdUI* pCmdUI);
	void	OnUpdateDraftDSNSuccess(CCmdUI* pCmdUI);
	void	OnUpdateDraftDSNFailure(CCmdUI* pCmdUI);
	void	OnUpdateDraftDSNDelay(CCmdUI* pCmdUI);
	void	OnUpdateDraftDSNFull(CCmdUI* pCmdUI);

	void	OnUpdateMenuExpandHeader(CCmdUI* pCmdUI);
	void	OnUpdateExpandHeader(CCmdUI* pCmdUI);
	void	OnUpdateWindowsShowParts(CCmdUI* pCmdUI);

	void	OnUpdateAdminLockDraftSave(CCmdUI* pCmdUI);

	// Command handlers
	void	OnFileNewDraft();
	void	OnFileSave();
	void	OnFileSaveAs();
	bool	DoFileSave();
	void	DoMailboxSave();
	void	OnFileImport();

	void	OnEditUndo();
	void	OnPasteIndented();
	void	OnEditSpellCheck();
	bool	DoSpellCheck(bool show_alert);

	void	OnDraftInclude();
	void	OnDraftInsertSignature();
	void	ReplaceSignature(const cdstring& old_sig, const cdstring& new_sig);
	void	NormaliseSignature(cdstring& old_sig);
	void	InsertSignature(const cdstring& new_sig);			// Insert current signature
	void	OnDraftAttachFile();
	void	OnDraftNewPlainPart();
	void	OnDraftNewEnrichedPart();
	void	OnDraftNewHTMLPart();
	void	OnDraftMultipartMixed();
	void	OnDraftMultipartParallel();
	void	OnDraftMultipartDigest();
	void	OnDraftMultipartAlternative();
	void	OnDraftCurrentText(unsigned long cmd);
	void	OnDraftSendMail();
	void	OnDraftAppendNow();									// Copy it to mailbox now
	void	OnDraftAppendNowBtn();								// Copy it to mailbox now
	void	OnDraftAppendNowOptionBtn();						// Copy it to mailbox now
	void	OnDraftAppend(JXTextMenu* menu, JIndex nID);		// Copy it to mailbox now
	void	OnDraftCopyOriginal();

	void	OnDraftSign();
	void	OnDraftEncrypt();

	void	OnDraftMDN();
	void	OnDraftDSNSuccess();
	void	OnDraftDSNFailure();
	void	OnDraftDSNDelay();
	void	OnDraftDSNFull();

	void	OnLetterHeaderTwister();

	void	OnLetterToTwister();
	void	OnLetterCCTwister();
	void	OnLetterBCCTwister();
	void	OnLetterCopyToPopup(JIndex nID);					// Copy to popup change
	void	OnLetterSign();
	void	OnLetterEncrypt();

	void	DoTwist(CAddressDisplay* aField,					// Handle twist
								CTwister* aTwister,
								JXDecorRect* mMove);

	void	OnDraftAddCc();											// Add Cc button clicked
	void	OnDraftAddBcc();											// Add Bcc button clicked

	void	LayoutHeader();										// Layout header component based on state
	void	UpdateRecipients();									// Update recipients caption

	void	OnLetterPartsTwister();
	bool	IsPartsTwist() const;
	void	DoPartsTwist(bool expand, bool change_focus);

	void	SetToolbar(const CMIMEContent& mime);
	void	SetToolbar(EContentSubType type);
	void	ShowToolbar(EContentSubType type, bool override = false);
	void	HideToolbar(bool override = false);

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveDefaultState();						// Save current state as default

			void	AddPrintSummary(const CMessage* msg);			// Temporarily add header summary for printing
			void	RemovePrintSummary(const CMessage* msg);		// Remove temp header summary after printing

	void	OnUpdateStyle(JXWidget* pWidget);
	void	OnUpdateAlignment(JXWidget* pWidget);
	void	OnUpdateFont(JXWidget* pWidget);
	void	OnUpdateFontSize(JXWidget* pWidget);
	void	OnUpdateColor(JXWidget* pWidget);
	
	void 	OnStyle(JIndex nID);
	void 	OnAlignment(JIndex nID);
	void 	OnFont(JIndex nID);
	void 	OnFontSize(JIndex nID);
	void 	OnColor(JIndex nID);

private:
	static UInt32 mDraftNum; //which draft are we up to (for default name)
	static unsigned long	sTemporaryCount;
	cdstring				mTemporary;
	time_t					mAutoSaveTime;						// Time at which to trigger auto save

	void NextDefaultDraftName(JString& name);//fills in name (Draft1,etc)
	JString mDraftName;

	SHeaderState			mHeaderState;						// State of items in header

	CToolbarView*			mToolbarView;						// The toolbar view
	CLetterView*			mView;								// The view that controls the main toolbar

	CSplitterView*			mSplitterView;						// Splitter window
	CLetterHeaderView*		mHeader;

	JXScrollbarSet*			mPartsScroller;				// Parts scroller
	CSimpleTitleTable*		mPartsTitles;				// Parts titles
	CFileTable*				mPartsTable;				// List of parts

    JXFlatRect*         	mBottomPane;
	CStyleToolbar*			mToolbar;
	CLetterTextEditView*	mText;

	bool					mStyledVisible;
	bool					mSaveToMailbox;
	cdstring				mMailboxSave;

	CMessageList*			mMsgs;								// The associated messages for this window
	cdstring				mIdentity;							// Identity to use when sending mail
	bool					mCustomIdentity;					// Use custom identity from popup
	bool					mDoSign;							// Crypto sign this message
	bool					mDoEncrypt;							// Crypto encrypt this message
	CDSN					mDSN;								// DSN to use when sending mail

	CAttachment*			mCurrentPart;						// Text part on display
	CAttachment*			mBody;								// Parts list for message
	bool					mSentOnce;							// Has been sent
	bool					mDirty;								// Window is dirty
	bool					mReply;								// Message is being forwarded
	cdstring				mReplySubject;						// Original subject of reply
	bool					mForward;							// Message is being forwarded
	bool					mBounce;							// Message is being bounced
	bool					mReject;							// Message is being rejected
	bool					mSendAgain;							// Message is being bounced
	bool					mOriginalEncrypted;					// Original content was encrypted
	bool					mMarkSaved;							// Has been saved at least once
	bool					mExternalEdit;						// In external edit mode
	char*					mBounceHeader;						// Bouncing message's header
	static cdstring			sLastAppendTo;						// Cache last selected append to
	cdstring				mSavedAppendMbox;					// Name of mailbox saved while server logged off
	cdstring				mSignature;							// Last signature inserted
	long					mSignatureLength;					// Length of last signature inserted
	bool					mCopyOriginalAllowed;				// Copy original allowed
	bool					mCopyOriginal;						// Copy original state
	LUndoer					mUndoer;							// Undoable actions
	cdstring				mOriginalTitle;						// Original document title
	
	void	IncludeText(const char* theText, bool forward, bool adding = false);	// Include some text
	void	UpdatePartsCaption();					// Update parts caption

	CMessage*	CreateMessage(bool alert,				// Create message from contents
								bool bcc_only = false);
	bool	ValidAddressList(const CAddressList* addrs,		// Check that all addresses are valid format
								const char* str_id1, const char* strid2) const;
	void	CopyNow(bool option_key);					// Copy it to mailbox now
	void	CopyNow(CMbox* mbox, bool option_key);		// Copy it to mailbox now
	
	void	DraftSaved();								// Draft was saved in some fashion
			
	bool	MissingAttachments();						// Check for missing attachments
	bool	UnencryptedSend();							// Check for unencrypted send of originally encrypted content
	
	void			RefreshToolbar();
	CMailboxPopup*	GetAppendPopup();
	CFileTable*		GetPartsTable();
	void			ShowSentIcon();
	bool			DoAppendReplied() const;
	bool			IsSigned() const;
	bool			IsEncrypted() const;

	// message handlers
	void OnChangeSubject();
protected:
	JSimpleProcess*	mExternalEditProcess;
	cdstring		mExternalEditFile;

			void OnExternalEdit();
			void OnExternalEditEnd();

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual CCommander* GetTarget();
};

#endif
