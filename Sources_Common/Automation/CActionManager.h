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


// Header for CActionManager class

#ifndef __CACTIONMANAGER__MULBERRY__
#define __CACTIONMANAGER__MULBERRY__

#include "templs.h"

#include "CMessageFwd.h"
#include "CMIMESupport.h"

// Classes
class CAddressBook;
class CAddressList;
class CAttachment;
class CDataAttachment;
class CDSN;
class CIdentity;
class CMbox;
class CMboxProtocol;
class CMessage;
class CMessageList;

class CActionManager
{
public:
	
	// Mailbox UI processing
	static bool OpenMailbox(CMbox* mbox, bool use3pane = false, bool is_search = false);
	static bool CloseMailbox(CMbox* mbox);

	// Address Book UI processing
	static bool OpenAddressBook(CAddressBook* adbk, bool use3pane = false);
	static void Open3PaneAddressBook();

	// Message UI processing
	static void	MessageSeenChange(CMessage* msg);			// User 'saw' the message for the first time

	static bool ForwardMessage(CMessage* msg, bool msg_wnd = false,
								const cdstring& quote = cdstring::null_str, EContentSubType subtype = eContentSubPlain,
								bool use_dialog = false);
	static bool ForwardMessages(CMessageList* msgs, bool use_dialog);

	static bool BounceMessage(CMessage* msg);

	static bool RejectMessage(CMessage* msg);

	static bool SendAgainMessage(CMessage* msg);

	static bool CreateDigest(CMessageList* msgs);

	static bool NewDraft(const CMbox* mbox);
	static bool NewDraft(const CMboxProtocol* acct);
	static bool NewDraft(CAddressList* to = NULL, CAddressList* cc = NULL, CAddressList* bcc = NULL);
	static bool NewDraft(CAddressList* to, CAddressList* cc, CAddressList* bcc,
							const cdstring& subject, const cdstring& body, const cdstrvect& files, const CIdentity* id = NULL);

	static bool CopyMessage(CMbox* from, CMbox* to, ulvector* nums, bool delete_after);

	static bool MDNMessage(CMessage* msg);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static bool	PrintMessage(CMessage* msg, LPrintSpec* printSpec);
#else
	static bool	PrintMessage(CMessage* msg);
#endif

	static bool SaveMessage(CMessage* msg);
	static bool SaveMessages(CMessageList*& msgs);
	
	// iMIP processing
	static bool NewDraftiTIP(const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id = NULL, CMessage* msg = NULL);
	static bool NewDraftiTIP(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id = NULL, CMessage* msg = NULL);
	static bool NewDraftiTIPManual(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id = NULL, CMessage* msg = NULL);
	static bool NewDraftiTIPAutomatic(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id = NULL, CMessage* msg = NULL);

	// Direct draft processing
	static bool ReplyMessages(CMessageList& msgs, bool quote, NMessage::EReplyType reply,
								const cdstring& text, const cdstring& identity, bool tied);
	static bool CreateReplyMessages(CMessageList& msgs, bool quote, NMessage::EReplyType reply,
								const cdstring& text, const cdstring& identity, bool tied);
	static bool ForwardMessages(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
								const cdstring& text, const cdstring& identity, bool tied);
	static bool CreateForwardMessages(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
								const cdstring& text, const cdstring& identity, bool tied);
	static bool BounceMessages(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied);
	static bool CreateBounceMessages(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied);
	static bool RejectMessages(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied);
	static bool CreateRejectMessages(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied);
	
	static bool SendMessage(CMessage& msg, const CIdentity* id, const CDSN* dsn, NMessage::EDraftType type, CMessageList* msgs, const char* bounce = NULL);

private:
	static CAddressBook*	sOpenAdbk;

	// Never created - always static
	CActionManager() {}
	~CActionManager() {}

	static CMessage* CreateMessage(CAddressList* to_list, CAddressList* cc_list, CAddressList* bcc_list,
										const char* subject, CAttachment* body, const CIdentity* identity, const CDSN* dsn,
										CMessageList* msgs, const char* bounce);

	static const CIdentity* GetIdentity(CMessageList& msgs, const cdstring& identity, bool tied);
	static void SetIdentity(CAddressList* to_list, CAddressList* cc_list, CAddressList* bcc_list, const CIdentity* identity);

	static void IncludeMessage(CMessage& theMsg, bool forward, cdstring& text);
	static void IncludeMessageText(CMessage& theMsg,
							const char* msg_txt,
							bool forward,
							EContentSubType type,
							bool is_flowed,
							cdstring& text);
	static void IncludeSignature(cdstring& text, const CIdentity* identity);
};

#endif
