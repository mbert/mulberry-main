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

#include "CTaskClasses.h"

#include "CAcceptCertDialog.h"
#include "CActionManager.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAddressView.h"
#include "CErrorHandler.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoView.h"
#include "CMailboxWindow.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CUserPswdDialog.h"
#include "CServerView.h"
#include "CServerWindow.h"
#include "CLetterWindow.h"
#include "CSearchEngine.h"
#include "CSearchWindow.h"
#include "CStatusWindow.h"

#pragma mark ____________________________CUserPswdTask

void CUserPswdTask::Work()
{
	mResult = CUserPswdDialog::PoseDialog(mUid, mPswd, mSaveUser, mSavePswd,
								mTitle, mServerIP, mMatch, mSecure, mIcon);
}

#pragma mark ____________________________CAcceptCertTask

void CAcceptCertTask::Work()
{
	mResult = CAcceptCertDialog::PoseDialog(mCert, mErrors);
}

#pragma mark ____________________________CServerForceoffTask

void CServerForceoffTask::Work()
{
	// Force connection closed
	mProto->Forceoff();
}

#pragma mark ____________________________CCloseServerWindowTask

void CCloseServerWindowTask::Work()
{
	// Check that window still exists and delete it
	if (CServerWindow::WindowExists(mWindow))
		FRAMEWORK_DELETE_WINDOW(mWindow)
}

#pragma mark ____________________________CServerViewChangedMboxTask

CServerViewChangedMboxTask::CServerViewChangedMboxTask(CServerView* view, const CMbox* mbox)
{
	mView = view;
	mMboxRef = new CMboxRef(mbox);
}

CServerViewChangedMboxTask::~CServerViewChangedMboxTask()
{
	delete mMboxRef;
	mMboxRef = NULL;
}

void CServerViewChangedMboxTask::Work()
{
	// Resolve the ref to a real mailbox
	CMbox* mbox = mMboxRef->ResolveMbox();

	// Check that view/mailbox still exist and do update
	if (CServerView::ViewExists(mView) && mbox)
		mView->ChangedMbox(mbox);
}

#pragma mark ____________________________CAddMailboxToNewMessagesTask

CAddMailboxToNewMessagesTask::CAddMailboxToNewMessagesTask(const CMbox* mbox, bool is_new)
{
	mMboxRef = new CMboxRef(mbox);
	mIsNew = is_new;
}

CAddMailboxToNewMessagesTask::~CAddMailboxToNewMessagesTask()
{
	delete mMboxRef;
	mMboxRef = NULL;
}

void CAddMailboxToNewMessagesTask::Work()
{
	// Resolve the ref to a real mailbox and pass that to the favourites
	CMbox* mbox = mMboxRef->ResolveMbox();

	// Only do if mailbox still exists
	if (mbox)
	{
		// Do cabinet processing
		CMailAccountManager::sMailAccountManager->ChangeFavouriteItem(mbox, CMailAccountManager::eFavouriteNew, mIsNew);
		
		// Resolve to open mailbox
		CMbox* open_mbox = mbox->GetProtocol()->FindOpenMbox(mbox->GetName());
		if (open_mbox)
		{
			// Inform each mailbox view of change in state
			cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
			for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			{
				if ((*iter)->IsOpen() && (*iter)->ContainsMailbox(open_mbox))
					(*iter)->ChangeIconSubstitute(open_mbox, mIsNew);
			}
		}
	}
}

#pragma mark ____________________________CMailboxPreviewTask

CMailboxPreviewTask::CMailboxPreviewTask(CMailboxInfoView* view, CMbox* mbox, bool use_dynamic, bool is_search)
{
	mView = view;
	mMboxRef = new CMboxRef(mbox);
	mUseDynamic = use_dynamic;
	mIsSearch = is_search;
}

CMailboxPreviewTask::~CMailboxPreviewTask()
{
	delete mMboxRef;
	mMboxRef = NULL;
}

void CMailboxPreviewTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CMailboxView::ViewExists(mView))
	{
		// Resolve the ref to a real mailbox and pass that to the favourites
		CMbox* mbox = mMboxRef->ResolveMbox();
		
		// Tell view about it even if NULL as view will empty itself
		mView->ViewMbox(mbox, mUseDynamic, mIsSearch);
		
		// Focus on the view if adbk exists
		if (mbox)
			mView->Focus();
	}
}

#pragma mark ____________________________CCloseMailboxWindowTask

void CCloseMailboxWindowTask::Work()
{
	// Check that window still exists and delete it
	if (CMailboxWindow::WindowExists(mWindow))
	{
		// Force off if required
		if (mForce)
			mWindow->GetMailboxView()->ForceClose(mWindow->GetMailboxView()->GetMbox());
		FRAMEWORK_DELETE_WINDOW(mWindow)
	}
}

#pragma mark ____________________________CCloseMailboxViewTask

void CCloseMailboxViewTask::Work()
{
	// Check that window still exists and delete it
	if (CMailboxView::ViewExists(mView))
	{
		// Force off if required
		if (mForce)
		{
			if (mMbox)
				mView->ForceClose(mMbox);
			if (mProto)
				mView->ForceClose(mProto);
		}
		if (mMbox)
			static_cast<CMailboxInfoView*>(mView)->DoClose(mMbox);
		if (mProto)
			static_cast<CMailboxInfoView*>(mView)->DoClose(mProto);
	}
}

#pragma mark ____________________________CMailboxResetTableTask

void CMailboxResetTableTask::Work()
{
	// Check that view still exists and update it
	if (CMailboxView::ViewExists(mView))
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CTableDrag::StDeferSelectionChanged _defer(mView->GetTable());
#else
		CTable::StDeferSelectionChanged _defer(mView->GetTable());
#endif

		mView->ResetTableNew();
		
		// Unpreserve selection
		mView->GetTable()->ResetSelection(false);
	}
}

#pragma mark ____________________________CMailboxRefreshTask

void CMailboxRefreshTask::Work()
{
	// Check that view still exists and update it
	if (CMailboxView::ViewExists(mView))
	{
		FRAMEWORK_REFRESH_WINDOW(mView)
	}
}

#pragma mark ____________________________CMailboxViewMessageChangedTask

void CMailboxViewMessageChangedTask::Work()
{
	// Check that view still exists and update it
	if (CMailboxView::ViewExists(mView))
	{
		// Resolve to a real message
		CMessage* msg = mMsg.ResolveMessage();
		
		// Tell view about it if it still exists
		if (msg)
			mView->ChangedMessage(msg);
	}
}

#pragma mark ____________________________CDeleteMailboxTask

CDeleteMailboxTask::CDeleteMailboxTask(const CMbox* mbox)
{
	mMboxRef = new CMboxRef(mbox);
}

CDeleteMailboxTask::~CDeleteMailboxTask()
{
	delete mMboxRef;
	mMboxRef = NULL;
}

void CDeleteMailboxTask::Work()
{
	// Resolve the ref to a real mailbox
	CMbox* mbox = mMboxRef->ResolveMbox();

	// Mailbox must exist
	if (mbox)
	{
		// See if it is still open
		CMbox* open_mbox = mbox->GetProtocol()->FindOpenMbox(mbox->GetName());
		
		// Cannot do delete if its still open at this point
		if (open_mbox)
			return;

		// Delete the mailbox (will delete mbox object)
		mbox->Delete();
	}
}

#pragma mark ____________________________CRemoveSingletonMailboxTask

void CRemoveSingletonMailboxTask::Work()
{
	// Mailbox must exist
	if (mMbox)
	{
		mMbox->GetProtocol()->RemoveSingleton(mMbox);
	}
}

#pragma mark ____________________________CCloseMessageWindowTask

void CCloseMessageWindowTask::Work()
{
	// Check that window still exists and delete it
	if (CMessageWindow::WindowExists(mWindow))
		FRAMEWORK_DELETE_WINDOW(mWindow)
}

#pragma mark ____________________________CCloseMessageViewTask

void CCloseMessageViewTask::Work()
{
	// Check that window still exists and delete it
	if (CMessageView::ViewExists(mView))
		mView->DoClose();
}

#pragma mark ____________________________CMessagePreviewTask

void CMessagePreviewTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CMessageView::ViewExists(mView))
	{
		// Resolve to a real message
		CMessage* msg = mMsg.ResolveMessage();
		
		// Tell view about it even if NULL as view will empty itself
		mView->SetMessage(msg);
	}
}

#pragma mark ____________________________CMessagePreviewRestoreTask

void CMessagePreviewRestoreTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CMessageView::ViewExists(mView))
	{
		// Resolve to a real message
		CMessage* msg = mMsg.ResolveMessage();
		
		// Tell view about it even if NULL as view will empty itself
		mView->RestoreMessage(msg, mState);
	}
}

#pragma mark ____________________________CMessageViewMessageChangedTask

void CMessageViewMessageChangedTask::Work()
{
	// Check that view still exists and update it
	if (CMessageView::ViewExists(mView))
		mView->MessageChanged();
}

#pragma mark ____________________________CMessageWindowMessageChangedTask

void CMessageWindowMessageChangedTask::Work()
{
	// Check that view still exists and update it
	if (CMessageWindow::WindowExists(mWindow))
		mWindow->MessageChanged();
}

#pragma mark ____________________________CLetterWindowMessageRemovedTask

void CLetterWindowMessageRemovedTask::Work()
{
	// Check that window still exists and update it
	if (CLetterWindow::WindowExists(mWindow))
	{
		// Resolve to a real message
		CMessage* msg = mMsg.ResolveMessage();
		
		// Tell view about it if it still exists
		if (msg)
			mWindow->MessageRemoved(msg);
	}
}

#pragma mark ____________________________CSearchWindowStartSearchTask

void CSearchWindowStartSearchTask::Work()
{
	if (CSearchWindow::sSearchWindow)
		CSearchWindow::sSearchWindow->StartSearch();
}

#pragma mark ____________________________CSearchWindowNextSearchTask

void CSearchWindowNextSearchTask::Work()
{
	if (CSearchWindow::sSearchWindow)
		CSearchWindow::sSearchWindow->NextSearch(mRow);
}

#pragma mark ____________________________CSearchWindowEndSearchTask

void CSearchWindowEndSearchTask::Work()
{
	if (CSearchWindow::sSearchWindow)
		CSearchWindow::sSearchWindow->EndSearch();
}

#pragma mark ____________________________CSearchWindowUpdateTask

void CSearchWindowUpdateTask::Work()
{
	if (CSearchWindow::sSearchWindow)
	{
		if (mFailed)
			// Clear any icon
			CSearchWindow::sSearchWindow->SetHitState(mIndex, false, true);
		else
		{
			CSearchWindow::sSearchWindow->SetHitState(mIndex, mHit);
			CSearchWindow::sSearchWindow->SetProgress(mDone);
			if (mHit)
			{
				CSearchWindow::sSearchWindow->SetFound(mFound);
				CSearchWindow::sSearchWindow->SetMessages(mMessages);
			}
		}
	}
}

#pragma mark ____________________________CSearchOpenTargetTask

void CSearchOpenTargetTask::Work()
{
	mSearchEngine->OpenTarget(mTarget, mRecycle);
}

#pragma mark ____________________________CAddressBookPreviewTask

CAddressBookPreviewTask::CAddressBookPreviewTask(CAddressBookView* view, CAddressBook* adbk)
{
	mView = view;
	mAdbkRef = adbk->GetAccountName();
}

void CAddressBookPreviewTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CAddressBookView::ViewExists(mView))
	{
		// Resolve to a real message
		CAddressBook* adbk = CAddressBookManager::sAddressBookManager ?
								const_cast<CAddressBook*>(CAddressBookManager::sAddressBookManager->GetNode(mAdbkRef)) :
								NULL;
		
		// Tell view about it even if NULL as view will empty itself
		mView->SetAddressBook(adbk);
		
		// Focus on the view if adbk exists
		if (adbk)
			mView->Focus();
	}
}

#pragma mark ____________________________CAddressBookViewChangedTask

void CAddressBookViewChangedTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CAddressBookView::ViewExists(mView))
	{
		// Refresh or reset it
		if (mReset)
			mView->ResetTable();
		else
			FRAMEWORK_REFRESH_WINDOW(mView)
	}
}

#pragma mark ____________________________CAddressBookViewClosedTask

void CAddressBookViewClosedTask::Work()
{
	// Check that view still exists and give it an address/group to display
	if (CAddressBookView::ViewExists(mView))
	{
		// Close it
		mView->DoClose();
	}
}

#pragma mark ____________________________CCloseAddressBookWindowTask

void CCloseAddressBookWindowTask::Work()
{
	// Check that window still exists and delete it
	if (CAddressBookWindow::WindowExists(mWindow))
		FRAMEWORK_DELETE_WINDOW(mWindow)
}

#pragma mark ____________________________CAddressViewChangedTask

void CAddressViewChangedTask::Work()
{
	// Check that view still exists and give it an address/group to display
	if (CAddressView::ViewExists(mView))
	{
		// Reset the address or group in its view
		if (mDoAddress)
			mView->SetAddress(mAdbk, mAddress, true);
		else
			mView->SetGroup(mAdbk, mGroup, true);
	}
}

#pragma mark ____________________________CAddressPreviewTask

void CAddressPreviewTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CAddressView::ViewExists(mView))
	{
		// Tell view about it even if NULL as view will empty itself
		mView->SetAddress(mAdbk, mAddress);
	}
}

#pragma mark ____________________________CGroupPreviewTask

void CGroupPreviewTask::Work()
{
	// Check that view still exists and give it a message to display
	if (CAddressView::ViewExists(mView))
	{
		// Tell view about it even if NULL as view will empty itself
		mView->SetGroup(mAdbk, mGroup);
	}
}

#pragma mark ____________________________CStopAlertTxtTask

void CStopAlertTxtTask::Work()
{
	if (mStrId && !mTxt.empty())
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutStopAlertTxt(mStrId, mIndex, mTxt.c_str());
#else
		CErrorHandler::PutStopAlertTxt(mStrId, mTxt.c_str());
#endif
	else if (mStrId && mTxt.empty())
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutStopAlert(mStrId, mIndex);
#else
		CErrorHandler::PutStopAlert(mStrId);
#endif
	else
		CErrorHandler::PutStopAlert(mTxt.c_str());
}

#pragma mark ____________________________CStopAlertRsrcTxtTask

void CStopAlertRsrcTxtTask::Work()
{
	if (mTxt.empty())
		CErrorHandler::PutStopAlertRsrc(mRsrcID);
	else
		CErrorHandler::PutStopAlertRsrcTxt(mRsrcID, mTxt.c_str());
}

#pragma mark ____________________________COSErrAlertTxtTask

void COSErrAlertTxtTask::Work()
{
	if (!mTxt.empty())
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutOSErrAlertTxt(mStrId, mIndex, mError, mTxt.c_str());
#else
		CErrorHandler::PutOSErrAlertTxt(mStrId, mError, mTxt.c_str());
#endif
	else
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutOSErrAlert(mStrId, mIndex, mError);
#else
		CErrorHandler::PutOSErrAlert(mStrId, mError);
#endif
}

#pragma mark ____________________________COSErrAlertRsrcTxtTask

void COSErrAlertRsrcTxtTask::Work()
{
	if (mTxt.empty())
		CErrorHandler::PutOSErrAlertRsrc(mRsrcId, mError);
	else
		CErrorHandler::PutOSErrAlertRsrcTxt(mRsrcId, mError, mTxt.c_str());
}

#pragma mark ____________________________CCautionAlertTxtTask

void CCautionAlertTxtTask::Work()
{
	if (mStrId != 0)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mResult = CErrorHandler::PutCautionAlert(mYesNo, mStrId, mIndex);
#else
		mResult = CErrorHandler::PutCautionAlert(mYesNo, mStrId);
#endif
	else
		mResult = CErrorHandler::PutCautionAlertRsrc(mYesNo, mRsrcID);
}

#pragma mark ____________________________CErrorDialogTask

void CErrorDialogTask::Work()
{
	mResult = CErrorDialog::PoseDialog(mType, mBtn1, mBtn2, mBtn3, mBtn4, mStrId, mCancelID, mExtra1.length() ? mExtra1.c_str() : NULL);
}

#pragma mark ____________________________CIMAPStatusTask

void CIMAPStatusTask::Work()
{
	CStatusWindow::SetIMAPStatusStr(mStr);
}

#pragma mark ____________________________CSMTPStatusTask

void CSMTPStatusTask::Work()
{
	CStatusWindow::SetSMTPStatusStr(mStr);
}

#pragma mark ____________________________CIMAPProgressTask

void CIMAPProgressTask::Work()
{
	CStatusWindow::SetIMAPProgressStr(mStr);
}

#pragma mark ____________________________CSMTPProgressTask

void CSMTPProgressTask::Work()
{
	CStatusWindow::SetSMTPProgressStr(mStr);
}

#pragma mark ____________________________CCreateReplyTask

CCreateReplyTask::CCreateReplyTask(const CMessageList& msgs, bool quote, NMessage::EReplyType reply,
									const cdstring& text, const cdstring& identity, bool tied)
{
	mMsgs.SetOwnership(false);
	mMsgs = msgs;
	mQuote = quote;
	mReply = reply;
	mText = text;
	mIdentity = identity;
	mTied = tied;
}

void CCreateReplyTask::Work()
{
	CActionManager::CreateReplyMessages(mMsgs, mQuote, mReply, mText, mIdentity, mTied);
}

#pragma mark ____________________________CCreateForwardTask

CCreateForwardTask::CCreateForwardTask(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
										const cdstring& text, const cdstring& identity, bool tied)
{
	mMsgs.SetOwnership(false);
	mMsgs = msgs;
	mQuote = quote;
	mAttach = attach;
	mAddresses = addresses;
	mText = text;
	mIdentity = identity;
	mTied = tied;
}

void CCreateForwardTask::Work()
{
	CActionManager::CreateForwardMessages(mMsgs, mQuote, mAttach, mAddresses, mText, mIdentity, mTied);
}

#pragma mark ____________________________CCreateBounceTask

CCreateBounceTask::CCreateBounceTask(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied)
{
	mMsgs.SetOwnership(false);
	mMsgs = msgs;
	mAddresses = addresses;
	mIdentity = identity;
	mTied = tied;
}

void CCreateBounceTask::Work()
{
	CActionManager::CreateBounceMessages(mMsgs, mAddresses, mIdentity, mTied);
}

#pragma mark ____________________________CCreateRejectTask

CCreateRejectTask::CCreateRejectTask(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied)
{
	mMsgs.SetOwnership(false);
	mMsgs = msgs;
	mReturnMsg = return_msg;
	mIdentity = identity;
	mTied = tied;
}

void CCreateRejectTask::Work()
{
	CActionManager::CreateRejectMessages(mMsgs, mReturnMsg, mIdentity, mTied);
}
