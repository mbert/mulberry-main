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

#include "CWindowStatus.h"
#include "CListener.h"
#include "CCommander.h"

#include "CAttachmentList.h"
#include "CFileTable.h"
#include "CLetterView.h"
#include "CLetterTextEditView.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

//#include "CEditFormattedTextDisplay.h"
#include "CMessageWindow.h"

#include "cdstring.h"
#include "cdmutexprotect.h"

#include <WIN_LUndoer.h>

#include "CIdentity.h"
#include "CMIMESupport.h"

// Classes
class CAddressList;
class CDataAttachment;
class CFileTable;
class CMailboxPopup;
class CMbox;
class CMboxProtocol;
class CMessage;
class CMessageList;
class CLetterHeaderView;
class CTwister;

class CLetterWindow : public CWnd,
						public CWindowStatus,
						public CListener,
						public CCommander
{
	DECLARE_DYNCREATE(CLetterWindow)

	friend class CLetterDoc;
	friend class CLetterFrameSDI;
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
	typedef vector<CLetterWindow*>	CLetterWindowList;
	static cdmutexprotect<CLetterWindowList> sLetterWindows;				// List of windows (protected for multi-thread access)
	static CMultiDocTemplate* sLetterDocTemplate;

					CLetterWindow();
	virtual 		~CLetterWindow();
	static CLetterWindow* ManualCreate();
	static void		UpdateUIPos();

	static bool	WindowExists(const CLetterWindow* wnd);	// Check for window

	virtual LUndoer* GetUndoer()
						{ return &mUndoer; }
	virtual void	OnDraw(CDC* pDC) {}

	virtual CDocument* GetDocument()
						{ return mText->m_pDocument; }
	virtual CCmdEditView* GetTextDisplay()
						{ return mText; }

			bool	SafetySave(const cdstring& path);

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes
			void	ServerState(CMboxProtocol* proto, bool logon);					// Server logs on/off
			void	MailboxState(const CMbox* mbox);								// Mailbox closed

			bool	TestOwnership(const CMbox* mbox, bool replies = true) const;		// Check whether mbox in use in draft
			void	MessageRemoved(const CMessage* msg);			// Message removed

			void	SetTwistList(CCmdEdit* aField,
									CTwister* aTwister,
									CWnd* aMover,
									const CAddressList* list);			// Add address list to caption/field

			void	SetAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Add address lists
			void	GetAddressLists(CAddressList*& to,
									CAddressList*& cc,
									CAddressList*& bcc);				// Set address lists
			void	AddAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Add address lists
			void	RemoveAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc);			// Remove address lists

			void	SetReplyMessage(CMessage* theMsg,
									EReplyTo reply_to);			// Set the reply to message
			void	SetReplyMessages(CMessageList* msgs,
									EReplyTo reply_to);			// Set the reply to message
			void	SetReplyMessage(CMessage* theMsg,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc);	// Set the reply to message
			void	SetReplyMessages(CMessageList* msgs,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc);	// Set the reply to message
			void	SetForwardMessage(CMessage* theMsg,				// Set the forwarded message
										EForwardOptions forward);
			void	SetForwardMessages(CMessageList* msgs,			// Set the forwarded message
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
			void	IncludeMessage(CMessage* theMsg,
									bool forward,
									int start = 0,
									int length = 0,
									bool showHeader = false);	// Include message selection indented

			void 	IncludeMessageTxt(CMessage* theMsg, 
										const char* header,
										const char* txt,
										bool forward,
										EContentSubType type = eContentSubPlain,
										bool is_flowed = false);
			void	IncludeFromReply(bool forward,			// Include repling to mail
										bool header);
	
			void	SetText(const char* text, bool plain = false);	// Set text
			void 	SetSubject(const cdstring& text);			// Set Subject
			void 	GetSubject(cdstring& text) const;			// Get Subject
			void 	SetFiles(const cdstrvect& files);			// Set files
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
			void	SyncPart();									// Sync text in current part
			void	SyncPartSize();								// Sync size of text in current part

			void    OnChangeTextStyle(UINT nID);

			void	InitIdentity(const CIdentity* id);					// Init identity
			void	SyncIdentities(const CIdentityList& ids);			// Handle change of identities
			void	SetIdentityPopup(unsigned long index);				// Change identity popup
			void	OnChangeIdentity(UINT nID);							// Handle identities popup command
			void	OnEditCustom();										// Edit custom identity
			const CIdentity* GetIdentity() const;						// Get current identity
			void	SetIdentity(const CIdentity* identity,				// Change identity
								bool custom = false,
								bool add_signature = true);

	virtual void	ResetFont(CFont* list_font,
								CFont* display_font);				// Reset fonts
	virtual void	ResetState();									// Reset state from prefs

			void	AddPrintSummary(const CMessage* msg);			// Temporarily add header summary for printing
			void	RemovePrintSummary(const CMessage* msg);		// Remove temp header summary after printing

	virtual BOOL	IsDirty();								// Get dirty state
	virtual void	SetDirty(bool dirty);					// Set dirty state

	virtual void Serialize(CArchive& ar);						// Read/Save data from/to file

	// Common updaters
	afx_msg void	OnUpdateAlways(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateNotReject(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFileImport(CCmdUI* pCmdUI);
	afx_msg void 	OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void 	OnUpdateEditSpeak(CCmdUI* pCmdUI);
	afx_msg void 	OnUpdateEditSpellCheck(CCmdUI* pCmdUI);
	afx_msg void	OnUpdatePasteIndented(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateAllowStyled(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftInclude(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftAddCc(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftAddBcc(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftNotReject(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateTextStyle(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftAttachFile(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftSendMail(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftAppend(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftCopyOriginal(CCmdUI* pCmdUI);

	afx_msg void	OnUpdateDraftSign(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftEncrypt(CCmdUI* pCmdUI);

	afx_msg void	OnUpdateDraftMDN(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftDSNSuccess(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftDSNFailure(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftDSNDelay(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDraftDSNFull(CCmdUI* pCmdUI);

	afx_msg void	OnUpdateMenuExpandHeader(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateExpandHeader(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateWindowsShowParts(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnFileNewDraft();
	afx_msg void	OnFileSave();
	afx_msg void	OnFileImport();
	afx_msg void	OnFilePrint();
	afx_msg void	OnFilePrintPreview();

	afx_msg void	OnEditUndo();
	afx_msg void	OnPasteIndented();
	afx_msg void	OnEditSpeak();
			void	SpeakMessage();
	afx_msg void	OnEditSpellCheck();
			bool	DoSpellCheck(bool sending);

	afx_msg void	OnDraftInclude();
	afx_msg void	OnDraftInsertSignature();
			void	InsertSignature(const cdstring& signature);
			void	ReplaceSignature(const cdstring& old_sig, const cdstring& new_sig);
			void	NormaliseSignature(cdstring& old_sig);
	afx_msg void	OnDraftAttachFile();
	afx_msg void	OnDraftNewPlainPart();
	afx_msg void	OnDraftNewEnrichedPart();
	afx_msg void	OnDraftNewHTMLPart();
	afx_msg void	OnDraftMultipartMixed();
	afx_msg void	OnDraftMultipartParallel();
	afx_msg void	OnDraftMultipartDigest();
	afx_msg void	OnDraftMultipartAlternative();
	afx_msg void	OnDraftSendMail();
	afx_msg void	OnDraftAppendNow();							// Copy it to mailbox now
	afx_msg void	OnDraftAppendNowBtn();						// Copy it to mailbox now
	afx_msg void	OnDraftAppendNowOptionBtn();				// Copy it to mailbox now
	afx_msg void	OnDraftAppend(UINT nID);					// Copy it to mailbox now
	afx_msg void	OnDraftCopyOriginal();

	afx_msg void	OnDraftSign();
	afx_msg void	OnDraftEncrypt();

	afx_msg void	OnDraftMDN();
	afx_msg void	OnDraftDSNSuccess();
	afx_msg void	OnDraftDSNFailure();
	afx_msg void	OnDraftDSNDelay();
	afx_msg void	OnDraftDSNFull();

	afx_msg void	OnLetterHeaderTwister();

	afx_msg void	OnLetterToTwister();
	afx_msg void	OnLetterCCTwister();
	afx_msg void	OnLetterBCCTwister();
	afx_msg void	OnLetterCopyToPopup(UINT nID);					// Copy to popup change

			void	DoTwist(CWnd* mover,							// Handle twist
							CCmdEdit* aEdit,
							CTwister* aTwister);
	afx_msg void	OnDraftAddCc();									// Add Cc button clicked
	afx_msg void	OnDraftAddBcc();								// Add Bcc button clicked

			void	LayoutHeader();									// Layout header component based on state
			void	UpdateRecipients();						// Update recipients caption

	afx_msg void	OnLetterPartsTwister();
			bool	IsPartsTwist() const;
			void	DoPartsTwist(bool expand, bool change_focus);

	afx_msg void	OnSaveDefaultState();					// Save state in prefs
	afx_msg void	OnResetDefaultState();

	afx_msg void	OnUpdateStyle(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateAlignment(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateFont(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateFontSize(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateColor(CCmdUI *pCmdUI);
	
	afx_msg void 	OnStyle(UINT nID);
	afx_msg void 	OnAlignment(UINT nID);
	afx_msg void 	OnFont(UINT nID);
	afx_msg void 	OnFontSize(UINT nID);
	afx_msg void 	OnColor(UINT nID);

protected:
	void			ResetAutoSaveTimer();
	void			PauseAutoSaveTimer(bool pause);
	void			RefreshToolbar();
	CMailboxPopup*	GetAppendPopup();
	CFileTable*		GetPartsTable();
	void			ShowSentIcon();
	bool			DoAppendReplied() const;
	bool			IsSigned() const;
	bool			IsEncrypted() const;

private:
	CToolbarView			mToolbarView;
	CLetterView				mView;								// The view that controls the main toolbar

	CSplitterView			mSplitterView;
	CLetterHeaderView*		mHeader;

	CGrayBackground			mPartsPane;					// Container for parts
	CSimpleTitleTable		mPartsTitles;				// Parts titles
	CFileTable				mPartsTable;				// List of parts
	CWndAlignment*			mPartsTableAlign;			// Aligner for parts table

	CGrayBackground			mTextArea;
	CLetterTextEditView*	mText;

	CMessageList*			mMsgs;								// The associated messages for this window
	cdstring				mIdentity;							// Identity to use when sending mail
	bool					mCustomIdentity;					// Use custom identity from popup
	bool					mDoSign;							// Crypto sign this message
	bool					mDoEncrypt;							// Crypto encrypt this message
	CDSN					mDSN;								// DSN to use when sending mail
	CAttachment*			mCurrentPart;						// Text part on display
	CAttachment*			mBody;								// Parts list for message

	SHeaderState			mHeaderState;						// State of items in header

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
	char*					mBounceHeader;						// Bouncing message's header
	static cdstring			sLastAppendTo;						// Cache last selected append to
	cdstring				mSavedAppendMbox;					// Name of mailbox saved while server logged off
	cdstring				mSignature;							// Last signature inserted
	long					mSignatureLength;					// Length of last signature inserted
	bool					mCopyOriginalAllowed;				// Copy original allowed
	bool					mCopyOriginal;						// Copy original state
	LUndoer					mUndoer;							// Undoable actions
	cdstring				mOriginalTitle;						// Original document title

	UINT					mTimerID;
	time_t					mAutoSaveTime;						// Time at which to trigger auto save

			void	IncludeText(const char* theText, bool forward, bool adding = false);	// Include some text
			void	UpdatePartsCaption();						// Update parts caption
			CMessage*	CreateMessage(bool alert,				// Create message from contents
										bool bcc_only = false);
			bool	ValidAddressList(const CAddressList* addrs,		// Check that all addresses are valid format
										const char* str_id1, const char* strid2) const;

			void	CopyNow(bool option_key);					// Copy it to mailbox now
			void	CopyNow(CMbox* mbox, bool option_key);		// Copy it to mailbox now

			void	DraftSaved();								// Draft was saved in some fashion
			
			bool	MissingAttachments();						// Check for missing attachments
			bool	UnencryptedSend();							// Check for unencrypted send of originally encrypted content

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChangeSubject();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};

#endif
