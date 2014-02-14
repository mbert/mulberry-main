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


// Source for CActionItem class

#include "CActionItem.h"

#include "CActionManager.h"
#include "CAdminLock.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"

#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif

#include <algorithm>
#include <iterator>

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* cActionDescriptors[] =
	{"None",
	 "Flag Mailbox",
	 "Copy Message",
	 "Move Message",
	 "Reply",
	 "Forward",
	 "Bounce",
	 "Reject",
	 "Expunge",
	 "Print",
	 "Save",
	 "Sound",
	 "Alert",
	 "Speak",
	 "Keep",
	 "Discard",
	 "Reject",
	 "Redirect",
     "FileInto",
     "SetFlag",
	 "Vacation",
	 NULL};

const char* cPrefsDescriptors[] =		// Due to duplicate 'Reject' actions above we need a way to discriminate in prefs
	{"None",
	 "Flag Mailbox",
	 "Copy Message",
	 "Move Message",
	 "Reply",
	 "Forward",
	 "Bounce",
	 "Reject",
	 "Expunge",
	 "Print",
	 "Save",
	 "Sound",
	 "Alert",
	 "Speak",
	 "Keep",
	 "Discard",
	 "Reject_SIEVE",
	 "Redirect",
	 "FileInto",
     "SetFlag",
	 "Vacation",
	 NULL};

void CActionItem::_copy(const CActionItem& copy)
{
	mType = copy.mType;

	// Copy data based on type
	switch(mType)
	{
	case eNone:
	case eExpunge:
	case ePrint:
	case eKeep:
	case eDiscard:
	default:
		mData = NULL;
		break;
	case eFlagMessage:
		mData = new CDataItem<CActionFlags>(*copy.GetFlagData());
		break;
	case eCopyMessage:
	case eMoveMessage:
	case eAlert:
	case eSpeak:
	case eReject:
	case eRedirect:
	case eFileInto:
		mData = new CDataItem<cdstring>(*copy.GetStringData());
		break;
	case eReplyMessage:
		mData = new CDataItem<CActionReply>(*copy.GetReplyData());
		break;
	case eForwardMessage:
		mData = new CDataItem<CActionForward>(*copy.GetForwardData());
		break;
	case eBounceMessage:
		mData = new CDataItem<CActionBounce>(*copy.GetBounceData());
		break;
	case eRejectMessage:
		mData = new CDataItem<CActionReject>(*copy.GetRejectData());
		break;
	case eSave:
		mData = new CDataItem<bool>(*copy.GetBoolData());
		break;
	case eSound:
		mData = new CDataItem<COSStringMap>(*copy.GetOSStringMapData());
		break;
    case eSetFlag:
        mData = new CDataItem<CActionFlags>(*copy.GetFlagData());
        break;
	case eVacation:
		mData = new CDataItem<CActionVacation>(*copy.GetVacationData());
		break;
	}
}

void CActionItem::_tidy()
{
	// Delete data based on type
	delete mData;
	mData = NULL;
}

void CActionItem::Execute(CMbox* mbox, const ulvector& uids) const
{
	// Log
	if (CPreferences::sPrefs->GetFilterManager()->DoLog())
		DescribeAction();

	// Copy data based on type
	switch(mType)
	{
	case eNone:
	default:
		return;
	case eFlagMessage:
		ExecuteFlags(mbox, uids);
		return;
	case eCopyMessage:
		ExecuteCopyMove(mbox, uids, false);
		return;
	case eMoveMessage:
		ExecuteCopyMove(mbox, uids, true);
		return;
	case eReplyMessage:
		ExecuteReply(mbox, uids);
		return;
	case eForwardMessage:
		ExecuteForward(mbox, uids);
		return;
	case eBounceMessage:
		ExecuteBounce(mbox, uids);
		return;
	case eRejectMessage:
		ExecuteReject(mbox, uids);
		return;
	case eExpunge:
		ExecuteExpunge(mbox, uids);
		return;
	case ePrint:
		ExecutePrint(mbox, uids);
		return;
	case eSave:
		ExecuteSave(mbox, uids);
		return;
	case eSound:
		ExecuteSound(mbox, uids);
		return;
	case eAlert:
		ExecuteAlert(mbox, uids);
		return;
	case eSpeak:
		ExecuteSpeech(mbox, uids);
		return;
	}
}

void CActionItem::DescribeAction() const
{
	// Copy data based on type
	switch(mType)
	{
	default:
		{
			cdstring what("    Unsupported action: ");
			what += cActionDescriptors[mType];
			CPreferences::sPrefs->GetFilterManager()->Log(what);
		}
		return;
    case eFlagMessage:
    case eSetFlag:
		{
			cdstring what("    Action: ");
			what += cActionDescriptors[mType];
			what += " using ";
			if (mData)
				what += mData->GetInfo();
			CPreferences::sPrefs->GetFilterManager()->Log(what);
		}
		return;
	case eCopyMessage:
	case eMoveMessage:
		{
			cdstring what("    Action: ");
			what += cActionDescriptors[mType];
			what += " to mailbox ";
			if (mData)
				what += mData->GetInfo();
			CPreferences::sPrefs->GetFilterManager()->Log(what);
		}
		return;
	case eNone:
	case eReplyMessage:
	case eForwardMessage:
	case eBounceMessage:
	case eRejectMessage:
	case eExpunge:
	case ePrint:
	case eSave:
	case eSound:
	case eAlert:
	case eSpeak:
		{
			cdstring what("    Action: ");
			what += cActionDescriptors[mType];
			CPreferences::sPrefs->GetFilterManager()->Log(what);
		}
		return;
	}
}

void CActionItem::ExecuteFlags(CMbox* mbox, const ulvector& uids) const
{
	NMessage::EFlags flags = GetFlagData()->GetData().GetFlags();
	bool set = GetFlagData()->GetData().IsSet();
	mbox->SetFlagMessage(uids, true, flags, set);
}

void CActionItem::ExecuteCopyMove(CMbox* mbox, const ulvector& uids, bool move) const
{
	cdstring copy_to = GetStringData()->GetData();

	// Create a mailbox reference which we will resolve
	CMboxRef ref(copy_to, '.');

	// Try to resolve mailbox - force it to be listed if not already
	CMbox* mbox_to = ref.ResolveMbox(true);
	
	// Only do copy if destination exists and not the same as source
	if (mbox_to && (mbox_to->GetAccountName() != mbox->GetAccountName()))
	{
		// Do copy followed by delete
		// NB Copy now automatically does check of destination mailbox
		ulmap ignore;
		mbox->CopyMessage(uids, true, mbox_to, ignore);
		
		// Do delete of originals if moving
		if (move)
			mbox->SetFlagMessage(uids, true, NMessage::eDeleted, true);
	}
	
	// For safety reasons we need to generate an error when copying to the same/missing mailbox
	// This is to prevent subsquent actions acting on the messages, and possibly
	// deleting the only copy of them
	else
	{
		// Log the failure
		if (CPreferences::sPrefs->GetFilterManager()->DoLog())
		{
			cdstring what;
			if (!mbox_to)
				what = "      Failed to copy messages because destination mailbox does not exist: ";
			else
				what = "      Failed to copy messages because source and destination mailboxes are the same: ";
			what += copy_to;
			CPreferences::sPrefs->GetFilterManager()->Log(what);
		}

		// Throw as this is fatal
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}
}

void CActionItem::ExecuteReply(CMbox* mbox, const ulvector& uids) const
{
	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// One reply per message
	const CActionReply& reply = GetReplyData()->GetData();
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		// Get messages from uids
		CMessageList msgs;
		msgs.SetOwnership(false);

		msgs.push_back(mbox->GetMessageUID(*iter));

		if (reply.CreateDraft())
		{
			// Do reply action as a task as window needs to be created
			CCreateReplyTask* task = new CCreateReplyTask(msgs,	
															reply.Quote(), 
															reply.GetReplyType(),
															reply.GetText(),
															reply.GetIdentity(),
															reply.UseTiedIdentity());
			task->Go();
		}
		else
			// Do reply action
			CActionManager::ReplyMessages(msgs,	
										reply.Quote(), 
										reply.GetReplyType(),
										reply.GetText(),
										reply.GetIdentity(),
										reply.UseTiedIdentity());
	}
}

void CActionItem::ExecuteForward(CMbox* mbox, const ulvector& uids) const
{
	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// One forward per message
	const CActionForward& forward = GetForwardData()->GetData();
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		// Get messages from uids
		CMessageList msgs;
		msgs.SetOwnership(false);

		msgs.push_back(mbox->GetMessageUID(*iter));

		if (forward.CreateDraft())
		{
			// Do forward action as a task as window needs to be created
			CCreateForwardTask* task = new CCreateForwardTask(msgs,	
																forward.Quote(), 
																forward.Attach(), 
																forward.Addrs(),
																forward.GetText(),
																forward.GetIdentity(),
																forward.UseTiedIdentity());
			task->Go();
		}
		else
			// Do forward action
			CActionManager::ForwardMessages(msgs,	
											forward.Quote(), 
											forward.Attach(), 
											forward.Addrs(),
											forward.GetText(),
											forward.GetIdentity(),
											forward.UseTiedIdentity());
	}
}

void CActionItem::ExecuteBounce(CMbox* mbox, const ulvector& uids) const
{
	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// One forward per message
	const CActionBounce& bounce = GetBounceData()->GetData();
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		// Get messages from uids
		CMessageList msgs;
		msgs.SetOwnership(false);

		msgs.push_back(mbox->GetMessageUID(*iter));

		if (bounce.CreateDraft())
		{
			// Do bounce action as a task as window needs to be created
			CCreateBounceTask* task = new CCreateBounceTask(msgs,
															bounce.Addrs(),
															bounce.GetIdentity(),
															bounce.UseTiedIdentity());
			task->Go();
		}
		else
			// Do bounce action
			CActionManager::BounceMessages(msgs,
											bounce.Addrs(),
											bounce.GetIdentity(),
											bounce.UseTiedIdentity());
	}
}

void CActionItem::ExecuteReject(CMbox* mbox, const ulvector& uids) const
{
	// Prevent reject if admin locks it out
	if (!CAdminLock::sAdminLock.mAllowRejectCommand)
		return;

	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// One reject per message
	const CActionReject& reject = GetRejectData()->GetData();
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		// Check that this message can be rejected
		CMessage* msg = mbox->GetMessageUID(*iter);
		if (!msg->CanReject())
		{
			// Log the failure
			if (CPreferences::sPrefs->GetFilterManager()->DoLog())
			{
				cdstring what("      Failed to reject message because it does not have suitable headers, uid: ");
				what += cdstring(*iter);
				CPreferences::sPrefs->GetFilterManager()->Log(what);
			}

			// Silently go to the next one - this is not a fatal error
			continue;
		}

		// Get messages from uids
		CMessageList msgs;
		msgs.SetOwnership(false);

		msgs.push_back(msg);

		if (reject.CreateDraft())
		{
			// Do reject action as a task as window needs to be created
			CCreateRejectTask* task = new CCreateRejectTask(msgs,	
															reject.ReturnMessage(), 
															reject.GetIdentity(),
															reject.UseTiedIdentity());
			task->Go();
		}
		else
			// Do reject action
			CActionManager::RejectMessages(msgs,	
										reject.ReturnMessage(), 
										reject.GetIdentity(),
										reject.UseTiedIdentity());
	}
}

void CActionItem::ExecuteExpunge(CMbox* mbox, const ulvector& uids) const
{
	// Look for undeleted messages in the set to be expunged
	ulvector undeleted;
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		CMessage* msg = mbox->GetMessageUID(*iter);
		if (msg && !msg->IsDeleted())
			undeleted.push_back(*iter);
	}

	// This is no longer done. I've decided it is too dangerous to allow rules to expunge
	// messages that are not already marked as deleted. This would behave differently than
	// ordinary user actions and could result in unexpected message loss.
#if 0
	// Force set deleted flag
	if (undeleted.size())
		mbox->SetFlagMessage(undeleted, true, NMessage::eDeleted, true);
#endif

	// Remove undeleted from the set
	ulvector actual_uids;
	std::set_difference(uids.begin(), uids.end(), undeleted.begin(), undeleted.end(), std::back_inserter<ulvector>(actual_uids));

	// Now do UID expunge
	mbox->ExpungeMessage(actual_uids, true);
}

void CActionItem::ExecutePrint(CMbox* mbox, const ulvector& uids) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	//  Hide status window as Print Manager gets name of top window and does not know about floats
	try
	{
		LPrintSpec printSpec;
		StPrintSession	session(printSpec);
		StStatusWindowHide hide;
		if (UPrinting::AskPrintJob(printSpec))
		{
			// Must force messages into cache
			mbox->CacheMessage(uids, true);
			
			// One print per message
			for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
			{
				// Get message to print
				CMessage* msg = mbox->GetMessageUID(*iter);
				
				// Print it
				CActionManager::PrintMessage(msg, &printSpec);
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);


		// Print job status window might mess up window order
		// Always do this just in case...
		UDesktop::NormalizeWindowOrder();

		// throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Print job status window might mess up window order
	UDesktop::NormalizeWindowOrder();
#elif __dest_os == __win32_os
	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// One print per message
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		// Get message to print
		CMessage* msg = mbox->GetMessageUID(*iter);
		
		// Print it
		CActionManager::PrintMessage(msg);
	}
#endif
}

void CActionItem::ExecuteSave(CMbox* mbox, const ulvector& uids) const
{
	// Must force messages into cache
	mbox->CacheMessage(uids, true);
	
	// Check for single file or multiple
	if (GetBoolData()->GetData())
	{
		// Create message list that does not delete messages when it is deleted
		CMessageList* msgs = new CMessageList;
		msgs->SetOwnership(false);
		
		// Add messages to list
		for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
			msgs->push_back(mbox->GetMessageUID(*iter));

		// Save all - this call will delete the message list - we do not do that
		CActionManager::SaveMessages(msgs);
	}
	else
	{
		// One save per message
		for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
		{
			// Get message to print
			CMessage* msg = mbox->GetMessageUID(*iter);
			
			// Save it
			CActionManager::SaveMessage(msg);
		}
	}
}

void CActionItem::ExecuteSound(CMbox* mbox, const ulvector& uids) const
{
	::PlayNamedSound(GetOSStringMapData()->GetData().GetData());
}

void CActionItem::ExecuteAlert(CMbox* mbox, const ulvector& uids) const
{
	CErrorHandler::PutNoteAlert(GetStringData()->GetData());
}

void CActionItem::ExecuteSpeech(CMbox* mbox, const ulvector& uids) const
{
#ifdef __use_speech
	CSpeechSynthesis::SpeakString(GetStringData()->GetData());
#endif
}

// Check that action is valid
bool CActionItem::CheckAction() const
{
	// Only test mailboxes in copy/move actions
	if ((mType == eCopyMessage) || (mType == eMoveMessage))
	{
		cdstring copy_to = GetStringData()->GetData();

		// Create a mailbox reference which we will resolve
		CMboxRef ref(copy_to, '.');

		// Try to resolve mailbox - force it to be listed if not already
		CMbox* mbox_to = ref.ResolveMbox(true);
		return (mbox_to != NULL);
	}
	else
		return true;
}

// Rename account
bool CActionItem::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	if ((mType == eCopyMessage) || (mType == eMoveMessage))
	{
		cdstring old_copy = GetStringData()->GetData();
		if (!::strncmp(old_copy, old_acct, old_acct.length()) &&
			(old_copy.c_str()[old_acct.length()] == cMailAccountSeparator))
		{
			cdstring new_copy = new_acct;
			new_copy += &old_copy.c_str()[old_acct.length()];
			GetStringData()->GetData() = new_copy;
			return true;
		}
	}
	
	return false;
}

// Delete account
bool CActionItem::DeleteAccount(const cdstring& old_acct)
{
	if ((mType == eCopyMessage) || (mType == eMoveMessage))
	{
		cdstring old_copy = GetStringData()->GetData();
		if (!::strncmp(old_copy, old_acct, old_acct.length()) &&
			(old_copy.c_str()[old_acct.length()] == cMailAccountSeparator))
		{
			return true;
		}
	}
	
	return false;
}

// Identity change
bool CActionItem::RenameIdentity(const cdstring& old_id, const cdstring& new_id)
{
	switch(mType)
	{
	case eReplyMessage:
		if (GetReplyData()->GetData().GetIdentity() == old_id)
		{
			GetReplyData()->GetData().SetIdentity(new_id);
			return true;
		}
		break;
	case eForwardMessage:
		if (GetForwardData()->GetData().GetIdentity() == old_id)
		{
			GetForwardData()->GetData().SetIdentity(new_id);
			return true;
		}
		break;
	case eBounceMessage:
		if (GetBounceData()->GetData().GetIdentity() == old_id)
		{
			GetBounceData()->GetData().SetIdentity(new_id);
			return true;
		}
		break;
	case eRejectMessage:
		if (GetRejectData()->GetData().GetIdentity() == old_id)
		{
			GetRejectData()->GetData().SetIdentity(new_id);
			return true;
		}
		break;
	default:;
	}
	
	return false;
}

// Identity deleted
bool CActionItem::DeleteIdentity(const cdstring& old_id)
{
	switch(mType)
	{
	case eReplyMessage:
		if (GetReplyData()->GetData().GetIdentity() == old_id)
			return true;
		break;
	case eForwardMessage:
		if (GetForwardData()->GetData().GetIdentity() == old_id)
			return true;
		break;
	case eBounceMessage:
		if (GetBounceData()->GetData().GetIdentity() == old_id)
			return true;
		break;
	case eRejectMessage:
		if (GetRejectData()->GetData().GetIdentity() == old_id)
			return true;
		break;
	default:;
	}
	
	return false;
}

void CActionItem::GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const
{
	switch(mType)
	{
        case eReject:
        {
            ext = static_cast<CFilterProtocol::EExtension>(ext | CFilterProtocol::eReject);
            break;
        }
        case eFileInto:
        {
            ext = static_cast<CFilterProtocol::EExtension>(ext | CFilterProtocol::eFileInto);
            break;
        }
        case eSetFlag:
        {
            ext = static_cast<CFilterProtocol::EExtension>(ext | CFilterProtocol::eIMAP4Flags);
            break;
        }
        case eVacation:
        {
            ext = static_cast<CFilterProtocol::EExtension>(ext | CFilterProtocol::eVacation);
            break;
        }
        default:;
	}
}

void CActionItem::GenerateSIEVEScript(std::ostream& out) const
{
	switch(mType)
	{
	case eKeep:
		out << "keep;";
		break;
	case eDiscard:
		out << "discard;";
		break;
	case eReject:
	{
		out << "reject ";
		cdstring temp = GetStringData()->GetData();
		temp.quote(true);
		out << temp << ";";
		break;
	}
	case eRedirect:
	{
		out << "redirect ";
		cdstring temp = GetStringData()->GetData();
		temp.quote(true);
		out << temp << ";";
		break;
	}
    case eFileInto:
    {
        out << "fileinto ";
        cdstring temp = GetStringData()->GetData();
        temp.quote(true);
        out << temp << ";";
        break;
    }
    case eSetFlag:
    {
        out << "setflag ";
        NMessage::EFlags flags = GetFlagData()->GetData().GetFlags();
        cdstring temp;
        if (flags & NMessage::eAnswered)
        {
            temp = "\\Answered";
        }
        else if (flags & NMessage::eFlagged)
        {
            temp = "\\Flagged";
        }
        else if (flags & NMessage::eDeleted)
        {
            temp = "\\Deleted";
        }
        else if (flags & NMessage::eSeen)
        {
            temp = "\\Seen";
        }
        else if (flags & NMessage::eDraft)
        {
            temp =  "\\Draft";
        }
        else if (flags & NMessage::eMDNSent)
        {
            temp =  "$MDNSent";
        }
        else if (flags & NMessage::eLabels)
        {
            // Scan over all labels and add each
            for(int i = 0; i < NMessage::eMaxLabels; i++)
            {
                if (flags & (NMessage::eLabel1 << i))
                {
                    temp = CPreferences::sPrefs->mIMAPLabels.GetValue()[i];
                    break;
                }
            }
        }
        temp.quote(true);
        out << temp << ";";
        break;
    }
	case eVacation:
	{
		out << "vacation";
		const CActionVacation& vacation = GetVacationData()->GetData();
		out << " :days " << vacation.GetDays();
		if (vacation.GetAddresses().size())
		{
			out << " :addresses [";
			for(cdstrvect::const_iterator iter = vacation.GetAddresses().begin(); iter != vacation.GetAddresses().end(); iter++)
			{
				cdstring temp = *iter;
				temp.quote(true);
				if (iter != vacation.GetAddresses().begin())
					out << ", ";
				out << temp;
			}
			out << "]";
		}
		if (!vacation.GetSubject().empty())
		{
			cdstring temp = vacation.GetSubject();
			temp.quote(true);
			out << " :subject " << temp;
		}
		cdstring temp = vacation.GetText();
		temp.quote(true);
		out << " " << temp << ";";
		break;
	}
	default:;
	}
}	

// Get text expansion for prefs
cdstring CActionItem::GetInfo() const
{
	cdstring info;
	cdstring temp = cPrefsDescriptors[mType];
	temp.quote();
	info += temp;
	
	if (mData)
	{
		info += cSpace;
		info += mData->GetInfo();
	}
	
	return info;
}

// Convert text to items
bool CActionItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	char* p = txt.get();
	mType = static_cast<EActionItem>(::strindexfind(p, cPrefsDescriptors, eNone));
    
	switch(mType)
	{
        case eNone:
        case eExpunge:
        case ePrint:
        case eKeep:
        case eDiscard:
        default:
            mData = NULL;
            break;
        case eFlagMessage:
        case eSetFlag:
            mData = new CDataItem<CActionFlags>;
            break;
        case eCopyMessage:
        case eMoveMessage:
        case eAlert:
        case eSpeak:
        case eReject:
        case eRedirect:
        case eFileInto:
            mData = new CDataItem<cdstring>;
            break;
        case eReplyMessage:
            mData = new CDataItem<CActionReply>;
            break;
        case eForwardMessage:
            mData = new CDataItem<CActionForward>;
            break;
        case eBounceMessage:
            mData = new CDataItem<CActionBounce>;
            break;
        case eRejectMessage:
            mData = new CDataItem<CActionReject>;
            break;
        case eSave:
            mData = new CDataItem<bool>;
            break;
        case eSound:
            mData = new CDataItem<COSStringMap>;
            break;
        case eVacation:
            mData = new CDataItem<CActionVacation>;
            break;
	}
    
	if (mData)
		mData->SetInfo(txt, vers_prefs);
    
	return true;
}
