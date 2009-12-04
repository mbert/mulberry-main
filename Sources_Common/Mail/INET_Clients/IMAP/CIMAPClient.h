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


// Header for IMAP client class

#ifndef __CIMAPCLIENT__MULBERRY__
#define __CIMAPCLIENT__MULBERRY__

#include "CMboxClient.h"
#include "CIMAPCommon.h"

// consts
// Others
enum EIMAPVersion
{
	eIMAP2bis = 1,
	eIMAP4,
	eIMAP4rev1
};

const char cType_IMAP2bis[] = "IMAP2bis";
const char cType_IMAP4[] = "IMAP4";
const char cType_IMAP4rev1[] = "IMAP4rev1";

enum
{
	err_FetchNoMemory = '*FET'
};

class CMbox;
class CMboxList;
class CMessage;
class CAttachment;
class CEnvelope;
class CAddress;
class CAddressList;

class LStream;

class CIMAPClient: public CMboxClient
{

	// I N S T A N C E  V A R I A B L E S

private:
	EIMAPVersion	mVersion;						// Version of server
	bool			mFindingHier;					// Finding hierarchy delimiter
	bool			mFindingSubs;					// Finding subscribed mailboxes
	CMboxList*		mCurrentWD;						// Current WD for LIST/LSUB
	unsigned long	mMboxNew;						// Counter to indicate new messages in mbox
	bool			mInPostProcess;					// Flag to indicate post-processing already happening
	bool			mMboxUpdate;					// Flag to indicate update of mbox
	bool			mMboxReset;						// Flag to indicate reset of mbox messages
	bool			mMboxReload;					// Flag to indicate mbox needs to be reread
	bool			mDummyFiles;					// Indicates that file attachments will not be processed
	CMessage*		mProcessMessage;				// Message being appended
	bool			mHasNamespace;					// Supports NAMESPACE extension
	bool			mHasUIDPlus;					// Supports UIDPLUS/LITERAL+ extension
	bool			mHasUnselect;					// Supports UNSELECT
	bool			mHasSort;						// Supports SORT
	bool			mHasThreadSubject;				// Supports THREAD=ORDEREDSUBJECT
	bool			mHasThreadReferences;			// Supports THREAD=REFERENCES
	threadvector*	mThreadResults;					// Place to store thread results

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CIMAPClient(CMboxProtocol* owner);
			CIMAPClient(const CIMAPClient& copy, CMboxProtocol* owner);
	virtual	~CIMAPClient();

private:
			void	InitIMAPClient();

public:
	virtual CINETClient*	CloneConnection();		// Create duplicate, empty connection

protected:
	virtual tcp_port GetDefaultPort();						// Get default port;

	// P R O T O C O L
	virtual void	_InitCapability();					// Initialise capability flags to empty set
	virtual void	_ProcessCapability();				// Handle capability response
	virtual void	_NoCapability();					// Handle failed capability response

	virtual void	_PreProcess();						// About to start processing input
	virtual void	_PostProcess();						// Finished processing input

	virtual void	_ParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// M B O X
	virtual void	_CreateMbox(CMbox* mbox);			// Do create
	virtual void	_TouchMbox(CMbox* mbox)	{}			// Do touch - not done by IMAP
	virtual bool	_TestMbox(CMbox* mbox);				// Do test of mailbox existence
	virtual void	_RebuildMbox(CMbox* mbox);			// Rebuild cache
	virtual void	_OpenMbox(CMbox* mbox);				// Do open mailbox
	virtual void	_CloseMbox(CMbox* mbox);			// Do close mailbox

	virtual void	_SelectMbox(CMbox* mbox,
								bool examine = false);	// Do Selection
	virtual void	_Deselect(CMbox* mbox);				// Unselect

	virtual void	_CheckMbox(CMbox* mbox,				// Do check
								bool fast = false);
	virtual void	_MailboxSize(CMbox* mbox) {}		// Get mailbox size;
	virtual bool	_DoesMailboxSize() const			// Does server handle mailbox size?
		{ return false; }
	virtual bool	_ExpungeMbox(bool closing);			// Do expunge mailbox
	virtual void	_DeleteMbox(CMbox* mbox);			// Do delete mailbox
	virtual void	_RenameMbox(CMbox* mbox_old,
									const char* mbox_new);	// Do rename mbox

	virtual void	_SubscribeMbox(CMbox* mbox);		// Do subscribe mbox
	virtual void	_UnsubscribeMbox(CMbox* mbox);		// Do unsubscribe mbox
	virtual void	_Namespace(CMboxProtocol::SNamespace* names);	// Get namespace
	virtual void	_FindAllSubsMbox(CMboxList* mboxes);		// Do find subscribed mboxes
	virtual void	_FindAllMbox(CMboxList* mboxes);			// Do find all mboxes
	virtual void	_StartAppend(CMbox* mbox) {}		// Starting multiple append
	virtual void	_StopAppend(CMbox* mbox) {}			// Stopping multiple append
	virtual void	_AppendMbox(CMbox* mbox,
									CMessage* theMsg,
									unsigned long& new_uid,
									bool dummy_files = false);	// Do append message to mbox

	virtual void	_SearchMbox(const CSearchItem* spec,	// Search messages on the server
								ulvector* results,
								bool uids);
			void	AddSearchItem(const CSearchItem* spec,	// Add search item to command line
									bool force_charset = false);
			void	AddSearchAddress(const CSearchItem* spec,
										const char* spec_type);	// Add search item referring to an address

	virtual void	_SetUIDValidity(unsigned long uidv) {}	// Set the UIDValidity - not done in IMAP
	virtual void	_SetUIDNext(unsigned long uidn) {}		// Set the UIDNext - not done in IMAP
	virtual void	_SetLastSync(unsigned long sync) {}		// Set the time of the last sync operation - not done in IMAP

	// M E S S A G E S
	virtual unsigned long _GetMessageLocalUID(unsigned long uid)
		{ return 0; }

	virtual void	_FetchItems(const ulvector& nums,					// Do fetch envelopes
										bool uids,
										CMboxProtocol::EFetchItems items);
	virtual void	_ReadHeaders(const ulvector& nums,				// Get header list from messages
									bool uids,
									const cdstring& hdrs);
	virtual void	_ReadHeader(CMessage* msg);					// Get message header
	virtual void	_ReadAttachment(unsigned long msg_num,
									CAttachment* attach,
									LStream* aStream,
									bool peek = false,
									unsigned long count = 0,
									unsigned long start = 1);	// Get attachment data into stream
	virtual void	_ReadAttachment(unsigned long msg_num,
									const char* attach_spec,
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
							unsigned long new_uid) {}			// Set the UID
	virtual void	_MapLocalUIDs(const ulvector& uids,			// Map local to remote UIDs
								ulvector* missing,
								ulmap* local_map) {}

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
									unsigned long start = 1);	// Do copy message to stream
	virtual bool	_DoesCopy() const							// Does server handle copy?
		{ return true; }
	virtual void	_ExpungeMessage(const ulvector& nums, bool uids);	// Expunge uids
	virtual bool	_DoesExpungeMessage() const					// Does server handle copy?
		{ return mHasUIDPlus; }

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
	virtual int		GetManualLiteralLength();				// Get length of manually processed literal
	virtual void	SendManualLiteral();					// Send manual literal

	// H A N D L E  E R R O R
	virtual void	INETRecoverReconnect();						// Force reconnect
	virtual void	INETRecoverDisconnect();					// Force disconnect
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	// H A N D L E  R E S P O N S E
	void 	IMAPParseResponse(char** txt,
							CINETClientResponse* response);			// Parse text sent by server
	void 	IMAPParseMessageResponse(char** txt,
							CINETClientResponse* response);			// Parse <n> message text sent by server

	// P A R S I N G  I M A P  I N F O
	void	IMAPParseListLsub(char** txt, bool lsub);			// Parse IMAP LIST/LSUB reply
	void	IMAPParseMailbox(char** txt,
								char delim,
								NMbox::EFlags mbox_flags);			// Parse IMAP MAILBOX reply
	void	IMAPParseSearch(char** txt);						// Parse IMAP SEARCH reply
	void	IMAPParseStatus(char** txt);						// Parse IMAP STATUS reply

	void	IMAPParseFetch(char** txt);							// Parse IMAP FETCH reply
	void	IMAPParseEnvelope(char** txt);						// Parse IMAP ENVELOPE reply
	void	IMAPParseFlags(char** txt);							// Parse IMAP FLAGS reply
	void	IMAPParseAddressList(char** txt,
									CAddressList* addr_list);	// Parse IMAP style address list
	void	IMAPParseAddress(char** txt,
								CAddress* addr);				// Parse IMAP style address
	void	IMAPParseBody(char** txt);							// Parse IMAP BODY/BODYSTRUCTURE reply
	CAttachment* IMAPParseBodyItem(char** txt,
										CAttachment* parent);	// Parse IMAP BODY/BODYSTRUCTURE item
	CAttachment* IMAPParseBodyContent(char** txt);				// Parse IMAP BODY/BODYSTRUCTURE content
	void	IMAPParseBodyParameter(char** txt,
										CMIMEContent& content,
										bool is_content,
										bool is_disposition);	// Parse IMAP body_fld_param
	void	IMAPParseBodyDisposition(char** txt,
										CMIMEContent& content);	// Parse IMAP body_fld_dsp
	void	IMAPParseBodyLanguage(char** txt,
										CMIMEContent& content);	// Parse IMAP body_fld_lang
	void	IMAPParseBodyExtension(char** txt,
										CMIMEContent& content);	// Parse IMAP body_fld_extension
	void	IMAPParseBodySection(char** txt);					// Parse IMAP BODY[#] reply
	void	IMAPParseBodySectionHeader(char** txt);				// Parse IMAP BODY[HEADER.XXX] reply
	void	IMAPParseInternalDate(char** txt);					// Parse IMAP INTERNALDATE reply
	void	IMAPParseUID(char** txt);							// Parse IMAP UID reply
	void	IMAPParseRFC822(char** txt);						// Parse IMAP RFC822 reply
	void	IMAPParseRFC822Header(char** txt);					// Parse IMAP RFC822.HEADER reply
	void	IMAPParseRFC822Size(char** txt);					// Parse IMAP RFC822.SIZE reply
	void	IMAPParseRFC822Text(char** txt);					// Parse IMAP RFC822.TEXT reply

	// E X T E N S I O N S
	void	IMAPParseSort(char** txt);							// Parse IMAP SORT reply
	void	IMAPParseThread(char** txt);						// Parse IMAP THREAD reply
	void	IMAPParseThreadItem(char** txt,						// Parse IMAP THREAD reply
								unsigned long depth);

	void	IMAPParseACL(char** txt);							// Parse IMAP ACL reply
	void	IMAPParseListRights(char** txt);					// Parse IMAP LISTRIGHTS reply
	void	IMAPParseMyRights(char** txt);						// Parse IMAP MYRIGHTS reply

	void	IMAPParseQuota(char** txt);							// Parse IMAP QUOTA reply
	void	IMAPParseQuotaRoot(char** txt);						// Parse IMAP QUOTAROOT reply

	void	IMAPParseNamespace(char** txt);						// Parse IMAP NAMESPACE reply
	void	IMAPParseNamespaceItem(cdstrpairvect* names,		// Parse IMAP NAMESPACE item
									char** txt);
	
};

#endif
