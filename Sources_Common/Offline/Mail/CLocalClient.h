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


// Header for Local client class

#ifndef __CLOCALCLIENT__MULBERRY__
#define __CLOCALCLIENT__MULBERRY__

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
#define FAST_STREAM
#else
#undef FAST_STREAM
#endif

#include "CMboxClient.h"

#ifdef FAST_STREAM
#include "cdfstream.h"
#else
#include <fstream>
#define cdfstream fstream
#endif

// consts
class CMailRecord;
class CMbox;
class CMessage;
class CLocalAttachment;
class CLocalMessage;
class CProgress;

class LStream;

class CLocalClient: public CMboxClient
{
protected:
	struct SIndexHeader
	{
		unsigned long	mVersion;
		unsigned long	mMboxModified;
		unsigned long	mCacheModified;
		unsigned long	mLastSync;
		unsigned long	mIndexSize;
		unsigned long	mUIDValidity;
		unsigned long	mUIDNext;
		unsigned long	mLastUID;
		unsigned long	mLocalUIDNext;

		unsigned long& Version()
			{ return mVersion; }
		const unsigned long& Version() const
			{ return mVersion; }

		unsigned long& MboxModified()
			{ return mMboxModified; }
		const unsigned long& MboxModified() const
			{ return mMboxModified; }

		unsigned long& CacheModified()
			{ return mCacheModified; }
		const unsigned long& CacheModified() const
			{ return mCacheModified; }

		unsigned long& LastSync()
			{ return mLastSync; }
		const unsigned long& LastSync() const
			{ return mLastSync; }

		unsigned long& IndexSize()
			{ return mIndexSize; }
		const unsigned long& IndexSize() const
			{ return mIndexSize; }

		unsigned long& UIDValidity()
			{ return mUIDValidity; }
		const unsigned long& UIDValidity() const
			{ return mUIDValidity; }

		unsigned long& UIDNext()
			{ return mUIDNext; }
		const unsigned long& UIDNext() const
			{ return mUIDNext; }

		unsigned long& LastUID()
			{ return mLastUID; }
		const unsigned long& LastUID() const
			{ return mLastUID; }

		unsigned long& LocalUIDNext()
			{ return mLocalUIDNext; }
		const unsigned long& LocalUIDNext() const
			{ return mLocalUIDNext; }

		void write(std::ostream& out) const;
		void write_LastSync(std::ostream& out) const;
		void write_IndexSize(std::ostream& out) const;
		void write_UIDValidity(std::ostream& out) const;
		void write_UIDNext(std::ostream& out) const;
		void write_LastUID(std::ostream& out) const;
		void write_LocalUIDNext(std::ostream& out) const;
		void read(std::istream& in);
	};

	struct SIndexRecord
	{
		unsigned long mCache;
		unsigned long mIndex;
		unsigned long mFlags;
		unsigned long mUID;
		unsigned long mLocalUID;
		unsigned long mMessageStart;

		unsigned long mSequence;		// Not part of cache but calculated on load

		SIndexRecord()
			{ mCache = 0;
			  mIndex = 0;
			  mFlags = 0;
			  mUID = 0;
			  mLocalUID = 0;
			  mMessageStart = 0;
			  mSequence = 0; }

		unsigned long& Cache()
			{ return mCache; }
		const unsigned long& Cache() const
			{ return mCache; }
		unsigned long& Index()
			{ return mIndex; }
		const unsigned long& Index() const
			{ return mIndex; }
		unsigned long& Flags()
			{ return mFlags; }
		const unsigned long& Flags() const
			{ return mFlags; }
		unsigned long& UID()
			{ return mUID; }
		const unsigned long& UID() const
			{ return mUID; }
		unsigned long& LocalUID()
			{ return mLocalUID; }
		const unsigned long& LocalUID() const
			{ return mLocalUID; }
		unsigned long& MessageStart()
			{ return mMessageStart; }
		const unsigned long& MessageStart() const
			{ return mMessageStart; }

		unsigned long& Sequence()
			{ return mSequence; }
		const unsigned long& Sequence() const
			{ return mSequence; }
		
		void write(std::ostream& out) const;
		void write_Flags(std::ostream& out) const;
		void write_UID(std::ostream& out) const;
		void read(std::istream& in);
	};

	typedef std::vector<SIndexRecord>	SIndexList;
	typedef std::vector<SIndexRecord*>	SIndexRefList;

	// I N S T A N C E  V A R I A B L E S

	CMailRecord*	mRecorder;						// Recording object
	unsigned long	mRecordID;						// Recording ID
	bool			mSeparateUIDs;					// Local and remote UIDs in use
	bool			mRecordLocalUIDs;				// Record local UIDs only
	CMessage*		mProcessMessage;				// Message being appended
	cdstring		mCWD;							// Working directory for entire hierarchy
	EEndl			mEndl;							// Line end format for current streams
	cdfstream		mMailbox;
	cdfstream		mCache;
	cdfstream		mIndex;
	SIndexList		mIndexList;
	ulvector		mIndexMapping;					// Mapping from message number to file index
	unsigned long	mMboxNew;						// Counter to indicate new messages in mbox
	bool			mInPostProcess;					// Flag to indicate post-processing already happening
	bool			mMboxUpdate;					// Flag to indicate update of mbox
	bool			mMboxReset;						// Flag to indicate reset of mbox messages
	bool			mMboxReload;					// Flag to indicate mbox needs to be reread
	bool			mMboxReadWrite;					// Flag indicating it was opened read-write

	std::istream::pos_type mAppendStart;					// Start of multiple append

	cdfstream*		mAppendMailbox;
	cdfstream*		mAppendCache;
	cdfstream*		mAppendIndex;

	char*			mSearchBuffer;					// Buffer used in text searches
	cdstring		mWorkBuffer;					// File I/O buffer

	CProgress*		mProgress;						// Progress indicator

	bool			mPendingAbort;					// Abort of operation is pending

	class StLocalProcess
	{
	public:
		StLocalProcess(CLocalClient* client)
			{ mClient = client; mClient->_PreProcess(); }
		~StLocalProcess()
			{ mClient->_PostProcess(); }
	private:
		CLocalClient* mClient;
	};

	friend class StLocalProcess;

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CLocalClient(CMboxProtocol* owner);
			CLocalClient(const CLocalClient& copy, CMboxProtocol* owner);
	virtual	~CLocalClient();

private:
			void	InitLocalClient();

public:
	virtual CINETClient*	CloneConnection();		// Create duplicate, empty connection

	// S T A R T  &  S T O P
	virtual void	SetRecorder(CMailRecord* recorder)
		{ mRecorder = recorder; }

	virtual void	Open();									// Start
	virtual void	Reset();								// Reset account
protected:
	virtual void	CheckCWD();								// Check CWD
public:
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort
	virtual void	Forceoff();								// Forced close

	// L O G I N  &  L O G O U T
	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server

	// P R O T O C O L
	virtual void	_Tickle(bool force_tickle);			// Do tickle
protected:
	virtual void	_PreProcess();						// About to start processing input
	virtual void	_PostProcess();						// Finished processing input

	// M B O X
	virtual void	_CreateMbox(CMbox* mbox);			// Do create
	virtual void	_TouchMbox(CMbox* mbox);			// Do touch
	virtual bool	_TestMbox(CMbox* mbox);				// Do test of mailbox existence
	virtual void	_RebuildMbox(CMbox* mbox);			// Rebuild cache
	virtual void	_OpenMbox(CMbox* mbox);				// Do open mailbox
	virtual void	_CloseMbox(CMbox* mbox);			// Do close mailbox

	virtual void	_SelectMbox(CMbox* mbox,
								bool examine = false);	// Do Selection
	virtual void	_Deselect(CMbox* mbox);				// Unselect

	virtual void	_CheckMbox(CMbox* mbox,				// Do check
								bool fast = false);
	virtual void	_MailboxSize(CMbox* mbox);			// Get mailbox size;
	virtual bool	_DoesMailboxSize() const			// Does server handle mailbox size?
		{ return true; }
	virtual bool	_ExpungeMbox(bool closing);			// Do expunge mailbox
	virtual void	_DeleteMbox(CMbox* mbox);			// Do delete mailbox
	virtual void	_RenameMbox(CMbox* mbox_old,
									const char* mbox_new);	// Do rename mbox

	virtual void	_SubscribeMbox(CMbox* mbox);		// Do subscribe mbox
	virtual void	_UnsubscribeMbox(CMbox* mbox);		// Do unsubscribe mbox
	virtual void	_Namespace(CMboxProtocol::SNamespace* names);	// Get namespace
	virtual void	_FindAllSubsMbox(CMboxList* mboxes);// Do find subscribed mboxes
	virtual void	_FindAllMbox(CMboxList* mboxes);	// Do find all mboxes in WD
	virtual void	_StartAppend(CMbox* mbox);			// Starting multiple append
	virtual void	_StopAppend(CMbox* mbox);			// Stopping multiple append
	virtual void	_AppendMbox(CMbox* mbox,
									CMessage* theMsg,
									unsigned long& new_uid,
									bool dummy_files = false);	// Do append message to mbox

	virtual void	_SearchMbox(const CSearchItem* spec,	// Search messages on the server
								ulvector* results,
								bool uids);
			void	AddSearchItem(const CSearchItem* spec);	// Add search item to command line
			void	AddSearchAddress(const CSearchItem* spec,
										const char* spec_type);	// Add search item referring to an address

	virtual void	_SetUIDValidity(unsigned long uidv);	// Set the UIDValidity
	virtual void	_SetUIDNext(unsigned long uidn);		// Set the UIDNext
	virtual void	_SetLastSync(unsigned long sync);		// Set the time of the last sync operation

	// M E S S A G E S
	virtual unsigned long _GetMessageLocalUID(unsigned long uid);

	virtual void	_FetchItems(const ulvector& nums,					// Do fetch envelopes
										bool uids,
										CMboxProtocol::EFetchItems items);
	virtual void	_ReadHeaders(const ulvector& nums,				// Get header list from messages
									bool uids,
									const cdstring& hdrs);
	virtual void	_ReadHeader(CMessage* msg);					// Get message header text from server
	virtual void	_ReadAttachment(unsigned long msg_num,
									CAttachment* attach,
									LStream* aStream,
									bool peek = false,
									unsigned long count = 0,
									unsigned long start = 1);	// Get attachment data into stream
	virtual void	_CopyAttachment(unsigned long msg_num,
									CAttachment* attach,
									costream* aStream,
									bool peek = false,
									unsigned long count = 0,
									unsigned long start = 1);	// Copy raw attachment data into stream

	virtual void	_RemapUID(unsigned long local_uid,
							unsigned long new_uid);				// Set the UID
	virtual void	_MapLocalUIDs(const ulvector& uids,				// Map local to remote UIDs
									ulvector* missing,
									ulmap* local_map);

	virtual void	_SetFlag(const ulvector& nums,
									bool uids,
									NMessage::EFlags flags,
									bool set);					// Change flag

	virtual void	_CopyMessage(const ulvector& nums,
									bool uids,
									CMbox* mbox_to,
									ulmap& copy_uids);			// Do copy sequence to mailbox
	virtual void	_CopyMessage(unsigned long msg_num,
									bool uids,
									costream* aStream,
									unsigned long count = 0,
									unsigned long start = 1);			// Do copy message to stream
	virtual bool	_DoesCopy() const							// Does server handle copy?
		{ return false; }

	virtual void	_ExpungeMessage(const ulvector& nums, bool uids);	// Expunge uids
	virtual bool	_DoesExpungeMessage() const					// Does server handle copy?
		{ return true; }

	// S O R T / T H R E A D
	virtual bool	_DoesSort(ESortMessageBy sortby) const;			// Does server-side sorting
	virtual void    _Sort(ESortMessageBy sortby,					// Do server-side sort
							EShowMessageBy show_by,
							const CSearchItem* search,
							ulvector* results,
							bool uids);
	
	virtual bool	_DoesThreading(EThreadMessageBy threadby) const;	// Does server-side threading
	virtual void    _Thread(EThreadMessageBy threadby,				// Do server-side thread
							const CSearchItem* search,
							threadvector* results,
							bool uids);

	// A C L S
	virtual void	_SetACL(CMbox* mbox, CACL* acl);			// Set acl on server
	virtual void	_DeleteACL(CMbox* mbox, CACL* acl);			// Delete acl on server
	virtual void	_GetACL(CMbox* mbox);						// Get all acls for mailbox from server
	virtual void	_ListRights(CMbox* mbox, CACL* acl);		// Get allowed rights for user
	virtual void	_MyRights(CMbox* mbox);						// Get current user's rights to mailbox

	// Q U O T A S
	virtual void	_SetQuota(CQuotaRoot* root);				// Set quota root values on server
	virtual void	_GetQuota(CQuotaRoot* root);				// Get quota root values from server
	virtual void	_GetQuotaRoot(CMbox* mbox);					// Get quota roots for a mailbox

protected:
	// H A N D L E  E R R O R
	virtual void	INETHandleError(std::exception& ex,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition

	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	// L O C A L  O P S
			void	GetNames(const CMbox* mbox, cdstring& mbox_name, cdstring& cache_name, cdstring& index_name) const;
			void	GetNames(const char* mbox, cdstring& mbox_name, cdstring& cache_name, cdstring& index_name) const;

			void	OpenCache(CMbox* mbox,						// Open cache files
								cdfstream& mailbox,
								cdfstream& cache,
								cdfstream& index,
								SIndexHeader& index_header,
								SIndexList& index_list,
								ulvector* index_mapping,
								bool examine);
			void	ValidateCache(CMbox* mbox,					// Check for valid cache files
								cdfstream& mailbox,
								cdfstream& cache,
								cdfstream& index,
								bool examine);

			void	SwitchCache(CMbox* mbox,					// Change streams from read-only <-> read-write
									cdfstream& index,
									cdfstream& mailbox,
									cdfstream& cache,
									bool read_only);

			void	Reconstruct(CMbox* mbox);					// Reconstruct cache and index
			bool	ReconstructRecover(CMbox* mbox,				// Reconstruct from old cache and index
								CMessageList* list,
								SIndexHeader& index_header,
								ulvector& local_uids,
								bool force = false);
			void	UpdateIndexSize(cdfstream& index,			// Update the index header size
								unsigned long size);
			void	UpdateIndexLastUID(cdfstream& index,		// Update the index last uid
								unsigned long luid);
			void	SyncIndexHeader(CMbox* mbox,				// Update the index header
								cdfstream& index);
			unsigned long CreateIndexVersion();					// Create index version

			void	ReadIndex(cdfstream& in,					// Read index file from disk
								SIndexHeader& header,
								SIndexList& index,
								ulvector* seq);
			void	ReadIndexHeader(cdfstream& in, SIndexHeader& header);	// Read index file from disk

			void	WriteIndex(cdfstream& out, unsigned long index);		// Write back single index entry
			void	WriteIndexFlag(cdfstream& out, unsigned long index);	// Write back single index flag entry
			
			void	ReadMessageIndex(CLocalMessage* lmsg,		// Read in this message's index
										ulvector* indices = NULL);
			void	ReadMessageCache(CLocalMessage* lmsg);		// Read in this message's cache

			void	ClearRecent();								// Clear all recent flags

			void	ConvertSequence(const ulvector& sequence,
										ulvector& nums,
										unsigned long total);									// Map sequence to nums
			void	MapToUIDs(const ulvector& nums, ulvector& uids, bool local) const;			// Map message numbers to (local) uids
			void	MapFromUIDs(const ulvector& uids, ulvector& nums, bool local) const;		// Map uids to message numbers
			void	MapBetweenUIDs(const ulvector& uids, ulvector& luids, bool to_local, 		// Map between uids and local uids
									ulvector* missing = NULL, ulmap* local_map = NULL) const;

			unsigned long GetIndex(unsigned long seq) const;
			unsigned long GetIndex(const CMessage* msg) const
				{ return GetIndex(msg->GetMessageNumber()); }

			void	ScanDirectory(const char* path, const cdstring& pattern, CMboxList* mboxes, bool first = false);
			void	AddMbox(const char* path_name, CMboxList* mboxes, NMbox::EFlags flags);

			void	FetchMessage(CLocalMessage* msg,					// Do fetch envelopes
								 	unsigned long seq,
									CMboxProtocol::EFetchItems items);

			void	ExpungeMessage(ulvector& nums);

			void	CheckFromIndex(CMbox* mbox, const SIndexList& index);

			void	CopyMessage(const CLocalMessage* lmsg,
										costream* aStream,
										unsigned long count,
										unsigned long start);
			unsigned long	AppendMessage(CMbox* mbox,
									CMessage* msg,
									bool add,
									bool copying,
									CLocalClient* copier,
									bool dummy_files = false);
			void	RollbackAppend(CMbox* mbox, cdfstream* stream, std::istream::pos_type old_start);

			bool	SearchMessage(const CLocalMessage* lmsg, const CSearchItem* spec);
			bool	AddressSearch(const CLocalMessage* lmsg, const CSearchItem* spec);
			bool	HeaderSearch(const CLocalMessage* lmsg, const cdstring& hdr, const cdstring& txt, unsigned long start);
			bool	TextSearch(const CLocalMessage* lmsg, const cdstring& txt, unsigned long start, bool do_header);
			bool	TextAttachmentSearch(const CLocalAttachment* lattach, const cdstring& txt, unsigned long start);
			bool	TextIndexSearch(const ulvector& indices, const cdstring& txt, unsigned long start);

			bool	DateCompare(time_t date1, time_t date2, int comp) const;
			time_t	DateRead(const CLocalMessage* lmsg);
			time_t	InternalDateRead(const CLocalMessage* lmsg);

			bool	StreamSearch(std::istream& in, unsigned long start, unsigned long length, const cdstring& txt, EContentTransferEncoding cte);

			bool	SearchBuffer(const char* str, unsigned long n, const char* pat, unsigned long& pat_pos, bool crlf_convert);

			bool	StreamSearch1522(std::istream& in, unsigned long start, unsigned long length, const cdstring& txt);

	static	bool	uid_index_sort(SIndexRecord* rec1, SIndexRecord* rec2);
};

#endif
