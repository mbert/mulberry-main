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


// Header for CActionItem class

#ifndef __CACTIONITEM__MULBERRY__
#define __CACTIONITEM__MULBERRY__

#include "cdstring.h"
#include "CFilterProtocol.h"
#include "CMessageFwd.h"
#include "CDataItem.h"
#include "COSStringMap.h"
#include "prefsvector.h"
#include "templs.h"

// Classes
class CActionItem;
typedef prefsptrvector<CActionItem> CActionItemList;

class char_stream;
class CMbox;

class CActionItem
{
public:
	enum EActionItem
	{
		eNone = 0,

		// Message actions
		eFlagMessage,				// CActionFlags
		eCopyMessage,				// cdstring
		eMoveMessage,				// cdstring
		eReplyMessage,				// CActionReply
		eForwardMessage,			// CActionForward
		eBounceMessage,				// CActionBounce
		eRejectMessage,				// CActionReject
		eExpunge,					// NULL
		ePrint,						// NULL
		eSave,						// bool
		eSound,						// COSStringMap
		eAlert,						// cdstring
		eSpeak,						// cdstring

		// Sieve actions
		eKeep,						// NULL
		eDiscard,					// NULL
		eReject,					// cdstring
		eRedirect,					// cdstring
		eFileInto,					// cdstring
        eSetFlag,                   // CActionFlags
		eVacation,					// CActionVacation

		eActionLast
	};
	
	class CActionFlags
	{
	public:
		CActionFlags()
			{ mFlags = NMessage::eNone; mSet = true; }
		CActionFlags(NMessage::EFlags flags, bool set)
			{ mFlags = flags; mSet = set; }
		CActionFlags(const CActionFlags& copy)
			{ _copy(copy); }

		CActionFlags& operator=(const CActionFlags& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }
	
		NMessage::EFlags GetFlags() const
			{ return mFlags; }
		void SetFlags(NMessage::EFlags flags)
			{ mFlags = flags; }

		bool IsSet() const
			{ return mSet; }
		void Set(bool set)
			{ mSet = set; }

	private:
		NMessage::EFlags mFlags;
		bool			 mSet;
		
		void _copy(const CActionFlags& copy)
			{ mFlags = copy.mFlags; mSet = copy.mSet; }
	};

	class CActionReply
	{
	public:

		CActionReply()
			{ mCreateDraft = true; mType = NMessage::eReplyTo; mQuote = true; mTiedIdentity = true; }
		CActionReply(bool create, NMessage::EReplyType type, bool quote, const cdstring& text, const cdstring& identity, bool tied)
			{ mCreateDraft = create; mType = type; mQuote = quote; mText = text; mIdentity = identity; mTiedIdentity = tied; }
		CActionReply(const CActionReply& copy)
			{ _copy(copy); }
		~CActionReply() {}

		CActionReply& operator=(const CActionReply& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }

		bool CreateDraft() const
			{ return mCreateDraft; }
		void SetCreateDraft(bool create)
			{ mCreateDraft = create; }

		NMessage::EReplyType GetReplyType() const
			{ return mType; }
		void SetReplyType(NMessage::EReplyType type)
			{ mType = type; }

		bool Quote() const
			{ return mQuote; }
		void SetQuote(bool quote)
			{ mQuote = quote; }

		const cdstring& GetText() const
			{ return mText; }
		void SetText(const cdstring& text)
			{ mText = text; }

		const cdstring& GetIdentity() const
			{ return mIdentity; }
		void SetIdentity(const cdstring& identity)
			{ mIdentity = identity; }

		bool UseTiedIdentity() const
			{ return mTiedIdentity; }
		void SetTiedIdentity(bool tie)
			{ mTiedIdentity = tie; }

	private:
		bool mCreateDraft;
		NMessage::EReplyType mType;
		bool mQuote;
		cdstring mText;
		cdstring mIdentity;
		bool mTiedIdentity;
		
		void _copy(const CActionReply& copy)
			{ mCreateDraft = copy.mCreateDraft; mType = copy.mType; mQuote = copy.mQuote; mText = copy.mText;
				mIdentity = copy.mIdentity; mTiedIdentity = copy.mTiedIdentity; }
	};

	class CActionForward
	{
	public:
		CActionForward()
			{ mCreateDraft = true; mQuote = true; mAttach = false; mTiedIdentity = true; }
		CActionForward(bool create, const cdstring& to, const cdstring& cc, const cdstring& bcc,
						bool quote, bool attach, const cdstring& text, const cdstring& identity, bool tied)
			{ mCreateDraft = create; mAddrs.mTo = to; mAddrs.mCC = cc; mAddrs.mBcc = bcc;
				mQuote = quote; mAttach = attach; mText = text; mIdentity = identity; mTiedIdentity = tied; }
		CActionForward(const CActionForward& copy)
			{ _copy(copy); }
		~CActionForward() {}

		CActionForward& operator=(const CActionForward& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }

		bool CreateDraft() const
			{ return mCreateDraft; }
		void SetCreateDraft(bool create)
			{ mCreateDraft = create; }

		const NMessage::SAddressing& Addrs() const
			{ return mAddrs; }
		NMessage::SAddressing& Addrs()
			{ return mAddrs; }

		bool Quote() const
			{ return mQuote; }
		void SetQuote(bool quote)
			{ mQuote = quote; }

		bool Attach() const
			{ return mAttach; }
		void SetAttach(bool attach)
			{ mAttach = attach; }

		const cdstring& GetText() const
			{ return mText; }
		void SetText(const cdstring& text)
			{ mText = text; }

		const cdstring& GetIdentity() const
			{ return mIdentity; }
		void SetIdentity(const cdstring& identity)
			{ mIdentity = identity; }

		bool UseTiedIdentity() const
			{ return mTiedIdentity; }
		void SetTiedIdentity(bool tie)
			{ mTiedIdentity = tie; }

	private:
		bool mCreateDraft;
		NMessage::SAddressing mAddrs;
		bool mQuote;
		bool mAttach;
		cdstring mText;
		cdstring mIdentity;
		bool mTiedIdentity;
		
		void _copy(const CActionForward& copy)
			{ mCreateDraft = copy.mCreateDraft; mAddrs = copy.mAddrs; mQuote = copy.mQuote; mAttach = copy.mAttach;
				mText = copy.mText; mIdentity = copy.mIdentity; mTiedIdentity = copy.mTiedIdentity; }
	};

	class CActionBounce
	{
	public:
		CActionBounce()
			{ mCreateDraft = true; mTiedIdentity = true; }
		CActionBounce(bool create, const cdstring& to, const cdstring& cc, const cdstring& bcc, const cdstring& identity, bool tied)
			{ mCreateDraft = create; mAddrs.mTo = to; mAddrs.mCC = cc; mAddrs.mBcc = bcc; mIdentity = identity; mTiedIdentity = tied; }
		CActionBounce(const CActionBounce& copy)
			{ _copy(copy); }
		~CActionBounce() {}

		CActionBounce& operator=(const CActionBounce& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }

		bool CreateDraft() const
			{ return mCreateDraft; }
		void SetCreateDraft(bool create)
			{ mCreateDraft = create; }

		const NMessage::SAddressing& Addrs() const
			{ return mAddrs; }
		NMessage::SAddressing& Addrs()
			{ return mAddrs; }

		const cdstring& GetIdentity() const
			{ return mIdentity; }
		void SetIdentity(const cdstring& identity)
			{ mIdentity = identity; }

		bool UseTiedIdentity() const
			{ return mTiedIdentity; }
		void SetTiedIdentity(bool tie)
			{ mTiedIdentity = tie; }

	private:
		bool mCreateDraft;
		NMessage::SAddressing mAddrs;
		cdstring mIdentity;
		bool mTiedIdentity;
		
		void _copy(const CActionBounce& copy)
			{ mCreateDraft = copy.mCreateDraft; mAddrs = copy.mAddrs; mIdentity = copy.mIdentity; mTiedIdentity = copy.mTiedIdentity; }
	};

	class CActionReject
	{
	public:

		CActionReject()
			{ mCreateDraft = true; mReturnMessage = false; mTiedIdentity = true; }
		CActionReject(bool create, bool return_msg, const cdstring& identity, bool tied)
			{ mCreateDraft = create; mReturnMessage = return_msg; mIdentity = identity; mTiedIdentity = tied; }
		CActionReject(const CActionReject& copy)
			{ _copy(copy); }
		~CActionReject() {}

		CActionReject& operator=(const CActionReject& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }

		bool CreateDraft() const
			{ return mCreateDraft; }
		void SetCreateDraft(bool create)
			{ mCreateDraft = create; }

		bool ReturnMessage() const
			{ return mReturnMessage; }
		void SetReturnMessage(bool return_msg)
			{ mReturnMessage = return_msg; }

		const cdstring& GetIdentity() const
			{ return mIdentity; }
		void SetIdentity(const cdstring& identity)
			{ mIdentity = identity; }

		bool UseTiedIdentity() const
			{ return mTiedIdentity; }
		void SetTiedIdentity(bool tie)
			{ mTiedIdentity = tie; }

	private:
		bool mCreateDraft;
		bool mReturnMessage;
		cdstring mIdentity;
		bool mTiedIdentity;
		
		void _copy(const CActionReject& copy)
			{ mCreateDraft = copy.mCreateDraft; mReturnMessage = copy.mReturnMessage;
				mIdentity = copy.mIdentity; mTiedIdentity = copy.mTiedIdentity; }
	};

	class CActionVacation
	{
	public:

		CActionVacation()
			{ mDays = 7; }
		CActionVacation(unsigned long days, const cdstring& subject, const cdstring& text, const cdstrvect& addrs)
			{ mDays = days; mSubject = subject; mText = text; mAddresses = addrs; }
		CActionVacation(const CActionVacation& copy)
			{ _copy(copy); }
		~CActionVacation() {}

		CActionVacation& operator=(const CActionVacation& copy)							// Assignment with same type
			{ if (this != &copy) { _copy(copy); } return *this; }

		unsigned long GetDays() const
			{ return mDays; }
		void SetDays(unsigned long days)
			{ mDays = days; }

		const cdstring& GetSubject() const
			{ return mSubject; }
		void SetSubject(const cdstring& subject)
			{ mSubject = subject; }

		const cdstring& GetText() const
			{ return mText; }
		void SetText(const cdstring& text)
			{ mText = text; }

		const cdstrvect& GetAddresses() const
			{ return mAddresses; }
		void SetAddresses(const cdstrvect& addrs)
			{ mAddresses = addrs; }

	private:
		unsigned long	mDays;
		cdstring		mSubject;
		cdstring		mText;
		cdstrvect		mAddresses;
		
		void _copy(const CActionVacation& copy)
			{ mDays = copy.mDays; mSubject = copy.mSubject;
				mText = copy.mText; mAddresses = copy.mAddresses; }
	};

	CActionItem()
		{ mType = eNone; mData = NULL; }
	CActionItem(EActionItem action)
		{ mType = action; mData = NULL; }
	CActionItem(EActionItem action, NMessage::EFlags flags, bool set)
		{ mType = action; mData = new CDataItem<CActionFlags>(CActionFlags(flags, set)); }
	CActionItem(EActionItem action, const CDataItem<CActionReply>& data)
		{ mType = action; mData = new CDataItem<CActionReply>(data); }
	CActionItem(EActionItem action, const CDataItem<CActionForward>& data)
		{ mType = action; mData = new CDataItem<CActionForward>(data); }
	CActionItem(EActionItem action, const CDataItem<CActionBounce>& data)
		{ mType = action; mData = new CDataItem<CActionBounce>(data); }
	CActionItem(EActionItem action, const CDataItem<CActionReject>& data)
		{ mType = action; mData = new CDataItem<CActionReject>(data); }
	CActionItem(EActionItem action, const CDataItem<CActionVacation>& data)
		{ mType = action; mData = new CDataItem<CActionVacation>(data); }
	CActionItem(EActionItem action, const cdstring& data)
		{ mType = action; mData = new CDataItem<cdstring>(data); }
	CActionItem(EActionItem action, bool data)
		{ mType = action; mData = new CDataItem<bool>(data); }
	CActionItem(EActionItem action, const COSStringMap& data)
		{ mType = action; mData = new CDataItem<COSStringMap>(data); }
	CActionItem(const CActionItem& copy)
		{ _copy(copy); }
	~CActionItem()
		{ _tidy(); }

	CActionItem& operator=(const CActionItem& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	int operator==(const CActionItem& comp) const			// Compare with same type
		{ return (this == &comp); }

	void clear()															// Clear out existing data
		{ _tidy(); }

	void Execute(CMbox* mbox, const ulvector& uids) const;

	bool CheckAction() const;

	EActionItem	GetActionItem() const
		{ return mType; }
	void SetActionItem(EActionItem action)
		{ mType = action; }

	const CDataItemBase* GetData() const												// Get data
		{ return mData; }
	const CDataItem<cdstring>* GetStringData() const									// Get data
		{ return static_cast<CDataItem<cdstring>*>(mData); }
	CDataItem<cdstring>* GetStringData() 												// Get data
		{ return static_cast<CDataItem<cdstring>*>(mData); }
	const CDataItem<bool>* GetBoolData() const											// Get data
		{ return static_cast<CDataItem<bool>*>(mData); }
	CDataItem<bool>* GetBoolData()		 												// Get data
		{ return static_cast<CDataItem<bool>*>(mData); }
	const CDataItem<CActionFlags>* GetFlagData() const									// Get data
		{ return static_cast<CDataItem<CActionFlags>*>(mData); }
	const CDataItem<CActionReply>* GetReplyData() const									// Get data
		{ return static_cast<CDataItem<CActionReply>*>(mData); }
	CDataItem<CActionReply>* GetReplyData()												// Get data
		{ return static_cast<CDataItem<CActionReply>*>(mData); }
	const CDataItem<CActionForward>* GetForwardData() const								// Get data
		{ return static_cast<CDataItem<CActionForward>*>(mData); }
	CDataItem<CActionForward>* GetForwardData()											// Get data
		{ return static_cast<CDataItem<CActionForward>*>(mData); }
	const CDataItem<CActionBounce>* GetBounceData() const								// Get data
		{ return static_cast<CDataItem<CActionBounce>*>(mData); }
	CDataItem<CActionBounce>* GetBounceData()											// Get data
		{ return static_cast<CDataItem<CActionBounce>*>(mData); }
	const CDataItem<CActionReject>* GetRejectData() const								// Get data
		{ return static_cast<CDataItem<CActionReject>*>(mData); }
	CDataItem<CActionReject>* GetRejectData()											// Get data
		{ return static_cast<CDataItem<CActionReject>*>(mData); }
	CDataItem<CActionVacation>* GetVacationData() const									// Get data
		{ return static_cast<CDataItem<CActionVacation>*>(mData); }
	const CDataItem<COSStringMap>* GetOSStringMapData() const							// Get data
		{ return static_cast<CDataItem<COSStringMap>*>(mData); }

	// Account management
	bool	RenameAccount(const cdstring& old_acct, const cdstring& new_acct);	// Rename account
	bool	DeleteAccount(const cdstring& old_acct);							// Delete account

	// Identity management
	bool	RenameIdentity(const cdstring& old_id, const cdstring& new_id);				// Identity change
	bool	DeleteIdentity(const cdstring& old_id);										// Identity deleted

	// SIEVE items
	void	GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const;
	void	GenerateSIEVEScript(std::ostream& out) const;

	// Read/write prefs
	cdstring GetInfo(void) const;														// Get text expansion for prefs
	bool SetInfo(char_stream& info, NumVersion vers_prefs);								// Convert text to items

protected:
	EActionItem		mType;
	CDataItemBase*	mData;

	void ExecuteFlags(CMbox* mbox, const ulvector& uids) const;
	void ExecuteCopyMove(CMbox* mbox, const ulvector& uids, bool move) const;
	void ExecuteReply(CMbox* mbox, const ulvector& uids) const;
	void ExecuteForward(CMbox* mbox, const ulvector& uids) const;
	void ExecuteBounce(CMbox* mbox, const ulvector& uids) const;
	void ExecuteReject(CMbox* mbox, const ulvector& uids) const;
	void ExecuteExpunge(CMbox* mbox, const ulvector& uids) const;
	void ExecutePrint(CMbox* mbox, const ulvector& uids) const;
	void ExecuteSave(CMbox* mbox, const ulvector& uids) const;
	void ExecuteSound(CMbox* mbox, const ulvector& uids) const;
	void ExecuteAlert(CMbox* mbox, const ulvector& uids) const;
	void ExecuteSpeech(CMbox* mbox, const ulvector& uids) const;

	void DescribeAction() const;

private:
	void _copy(const CActionItem& copy);
	void _tidy();
};

#endif
