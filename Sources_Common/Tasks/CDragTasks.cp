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


// Source for CDragTasks class

#include "CDragTasks.h"

#include "CCalendarStoreManager.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoTable.h"
#include "CMailboxView.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMessageWindow.h"
#include "CServerBrowse.h"

CDragTask* CDragTask::sCurrentDragTask = NULL;

bool CDragTask::Execute()
{
	bool result = false;
	CDragTask* task = GetCurrentDragTask();
	if (task)
	{
		try
		{
			// Execute the actual task
			result = task->ExecuteSelf();
		}
		catch(...)
		{
			// Catch all errors
			CLOG_LOGCATCH(...);
		}

		// Always delete task after execution
		delete task;
	}

	return result;
}

bool CServerDragOpenServerTask::ExecuteSelf()
{
	// Validate data
	if (!mOwner || !mProto)
		return false;
	
	// Execute operation
	mOwner->SetServer(mProto);
	
	return true;
}

void CDragServerTask::AddIndex(unsigned long index)
{
	mMoveIndices.push_back(index);
}

bool CDragServerTask::ExecuteSelf()
{
	// Validate data
	if ((mDropAtItemIndex < 0) || !mMoveIndices.size())
		return false;
	
	// Execute operation
	std::sort(mMoveIndices.begin(), mMoveIndices.end());
	for(ulvector::iterator iter = mMoveIndices.begin(); iter != mMoveIndices.end(); iter++)
	{
		// Only if different
		if (*iter != mDropAtItemIndex)
			CMailAccountManager::sMailAccountManager->MoveProtocol(*iter, mDropAtItemIndex);

		// Must adjust if dragging above
		if (mDropAtItemIndex < *iter)
			mDropAtItemIndex++;
	}
	
	return true;
}

void CDragWDTask::AddMboxList(CMboxList* wd)
{
	mMoveWDs.push_back(wd);
}

bool CDragWDTask::ExecuteSelf()
{
	// Validate data
	if ((mDropAtItemIndex < 0) || !mMoveWDs.size())
		return false;
	
	// Execute operation
	for(CHierarchies::iterator iter = mMoveWDs.begin(); iter != mMoveWDs.end(); iter++)
	{
		unsigned long old_index = (*iter)->GetHierarchyIndex();

		// Only if different
		if (old_index != mDropAtItemIndex)
			// Change wds
			(*iter)->GetProtocol()->MoveWD(old_index, mDropAtItemIndex);

		// Must adjust if dragging above
		if (mDropAtItemIndex < old_index)
			mDropAtItemIndex++;
	}
	
	return true;
}

void CDragMessagesTask::AddMessage(const CMessage* msg)
{
	mCopyMsgNums.push_back(msg->GetMessageNumber());
}

bool CDragMessagesTask::ExecuteSelf()
{
	// Validate data
	if (!mCopyToMbox || !mCopyFromMbox || !mCopyMsgNums.size())
		return false;

	try
	{
		// Set flag
		CMailboxInfoTable::sDropOnMailbox = true;

		ulmap temp;
		mCopyFromMbox->CopyMessage(mCopyMsgNums, false, mCopyToMbox, temp);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Make sure not signalled as mailbox to prevent delete after copy
		CMailboxInfoTable::sDropOnMailbox = false;

		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

void CDragMailboxTask::AddMbox(CMbox* mbox)
{
	mMboxList.push_back(mbox);
}

bool CDragMailboxIntoMailboxTask::ExecuteSelf()
{
	// Validate data
	if (!mMbox || !mMboxList.size())
		return false;

	for(CMboxList::iterator iter = mMboxList.begin(); iter != mMboxList.end(); iter++)
	{
		if (mMbox->IsDirectory())
		{
			// Move mailbox
			static_cast<CMbox*>(*iter)->MoveMbox(mMbox, false);
		}
		else
		{
			// Do mailbox -> mailbox copy
			CMbox* open_mbox = static_cast<CMbox*>(*iter)->GetProtocol()->FindOpenMbox(static_cast<CMbox*>(*iter)->GetName());
			if (!open_mbox)
				open_mbox = static_cast<CMbox*>(*iter);

			// Do copy (will open/close if not already)
			open_mbox->CopyMbox(mMbox);
		}
	}

	return true;
}

bool CDragMailboxAtMailboxTask::ExecuteSelf()
{
	// Validate data
	if (!mParent && !mDestination || !mMboxList.size())
		return false;

	for(CMboxList::iterator iter = mMboxList.begin(); iter != mMboxList.end(); iter++)
	{
		// Move mailbox to WD level
		static_cast<CMbox*>(*iter)->MoveMbox(mParent ? mParent : mDestination, !mParent);
	}

	return true;
}

bool CDragMailboxToCabinetTask::ExecuteSelf()
{
	// Validate data
	if (!mCabinet || !mMboxList.size())
		return false;

	for(CMboxList::iterator iter = mMboxList.begin(); iter != mMboxList.end(); iter++)
	{
		// Add to list
		CMailAccountManager::sMailAccountManager->AddFavouriteItem(mCabinet, static_cast<CMbox*>(*iter));
	}
	
	return true;
}

bool CDragMailboxSubscribeTask::ExecuteSelf()
{
	// Validate data
	if (!mMboxList.size())
		return false;

	for(CMboxList::iterator iter = mMboxList.begin(); iter != mMboxList.end(); iter++)
	{
		if (!static_cast<CMbox*>(*iter)->IsSubscribed())
			static_cast<CMbox*>(*iter)->Subscribe();
	}
	
	return true;
}

void CDragMailboxRefTask::AddMboxRef(CMboxRef* mboxref)
{
	mMboxRefList.push_back(mboxref);
}

bool CDragMailboxRefToCabinetTask::ExecuteSelf()
{
	// Validate data
	if (!mCabinet || !mMboxRefList.size())
		return false;

	for(CMboxRefList::iterator iter = mMboxRefList.begin(); iter != mMboxRefList.end(); iter++)
	{
		// Make sure its not copied back to itself
		if (mCabinet->FetchIndexOf(static_cast<CMboxRef*>(*iter)) == 0)
			// Add to list
			CMailAccountManager::sMailAccountManager->AddFavouriteItem(mCabinet, static_cast<CMboxRef*>(*iter));
	}
	
	return true;
}

bool CDragMailboxToView::ExecuteSelf()
{
	// Validate data
	if (!mView || !CMailboxView::ViewExists(mView) || !mMbox)
		return false;

	// Give mailbox to view for recyle operation
	mView->Recycle(mMbox);
	
	return true;
}

bool CDragMessageToWindow::ExecuteSelf()
{
	// Validate data
	if (!mWnd || !CMessageWindow::WindowExists(mWnd) || !mMsg)
		return false;

	// Give message to window
	mWnd->SetMessage(mMsg);
	
	return true;
}

void CDragCalendarServerTask::AddIndex(unsigned long index)
{
	mMoveIndices.push_back(index);
}

bool CDragCalendarServerTask::ExecuteSelf()
{
	// Validate data
	if ((mDropAtItemIndex < 0) || !mMoveIndices.size())
		return false;
	
	// Execute operation
	std::sort(mMoveIndices.begin(), mMoveIndices.end());
	for(ulvector::iterator iter = mMoveIndices.begin(); iter != mMoveIndices.end(); iter++)
	{
		// Only if different
		if (*iter != mDropAtItemIndex)
			calstore::CCalendarStoreManager::sCalendarStoreManager->MoveProtocol(*iter, mDropAtItemIndex);

		// Must adjust if dragging above
		if (mDropAtItemIndex < *iter)
			mDropAtItemIndex++;
	}
	
	return true;
}

void CDragCalendarTask::AddCalendar(calstore::CCalendarStoreNode* node)
{
	mCalendarList.push_back(node);
}

bool CDragCalendarIntoCalendarTask::ExecuteSelf()
{
	// Validate data
	if (!mCalendar || !mCalendarList.size())
		return false;
	
	// Ask user before doing task
	if (CErrorHandler::PutCautionAlertRsrcStr(true, mCalendar->IsDirectory() ? "CCalendarStoreTable::ReallyMoveCalendar" : "CCalendarStoreTable::ReallyCopyCalendar", mCalendar->GetName()) == CErrorHandler::Cancel)
		return false;

	for(calstore::CCalendarStoreNodeList::iterator iter = mCalendarList.begin(); iter != mCalendarList.end(); iter++)
	{
		if (mCalendar->IsDirectory())
		{
			// Move mailbox
			static_cast<calstore::CCalendarStoreNode*>(*iter)->MoveCalendar(mCalendar, false);
		}
		else
		{
			// Do calendar -> calendar copy
			static_cast<calstore::CCalendarStoreNode*>(*iter)->CopyCalendarContents(mCalendar);
		}
	}

	return true;
}

bool CDragCalendarAtCalendarTask::ExecuteSelf()
{
	// Validate data
	if (!mParent && !mDestination || !mCalendarList.size())
		return false;

	// Ask user before doing task
	if (CErrorHandler::PutCautionAlertRsrcStr(true, "CCalendarStoreTable::ReallyMoveCalendar", (mParent ? mParent : mDestination)->GetName()) == CErrorHandler::Cancel)
		return false;

	for(calstore::CCalendarStoreNodeList::iterator iter = mCalendarList.begin(); iter != mCalendarList.end(); iter++)
	{
		// Move calendar into directory
		static_cast<calstore::CCalendarStoreNode*>(*iter)->MoveCalendar(mParent ? mParent : mDestination, !mParent);
	}

	return true;
}
