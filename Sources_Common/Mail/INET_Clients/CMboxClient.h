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


// CMboxClient

// Abstract base class that encapsulates the functionality of mailbox/message store.
// This is used by the high level CMboxProtocol object to control the mailbox/message store.
// Derived classes will implement the backend mailbox/message client.
// e.g. CIMAPClient is derived from this class. Another class could be CPOPClient or CLocalClient.

#ifndef __CMBOXCLIENT__MULBERRY__
#define __CMBOXCLIENT__MULBERRY__

#include "CINETClient.h"

#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessage.h"

// Classes

class CSearchItem;

class CMboxClient : public CINETClient
{
public:
		CMboxClient(CMboxProtocol* owner)
			: CINETClient(owner)
			{ mCurrent_msg = NULL;
			  mCurrentResults = NULL;
			  mTargetMbox = NULL;
			  mACL = NULL;
			  mNamespace = NULL; }
		CMboxClient(const CMboxClient& copy, CMboxProtocol* owner)
			: CINETClient(copy, owner)
			{ mCurrent_msg = NULL;
			  mCurrentResults = NULL;
			  mTargetMbox = NULL;
			  mACL = NULL;
			  mNamespace = NULL; }
	virtual ~CMboxClient()
			{ mCurrent_msg = NULL;
			  mCurrentResults = NULL;
			  mTargetMbox = NULL;
			  mACL = NULL;
			  mNamespace = NULL; }

	// Mailbox related
	virtual void	_CreateMbox(CMbox* mbox) = 0;			// Do create
	virtual void	_TouchMbox(CMbox* mbox) = 0;			// Do touch
	virtual bool	_TestMbox(CMbox* mbox) = 0;				// Do test
	virtual void	_RebuildMbox(CMbox* mbox) = 0;			// Rebuild cache
	virtual void	_OpenMbox(CMbox* mbox) = 0;				// Do open mailbox
	virtual void	_CloseMbox(CMbox* mbox) = 0;			// Do close mailbox
	virtual void	_SelectMbox(CMbox* mbox, bool examine = false) = 0;			// Do Selection
	virtual void	_Deselect(CMbox* mbox) = 0;				// Unselect
	virtual void	_CheckMbox(CMbox* mbox,					// Do check
								bool fast = false) = 0;
	virtual void	_MailboxSize(CMbox* mbox) = 0;			// Get mailbox size;
	virtual bool	_DoesMailboxSize() const = 0;			// Does server handle mailbox size?
	virtual bool	_ExpungeMbox(bool closing) = 0;			// Do expunge mailbox
	virtual void	_SearchMbox(const CSearchItem* spec,	// Do message search
								ulvector* results,
								bool uids) = 0;
	virtual void	_DeleteMbox(CMbox* mbox) = 0;			// Do delete mailbox
	virtual void	_RenameMbox(CMbox* mbox_old,
								const char* mbox_new) = 0;	// Do rename mbox
	virtual void	_SubscribeMbox(CMbox* mbox) = 0;		// Do subscribe mbox
	virtual void	_UnsubscribeMbox(CMbox* mbox) = 0;		// Do unsubscribe mbox
	virtual void	_Namespace(CMboxProtocol::SNamespace* names) = 0;	// Get namespace
	virtual void	_FindAllSubsMbox(CMboxList* mboxes) = 0;		// Do find subscribed mboxes
	virtual void	_FindAllMbox(CMboxList* mboxes) = 0;			// Do find all mboxes
	virtual void	_StartAppend(CMbox* mbox) = 0;			// Starting multiple append
	virtual void	_StopAppend(CMbox* mbox) = 0;			// Stopping multiple append
	virtual void	_AppendMbox(CMbox* mbox,
								CMessage* theMsg,
								unsigned long& new_uid,
								bool dummy_files = false) = 0;		// Do append message to mbox
	virtual void	_SetUIDValidity(unsigned long uidv) = 0;	// Set the UIDValidity
	virtual void	_SetUIDNext(unsigned long uidn) = 0;	// Set the UIDNext
	virtual void	_SetLastSync(unsigned long sync) = 0;	// Set the time of the last sync operation

	// Message related
	virtual unsigned long _GetMessageLocalUID(unsigned long uid) = 0;

	virtual void	_FetchItems(const ulvector& nums,					// Do fetch envelopes
										bool uids,
										CMboxProtocol::EFetchItems items) = 0;
	virtual void	_ReadHeaders(const ulvector& nums,				// Get header list from messages
									bool uids,
									const cdstring& hdrs) = 0;
	virtual void	_ReadHeader(CMessage* msg) = 0;					// Get message header from message/rfc822
	virtual void	_ReadAttachment(unsigned long msg_num,
									CAttachment* attach,
									LStream* aStream,
									bool peek = false,
									unsigned long count = 0,
									unsigned long start = 1) = 0;	// Get attachment data into stream
	virtual void	_CopyAttachment(unsigned long msg_num,
									CAttachment* attach,
									costream* aStream,
									bool peek = false,
									unsigned long count = 0,
									unsigned long start = 1) = 0;	// Copy raw attachment data into stream
	virtual void	_RemapUID(unsigned long local_uid,
							unsigned long new_uid) = 0;				// Set the UID
	virtual void	_MapLocalUIDs(const ulvector& uids,				// Map local to remote UIDs
									ulvector* missing,
									ulmap* local_map) = 0;

	virtual void	_SetFlag(const ulvector& nums,
									bool uids,
									NMessage::EFlags flags,
									bool set) = 0;					// Change flag
	virtual void	_CopyMessage(const ulvector& nums,
									bool uids,
									CMbox* mbox_to,
									ulmap& copy_uids) = 0;			// Do copy message to mailbox
	virtual void	_CopyMessage(unsigned long msg_num,
									bool uids,
									costream* aStream,
									unsigned long count = 0,
									unsigned long start = 1) = 0;	// Do copy message to stream
	virtual bool	_DoesCopy() const = 0;							// Does server handle copy?

	virtual void	_ExpungeMessage(const ulvector& nums, bool uids) = 0;			// Expunge uids
	virtual bool	_DoesExpungeMessage() const = 0;				// Does server handle copy?

	// S O R T / T H R E A D
	virtual bool	_DoesSort(ESortMessageBy sortby) const = 0;		// Does server-side sorting
	virtual void    _Sort(ESortMessageBy sortby,					// Do server-side sort
							EShowMessageBy show_by,
							const CSearchItem* search,
							ulvector* results,
							bool uids) = 0;
	
	virtual bool	_DoesThreading(EThreadMessageBy threadby) const = 0;	// Does server-side threading
	virtual void    _Thread(EThreadMessageBy threadby,				// Do server-side thread
							const CSearchItem* search,
							threadvector* results,
							bool uids) = 0;
	
	// ACL related
	virtual void	_SetACL(CMbox* mbox, CACL* acl) = 0;			// Set acl on server
	virtual void	_DeleteACL(CMbox* mbox, CACL* acl) = 0;			// Delete acl on server
	virtual void	_GetACL(CMbox* mbox) = 0;						// Get all acls for mailbox from server
	virtual void	_ListRights(CMbox* mbox, CACL* acl) = 0;		// Get allowed rights for user
	virtual void	_MyRights(CMbox* mbox) = 0;						// Get current user's rights to mailbox

	// Quota related
	virtual void	_SetQuota(CQuotaRoot* root) = 0;				// Set quota root values on server
	virtual void	_GetQuota(CQuotaRoot* root) = 0;				// Get quota root values from server
	virtual void	_GetQuotaRoot(CMbox* mbox) = 0;					// Get quota roots for a mailbox

protected:
	CMessage*		mCurrent_msg;									// Current message to process
	ulvector*		mCurrentResults;								// Results of SEARCH etc
	CMbox*			mTargetMbox;									// Mailbox to target
	CACL*			mACL;											// ACL being processed (LISTRIGHTS only)
	CMboxProtocol::SNamespace*	mNamespace;							// Place to read namespace info into

	CMboxProtocol*	GetMboxOwner() const							// Return type-cast owner
		{ return static_cast<CMboxProtocol*>(mOwner); }

	virtual CMbox*	GetCurrentMbox()								// Get currently selected mailbox
		{ return GetMboxOwner()->GetCurrentMbox(); }
	virtual void	SetMessage(unsigned long msg_num)				// Set message as current
		{CMbox* mbox = GetMboxOwner()->GetCurrentMbox();
			mCurrent_msg = ((mbox && mbox->IsFullOpen()) ? mbox->GetMessage(msg_num) : NULL);}
	virtual void	SetEnvelope(CEnvelope* env)						// Set current message's envelope
		{if (mCurrent_msg) mCurrent_msg->SetEnvelope(env);}

	virtual void	SetBody(CAttachment* body)						// Set current message's body
		{if (mCurrent_msg) mCurrent_msg->SetBody(body);}

};

#endif
