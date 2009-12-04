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


#ifndef __CTASKCLASSES__MULBERRY__
#define __CTASKCLASSES__MULBERRY__

#include "CTaskQueue.h"

#include "CErrorDialog.h"
#include "CMessageFwd.h"
#include "CMessageList.h"
#include "CMessageRef.h"
#include "CMessageView.h"

class CAdbkAddress;
class CAddress;
class CAddressBook;
class CAddressBookView;
class CAddressBookWindow;
class CAddressView;
class CGroup;
class CINETProtocol;
class CMailboxInfoView;
class CMailboxView;
class CMailboxWindow;
class CMbox;
class CMboxProtocol;
class CMboxRef;
class CMessage;
class CMessageWindow;
class CServerView;
class CServerWindow;
class CLetterWindow;
class CSearchEngine;

// Password at idle time
class CUserPswdTask : public CTaskSync
{
public:
	CUserPswdTask(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
							const cdstring& title, const cdstring& server_ip,
							const cdstring& match, bool secure, int icon) :
		mUid(uid), mPswd(pswd), mSaveUser(save_user), mSavePswd(save_pswd),
		mTitle(title), mServerIP(server_ip), mMatch(match), mSecure(secure), mIcon(icon) {}

protected:
	cdstring& mUid;
	cdstring& mPswd;
	bool mSaveUser;
	bool mSavePswd;
	const cdstring& mTitle;
	const cdstring& mServerIP;
	const cdstring& mMatch;
	bool mSecure;
	int mIcon;

	virtual void Work();
};

// Accept cert at idle time
class CAcceptCertTask : public CTaskSync
{
public:
	CAcceptCertTask(const char* cert, const cdstrvect& errors) :
		mCert(cert), mErrors(errors) {}

protected:
	const char* mCert;
	const cdstrvect& mErrors;

	virtual void Work();
};

// Force close a server at idle time
class CServerForceoffTask : public CTaskAsync
{
public:
	CServerForceoffTask(CINETProtocol* proto) : mProto(proto) {}

protected:
	CINETProtocol* mProto;

	virtual void Work();
};

// Close a server window always at idle time
class CCloseServerWindowTask : public CTaskAlwaysQueued
{
public:
	CCloseServerWindowTask(CServerWindow* win) : mWindow(win) {}

protected:
	CServerWindow* mWindow;

	virtual void Work();
};

// Refresh a mailbox in a server view at idle time
class CServerViewChangedMboxTask : public CTaskAsync
{
public:
	CServerViewChangedMboxTask(CServerView* view, const CMbox* mbox);

protected:
	CServerView* mView;
	CMboxRef* mMboxRef;

	// Protect to ensure it cannot be created on the stack
	virtual ~CServerViewChangedMboxTask();

	virtual void Work();
};

// Change the New Message cabinet items at idle time
class CAddMailboxToNewMessagesTask : public CTaskAsync 
{
public:
	CAddMailboxToNewMessagesTask(const CMbox* mbox, bool is_new);

protected:
	CMboxRef* mMboxRef;
	bool mIsNew;

	// Protect to ensure it cannot be created on the stack
	virtual ~CAddMailboxToNewMessagesTask();

	virtual void Work();
};

// Generate mailbox preview at idle time
class CMailboxPreviewTask : public CTaskAlwaysQueued
{
public:
	CMailboxPreviewTask(CMailboxInfoView* view, CMbox* mbox, bool use_dynamic, bool is_search);

protected:
	CMailboxInfoView*	mView;
	CMboxRef*			mMboxRef;
	bool				mUseDynamic;
	bool				mIsSearch;

	// Protect to ensure it cannot be created on the stack
	virtual ~CMailboxPreviewTask();

	virtual void Work();
};

// Close a mailbox window always at idle time
class CCloseMailboxWindowTask : public CTaskAlwaysQueued
{
public:
	CCloseMailboxWindowTask(CMailboxWindow* window, bool force = false) : mWindow(window), mForce(force) {}
protected:
	CMailboxWindow* mWindow;
	bool			mForce;

	virtual void Work();
};

// Close a mailbox view always at idle time
class CCloseMailboxViewTask : public CTaskAlwaysQueued
{
public:
	CCloseMailboxViewTask(CMailboxView* view, CMbox* mbox, bool force = false) : mView(view), mMbox(mbox), mProto(NULL), mForce(force) {}
	CCloseMailboxViewTask(CMailboxView* view, CMboxProtocol* proto, bool force = false) : mView(view), mMbox(NULL), mProto(proto), mForce(force) {}
protected:
	CMailboxView*	mView;
	CMbox*			mMbox;
	CMboxProtocol*	mProto;
	bool			mForce;

	virtual void Work();
};

// Reset entire mailbox table
class CMailboxResetTableTask : public CTaskAsync
{
public:
	CMailboxResetTableTask(CMailboxView* view) : mView(view) {}

protected:
	CMailboxView* mView;

	virtual void Work();
};

// Refresh mailbox view
class CMailboxRefreshTask : public CTaskAsync
{
public:
	CMailboxRefreshTask(CMailboxView* view) : mView(view) {}

protected:
	CMailboxView* mView;

	virtual void Work();
};

// Refresh message in mailbox view
class CMailboxViewMessageChangedTask : public CTaskAsync
{
public:
	CMailboxViewMessageChangedTask(CMailboxView* view, const CMessage* msg)
		: mView(view), mMsg(msg) {}

protected:
	CMailboxView* mView;
	const CMessageRef mMsg;

	virtual void Work();
};

// Delete a mailbox always at idle time
class CDeleteMailboxTask : public CTaskAlwaysQueued
{
public:
	CDeleteMailboxTask(const CMbox* mbox);

protected:
	CMboxRef*	mMboxRef;

	// Protect to ensure it cannot be created on the stack
	virtual ~CDeleteMailboxTask();

	virtual void Work();
};

// Remove a singleton mailbox always at idle time
class CRemoveSingletonMailboxTask : public CTaskAlwaysQueued
{
public:
	CRemoveSingletonMailboxTask(CMbox* mbox)
	{
		mMbox = mbox;
	}

protected:
	CMbox*	mMbox;

	// Protect to ensure it cannot be created on the stack
	virtual ~CRemoveSingletonMailboxTask() {}

	virtual void Work();
};

// Generate message preview at idle time
class CMessagePreviewTask : public CTaskAlwaysQueued
{
public:
	CMessagePreviewTask(CMessageView* view, CMessage* msg) : mView(view), mMsg(msg) {}

protected:
	CMessageView*		mView;
	const CMessageRef	mMsg;

	virtual void Work();
};

// Generate message preview/restore at idle time
class CMessagePreviewRestoreTask : public CTaskAlwaysQueued
{
public:
	CMessagePreviewRestoreTask(CMessageView* view, const CMessageView::SMessageViewState& state, CMessage* msg) : mView(view), mState(state), mMsg(msg) {}

protected:
	CMessageView*						mView;
	CMessageView::SMessageViewState		mState;
	const CMessageRef					mMsg;

	virtual void Work();
};

// Close message window always at idle time
class CCloseMessageWindowTask : public CTaskAlwaysQueued
{
public:
	CCloseMessageWindowTask(CMessageWindow* window) : mWindow(window) {}

protected:
	CMessageWindow* mWindow;

	virtual void Work();
};

// Close message view always at idle time
class CCloseMessageViewTask : public CTaskAlwaysQueued
{
public:
	CCloseMessageViewTask(CMessageView* view) : mView(view) {}

protected:
	CMessageView* mView;

	virtual void Work();
};

// Refresh message view
class CMessageViewMessageChangedTask : public CTaskAsync
{
public:
	CMessageViewMessageChangedTask(CMessageView* view) : mView(view) {}

protected:
	CMessageView* mView;

	virtual void Work();
};

// Refresh message window
class CMessageWindowMessageChangedTask : public CTaskAsync
{
public:
	CMessageWindowMessageChangedTask(CMessageWindow* wnd) : mWindow(wnd) {}

protected:
	CMessageWindow* mWindow;

	virtual void Work();
};

// Allow letter windows to remove cached message objects
class CLetterWindowMessageRemovedTask : public CTaskAsync
{
public:
	CLetterWindowMessageRemovedTask(CLetterWindow* win, const CMessage* msg)
	 : mWindow(win), mMsg(msg) {}

protected:
	CLetterWindow*		mWindow;
	const CMessageRef	mMsg;

	virtual void Work();
};

// Start a search and update search window
class CSearchWindowStartSearchTask : public CTaskAsync
{
public:
	CSearchWindowStartSearchTask() {}

protected:
	virtual void Work();
};

// Update search window for next search
class CSearchWindowNextSearchTask : public CTaskAsync
{
public:
	CSearchWindowNextSearchTask(unsigned long row)
		: mRow(row) {}

protected:
	unsigned long mRow;

	virtual void Work();
};

// End a search and update search window
class CSearchWindowEndSearchTask : public CTaskAsync
{
public:
	CSearchWindowEndSearchTask() {}

protected:
	virtual void Work();
};

// Update hits and counters in search window
class CSearchWindowUpdateTask : public CTaskAsync
{
public:
	CSearchWindowUpdateTask(bool failed, unsigned long index, bool hit,
						unsigned long done, unsigned long found, unsigned long messages)
		: mFailed(failed), mIndex(index), mHit(hit),
		  mDone(done), mFound(found), mMessages(messages) {}

protected:
	bool mFailed;
	unsigned long mIndex;
	bool mHit;
	unsigned long mDone;
	unsigned long mFound;
	unsigned long mMessages;

	virtual void Work();
};

// Open a mailbox view with search results
class CSearchOpenTargetTask : public CTaskAsync
{
public:
	CSearchOpenTargetTask(CSearchEngine* se, unsigned long target, 
										CMailboxView* recycle = NULL)
		: mSearchEngine(se), mTarget(target), mRecycle(recycle) {}

protected:
	CSearchEngine* mSearchEngine;
	unsigned long mTarget;
	CMailboxView* mRecycle;

	virtual void Work();
};

// Generate address book preview at idle time
class CAddressBookPreviewTask : public CTaskAlwaysQueued
{
public:
	CAddressBookPreviewTask(CAddressBookView* view, CAddressBook* adbk);

protected:
	CAddressBookView*		mView;
	cdstring				mAdbkRef;

	virtual void Work();
};

// Update address book preview
class CAddressBookViewChangedTask : public CTaskAsync
{
public:
	CAddressBookViewChangedTask(CAddressBookView* view, bool reset = false)
		: mView(view), mReset(reset) {}

protected:
	CAddressBookView*	mView;
	bool				mReset;

	virtual void Work();
};

// Close address book view
class CAddressBookViewClosedTask : public CTaskAlwaysQueued
{
public:
	CAddressBookViewClosedTask(CAddressBookView* view)
		: mView(view) {}

protected:
	CAddressBookView*	mView;

	virtual void Work();
};

// Close an address book window always at idle time
class CCloseAddressBookWindowTask : public CTaskAlwaysQueued
{
public:
	CCloseAddressBookWindowTask(CAddressBookWindow* win) : mWindow(win) {}

protected:
	CAddressBookWindow* mWindow;

	virtual void Work();
};

// Update address view
class CAddressViewChangedTask : public CTaskAsync
{
public:
	CAddressViewChangedTask(CAddressView* view, CAddressBook* adbk, CAdbkAddress* addr)
		: mView(view), mAdbk(adbk), mAddress(addr), mGroup(NULL), mDoAddress(true) {}
	CAddressViewChangedTask(CAddressView* view, CAddressBook* adbk, CGroup* grp)
		: mView(view), mAdbk(adbk), mAddress(NULL), mGroup(grp), mDoAddress(false) {}

protected:
	CAddressView*		mView;
	CAddressBook*		mAdbk;
	CAdbkAddress*		mAddress;
	CGroup*				mGroup;
	bool				mDoAddress;

	virtual void Work();
};

// Generate address preview at idle time
class CAddressPreviewTask : public CTaskAlwaysQueued
{
public:
	CAddressPreviewTask(CAddressView* view, CAddressBook* adbk, CAdbkAddress* addr)
		: mView(view), mAdbk(adbk), mAddress(addr) {}

protected:
	CAddressView*		mView;
	CAddressBook*		mAdbk;
	CAdbkAddress*		mAddress;

	virtual void Work();
};

// Generate group preview at idle time
class CGroupPreviewTask : public CTaskAlwaysQueued
{
public:
	CGroupPreviewTask(CAddressView* view, CAddressBook* adbk, CGroup* grp)
		: mView(view), mAdbk(adbk), mGroup(grp) {}

protected:
	CAddressView*		mView;
	CAddressBook*		mAdbk;
	CGroup*				mGroup;

	virtual void Work();
};

// Stop alert
class CStopAlertTxtTask : public CTaskAsync
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CStopAlertTxtTask(short strId, short index, const char* txt)
		: mStrId(strId), mIndex(index), mTxt(txt) {}
	CStopAlertTxtTask(short strId, short index)
		: mStrId(strId), mIndex(index) {}
	CStopAlertTxtTask(const char* txt)
		: mStrId(0), mIndex(0), mTxt(txt) {}
#else
	CStopAlertTxtTask(short strId, const char* txt)
		: mStrId(strId), mTxt(txt) {}
	CStopAlertTxtTask(short strId)
		: mStrId(strId) {}
	CStopAlertTxtTask(const char* txt)
		: mStrId(0), mTxt(txt) {}
#endif

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	short		mStrId;
	short		mIndex;
	cdstring	mTxt;
#else
	short		mStrId;
	cdstring	mTxt;
#endif

	virtual void Work();
};

class CStopAlertRsrcTxtTask : public CTaskAsync
{
public:
	CStopAlertRsrcTxtTask(const char* rsrcid, const char* txt)
		: mRsrcID(rsrcid), mTxt(txt) {}
	CStopAlertRsrcTxtTask(const char* rsrcid)
		: mRsrcID(rsrcid) {}

protected:
	const char*	mRsrcID;
	cdstring	mTxt;

	virtual void Work();
};

// OSErr alert
class COSErrAlertTxtTask : public CTaskAsync
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	COSErrAlertTxtTask(short strId, short index, OSErr err, const char* txt)
		: mStrId(strId), mIndex(index), mError(err), mTxt(txt) {}
	COSErrAlertTxtTask(short strId, short index, OSErr err)
		: mStrId(strId), mIndex(index), mError(err) {}
#else
	COSErrAlertTxtTask(short strId, OSErr err, const char* txt)
		: mStrId(strId), mError(err), mTxt(txt) {}
	COSErrAlertTxtTask(short strId, OSErr err)
		: mStrId(strId), mError(err) {}
#endif

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	short		mStrId;
	short		mIndex;
	OSErr		mError;
	cdstring	mTxt;
#else
	short		mStrId;
	OSErr		mError;
	cdstring	mTxt;
#endif

	virtual void Work();
};

class COSErrAlertRsrcTxtTask : public CTaskAsync
{
public:
	COSErrAlertRsrcTxtTask(const char* rsrcid, OSErr err, const char* txt)
		: mRsrcId(rsrcid), mError(err), mTxt(txt) {}
	COSErrAlertRsrcTxtTask(const char* rsrcid, OSErr err)
		: mRsrcId(rsrcid), mError(err) {}

protected:
	const char*	mRsrcId;
	OSErr		mError;
	cdstring	mTxt;

	virtual void Work();
};

// Caution alert - must be sync as a result is required before continuing
class CCautionAlertTxtTask : public CTaskSync
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CCautionAlertTxtTask(bool yesno, short strId, short index)
		: mYesNo(yesno), mStrId(strId), mIndex(index), mRsrcID(NULL) {}
#else
	CCautionAlertTxtTask(bool yesno, short strId)
		: mYesNo(yesno), mStrId(strId), mRsrcID(NULL) {}
#endif
	CCautionAlertTxtTask(bool yesno, const char* rsrcid)
		: mYesNo(yesno), mStrId(0), mRsrcID(rsrcid) {}

protected:
	bool		mYesNo;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	short		mStrId;
	short		mIndex;
#else
	short		mStrId;
#endif
	const char*	mRsrcID;

	virtual void Work();
};

// Error dialog alert - must be sync as a result is required before continuing
class CErrorDialogTask : public CTaskSync
{
public:
	CErrorDialogTask(CErrorDialog::EErrDialogType type, const char* btn1, const char* btn2,
								const char* btn3, const char* btn4, const char* str, unsigned long cancel_id = 2,
								const char* extra1 = NULL)
		: mType(type), mBtn1(btn1), mBtn2(btn2), mBtn3(btn3), mBtn4(btn4),
			mStrId(str), mCancelID(cancel_id), mExtra1(extra1) {}

protected:
	CErrorDialog::EErrDialogType	mType;
	const char*						mBtn1;
	const char*						mBtn2;
	const char*						mBtn3;
	const char*						mBtn4;
	const char*						mStrId;
	unsigned long					mCancelID;
	cdstring						mExtra1;

	virtual void Work();
};

// Update IMAP status
class CIMAPStatusTask : public CTaskAsync
{
public:
	CIMAPStatusTask(const cdstring& s) : mStr(s) {}

protected:
	cdstring mStr;

	virtual void Work();
};

// Update SMTP status
class CSMTPStatusTask : public CTaskAsync
{
public:
	CSMTPStatusTask(const cdstring& s) : mStr(s) {}

protected:
	cdstring mStr;

	virtual void Work();
};

// Update IMAP progress
class CIMAPProgressTask : public CTaskAsync
{
public:
	CIMAPProgressTask(const cdstring& s) : mStr(s) {}

protected:
	cdstring mStr;

	virtual void Work();
};

// Update SMTP progress
class CSMTPProgressTask : public CTaskAsync
{
public:
	CSMTPProgressTask(const cdstring& s) : mStr(s) {}

protected:
	cdstring mStr;

	virtual void Work();
};

// Action to create a reply draft
class CCreateReplyTask : public CTaskAsync
{
public:
	CCreateReplyTask(const CMessageList& msgs, bool quote, NMessage::EReplyType reply,
									const cdstring& text, const cdstring& identity, bool tied);

protected:
	CMessageList			mMsgs;
	bool 					mQuote;
	NMessage::EReplyType	mReply;
	cdstring 				mText;
	cdstring				mIdentity;
	bool 					mTied;

	virtual void Work();
};

// Action to create a forward draft
class CCreateForwardTask : public CTaskAsync
{
public:
	CCreateForwardTask(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
										const cdstring& text, const cdstring& identity, bool tied);

protected:
	CMessageList			mMsgs;
	bool 					mQuote;
	bool 					mAttach;
	NMessage::SAddressing	mAddresses;
	cdstring 				mText;
	cdstring				mIdentity;
	bool 					mTied;

	virtual void Work();
};

// Action to create a bounce draft
class CCreateBounceTask : public CTaskAsync
{
public:
	CCreateBounceTask(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied);

protected:
	CMessageList			mMsgs;
	NMessage::SAddressing	mAddresses;
	cdstring				mIdentity;
	bool 					mTied;

	virtual void Work();
};

// Action to create a reject draft
class CCreateRejectTask : public CTaskAsync
{
public:
	CCreateRejectTask(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied);

protected:
	CMessageList			mMsgs;
	bool 					mReturnMsg;
	cdstring				mIdentity;
	bool 					mTied;

	virtual void Work();
};

#endif
