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


// Header for CDragTasks class

#ifndef __CDRAGTASKS__MULBERRY__
#define __CDRAGTASKS__MULBERRY__

#include "CMboxList.h"
#include "CMboxRefList.h"

#include "CCalendarStoreNode.h"

// Consts

// Classes
class CMailboxView;
class CMbox;
class CMboxProtocol;
class CMboxRef;
class CMessage;
class CMessageWindow;
class CServerBrowse;

// Abstract base class for all drag tasks
class CDragTask
{
public:
	CDragTask()
		{ if (sCurrentDragTask) delete sCurrentDragTask; sCurrentDragTask = this; }
	virtual ~CDragTask()
		{ if (sCurrentDragTask == this) sCurrentDragTask = NULL; }

	static CDragTask* GetCurrentDragTask()
		{ return sCurrentDragTask; }

	static bool Execute();

protected:	
	virtual bool ExecuteSelf() = 0;

private:
	static CDragTask* sCurrentDragTask;
};

// Abstract base class for all drag tasks using a server
class CServerDragTask : public CDragTask
{
public:
	CServerDragTask(CServerBrowse* owner)
		{ mOwner = owner; }
	virtual ~CServerDragTask()
		{ mOwner = NULL; }

protected:
	CServerBrowse*	mOwner;
};

class CServerDragOpenServerTask : public CServerDragTask
{
public:
	CServerDragOpenServerTask(CServerBrowse* owner, CMboxProtocol* proto)
		: CServerDragTask(owner)
		{ mProto = proto; }
	virtual ~CServerDragOpenServerTask()
		{ mProto = NULL; }
	
protected:	
	virtual bool ExecuteSelf();

private:
	CMboxProtocol*	mProto;
};

class CDragServerTask : public CDragTask
{
public:
	CDragServerTask(long dropat)
		{ mDropAtItemIndex = dropat; }
	virtual ~CDragServerTask() {}
	
			void AddIndex(unsigned long index);

protected:	
	virtual bool ExecuteSelf();

private:
	long		mDropAtItemIndex;
	ulvector	mMoveIndices;
};

class CDragWDTask : public CDragTask
{
public:
	CDragWDTask(long dropat)
		{ mDropAtItemIndex = dropat; }
	virtual ~CDragWDTask() {}
	
			void AddMboxList(CMboxList* wd);

protected:	
	virtual bool ExecuteSelf();

private:
	long			mDropAtItemIndex;
	CHierarchies	mMoveWDs;
};

class CDragMessagesTask : public CDragTask
{
public:
	CDragMessagesTask(CMbox* to, CMbox* from)
		{ mCopyToMbox = to; mCopyFromMbox = from; }
	virtual ~CDragMessagesTask()
		{ mCopyToMbox = NULL; mCopyFromMbox = NULL; }
	
			void AddMessage(const CMessage* msg);

protected:	
	virtual bool ExecuteSelf();

private:
	CMbox*		mCopyToMbox;				// Mbox to copy to during drop
	CMbox*		mCopyFromMbox;				// Mbox to copy from during drop
	ulvector	mCopyMsgNums;				// Messages numbers to copy during drop
};

class CDragMailboxTask : public CDragTask
{
public:
	CDragMailboxTask() {}
	virtual ~CDragMailboxTask() {}

			void AddMbox(CMbox* mbox);

protected:
	CMboxList	mMboxList;				// Mboxes to operate on
};

class CDragMailboxIntoMailboxTask : public CDragMailboxTask
{
public:
	CDragMailboxIntoMailboxTask(CMbox* mbox)
		{ mMbox = mbox; }
	virtual ~CDragMailboxIntoMailboxTask()
		{ mMbox = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMbox* mMbox;
};

class CDragMailboxAtMailboxTask : public CDragMailboxTask
{
public:
	CDragMailboxAtMailboxTask(CMbox* parent, CMbox* destination)
		{ mParent = parent; mDestination = destination; }
	virtual ~CDragMailboxAtMailboxTask()
		{ mParent = NULL; mDestination = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMbox* mParent;
	CMbox* mDestination;
};

class CDragMailboxToCabinetTask : public CDragMailboxTask
{
public:
	CDragMailboxToCabinetTask(CMboxRefList* cab)
		{ mCabinet = cab; }
	virtual ~CDragMailboxToCabinetTask()
		{ mCabinet = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMboxRefList* mCabinet;
};

class CDragMailboxSubscribeTask : public CDragMailboxTask
{
public:
	CDragMailboxSubscribeTask() {}
	virtual ~CDragMailboxSubscribeTask() {}

protected:	
	virtual bool ExecuteSelf();
};

class CDragMailboxRefTask : public CDragTask
{
public:
	CDragMailboxRefTask() {}
	virtual ~CDragMailboxRefTask() {}

			void AddMboxRef(CMboxRef* mboxref);

protected:
	CMboxRefList	mMboxRefList;				// Mboxes to operate on
};

class CDragMailboxRefToCabinetTask : public CDragMailboxRefTask
{
public:
	CDragMailboxRefToCabinetTask(CMboxRefList* cab)
		{ mCabinet = cab; }
	virtual ~CDragMailboxRefToCabinetTask()
		{ mCabinet = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMboxRefList* mCabinet;
};

class CDragMailboxToView : public CDragTask
{
public:
	CDragMailboxToView(CMailboxView* view, CMbox* mbox)
		{ mView = view; mMbox = mbox; }
	virtual ~CDragMailboxToView()
		{ mView = NULL; mMbox = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMailboxView*	mView;
	CMbox*			mMbox;
};

class CDragMessageToWindow : public CDragTask
{
public:
	CDragMessageToWindow(CMessageWindow* wnd, CMessage* msg)
		{ mWnd = wnd; mMsg = msg; }
	virtual ~CDragMessageToWindow()
		{ mWnd = NULL; mMsg = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	CMessageWindow*	mWnd;
	CMessage*		mMsg;
};

class CDragCalendarServerTask : public CDragTask
{
public:
	CDragCalendarServerTask(long dropat)
		{ mDropAtItemIndex = dropat; }
	virtual ~CDragCalendarServerTask() {}
	
			void AddIndex(unsigned long index);

protected:	
	virtual bool ExecuteSelf();

private:
	long		mDropAtItemIndex;
	ulvector	mMoveIndices;
};

class CDragCalendarTask : public CDragTask
{
public:
	CDragCalendarTask()
	{
		mCalendarList.set_delete_data(false);
	}
	virtual ~CDragCalendarTask() {}

			void AddCalendar(calstore::CCalendarStoreNode* node);

protected:
	calstore::CCalendarStoreNodeList	mCalendarList;				// Mboxes to operate on
};

class CDragCalendarIntoCalendarTask : public CDragCalendarTask
{
public:
	CDragCalendarIntoCalendarTask(calstore::CCalendarStoreNode* node)
		{ mCalendar = node; }
	virtual ~CDragCalendarIntoCalendarTask()
		{ mCalendar = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	calstore::CCalendarStoreNode* mCalendar;
};

class CDragCalendarAtCalendarTask : public CDragCalendarTask
{
public:
	CDragCalendarAtCalendarTask(calstore::CCalendarStoreNode* parent, calstore::CCalendarStoreNode* destination)
		{ mParent = parent; mDestination = destination; }
	virtual ~CDragCalendarAtCalendarTask()
		{ mParent = NULL; mDestination = NULL; }

protected:	
	virtual bool ExecuteSelf();

private:
	calstore::CCalendarStoreNode* mParent;
	calstore::CCalendarStoreNode* mDestination;
};

#endif
