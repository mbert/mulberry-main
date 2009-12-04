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


// CIdentity.h

// Class that encapsulates a 'mail identity' used when sending messages

#ifndef __CIDENTITY__MULBERRY__
#define __CIDENTITY__MULBERRY__

#include "cdstring.h"
#include "CDSN.h"
#include "CFutureItems.h"
#include "prefsvector.h"

class CAddress;
class CCalendarAddress;
class CMbox;
namespace calstore
{
class CCalendarStoreNode;
};

class CIdentity
{
public:
	enum ESignWith
	{
		eSignWithDefault,
		eSignWithFrom,
		eSignWithReplyTo,
		eSignWithSender,
		eSignWithOther
	};

	enum ETiedInfo
	{
		eMbox,
		eCal
	};

	typedef const cdstring& (CIdentity::*TInheritDataStr)(bool) const;
	typedef cdstring (CIdentity::*TInheritDataStrVal)(bool) const;
	typedef bool (CIdentity::*TInheritDataBool)(bool) const;
	typedef bool (CIdentity::*TInheritTest)() const;

	CIdentity();
	CIdentity(const CIdentity& copy)			// Copy construct
		{ _copy(copy); }

	~CIdentity() {}
		
	CIdentity& operator=(const CIdentity& copy)						// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }
	int operator==(const CIdentity& test) const;					// Compare with same type

	bool RenameAccount(const cdstring& old_acct, const cdstring& new_acct);

	// Serialize
	bool SetInfo(char_stream& info, NumVersion vers_prefs);					// Parse S-Expression element
	cdstring GetInfo() const;										// Create S_Expression element

	cdstring GetTiedInfo(ETiedInfo type) const;
	bool SetTiedInfo(ETiedInfo type, char_stream& info, NumVersion vers_prefs);
	
	// Getters & Setters
	void SetTemp()
	{
		mTemp = true;
	}
	bool GetTemp() const
	{
		return mTemp;
	}

	void	SetInherit(const cdstring& inherit)
		{ mInherit = inherit; }
	const cdstring& GetInherit() const
		{ return mInherit; }
	const CIdentity& GetInheritIdentity() const;

	const CIdentity* GetInherited(TInheritTest inheritfn) const;
	const cdstring& GetInheritedValue(TInheritDataStr datafn, TInheritTest testfn) const;
	cdstring GetInheritedValue(TInheritDataStrVal datafn, TInheritTest testfn) const;
	bool GetInheritedValue(TInheritDataBool datafn, TInheritTest testfn) const;

	void	SetIdentity(const cdstring& identity)
		{ mIdentity = identity; }
	const cdstring& GetIdentity() const
		{ return mIdentity; }
	
	void	SetSMTPAccount(const cdstring& acct, bool use_smtp)
		{ mSMTPAccount = std::make_pair(acct, use_smtp); }
	bool	UseSMTPAccount() const
		{ return mSMTPAccount.second; }
	const cdstring& GetSMTPAccount(bool resolve = false) const;
	
	void	SetFrom(const cdstring& from, bool use_from)
		{ mFrom = std::make_pair(from, use_from); }
	bool	UseFrom() const
		{ return mFrom.second; }
	const cdstring& GetFrom(bool resolve = false) const;
	
	void	SetReplyTo(const cdstring& reply_to, bool use_reply_to)
		{ mReplyTo = std::make_pair(reply_to, use_reply_to); }
	bool	UseReplyTo() const
		{ return mReplyTo.second; }
	const cdstring& GetReplyTo(bool resolve = false) const;
	
	void	SetSender(const cdstring& sender, bool use_sender)
		{ mSender = std::make_pair(sender, use_sender); }
	bool	UseSender() const
		{ return mSender.second; }
	const cdstring& GetSender(bool resolve = false) const;
	
	void	SetCalendar(const cdstring& calendar, bool use_calendar)
		{ mCalendar = std::make_pair(calendar, use_calendar); }
	bool	UseCalendar() const
		{ return mCalendar.second; }
	const cdstring& GetCalendar(bool resolve = false) const;
	cdstring GetCalendarAddress() const;
	
	void	SetCopyTo(const cdstring& copy_to, bool use_copy_to)
		{ mCopyTo = std::make_pair(copy_to, use_copy_to); }
	bool	UseCopyTo() const
		{ return mCopyTo.second; }
	const cdstring& GetCopyTo(bool resolve = false) const;
	
	void	SetCopyToNone(bool copy)
		{ mCopyToNone = copy; }
	bool GetCopyToNone(bool resolve = false) const;
	
	void	SetCopyToChoose(bool copy)
		{ mCopyToChoose = copy; }
	bool GetCopyToChoose(bool resolve = false) const;
	
	void	SetCopyReplied(bool copy)
		{ mCopyReplied = copy; }
	bool GetCopyReplied(bool resolve = false) const;
	
	void	SetHeader(const cdstring& header, bool use_header)
		{ mHeader = std::make_pair(header, use_header); }
	bool	UseHeader() const
		{ return mHeader.second; }
	const cdstring& GetHeader(bool resolve = false) const;
	
	void	SetSignature(const cdstring& signature, bool use_signature)
		{ mSignature = std::make_pair(signature, use_signature); }
	bool	UseSignature() const
		{ return mSignature.second; }
	cdstring GetSignature(bool resolve = false) const;
	cdstring GetSignatureRaw(bool resolve = false) const;
		
	void	SetAddTo(const cdstring& add_to, bool use_add_to)
		{ mAddTo = std::make_pair(add_to, use_add_to); }
	bool	UseAddTo() const
		{ return mAddTo.second; }
	const cdstring& GetAddTo(bool resolve = false) const;
	
	void	SetAddCC(const cdstring& add_cc, bool use_add_cc)
		{ mAddCC = std::make_pair(add_cc, use_add_cc); }
	bool	UseAddCC() const
		{ return mAddCC.second; }
	const cdstring& GetAddCC(bool resolve = false) const;
	
	void	SetAddBcc(const cdstring& add_bcc, bool use_add_bcc)
		{ mAddBcc = std::make_pair(add_bcc, use_add_bcc); }
	bool	UseAddBcc() const
		{ return mAddBcc.second; }
	const cdstring& GetAddBcc(bool resolve = false) const;
	
	void SetUseDSN(bool use_it)
		{ mUseDSN = use_it; }
	bool GetUseDSN() const
		{ return mUseDSN; }
	
	CDSN& SetDSN()
		{ return mDSN; }
	const CDSN& GetDSN(bool resolve = false) const;

	void SetUseSecurity(bool use_it)
		{ mUseSecurity = use_it; }
	bool GetUseSecurity() const
		{ return mUseSecurity; }
	
	void	SetSign(bool sign)
		{ mSign = sign; }
	bool GetSign(bool resolve = false) const;
	
	void	SetEncrypt(bool encrypt)
		{ mEncrypt = encrypt; }
	bool GetEncrypt(bool resolve = false) const;
	
	void	SetSignWith(ESignWith sign_with, const cdstring& other = cdstring::null_str)
		{ mSignWith = sign_with; mSignOther = other; }
	ESignWith GetSignWith(bool resolve = false) const;
	const cdstring& GetSignOther() const
		{ return mSignOther; }
	cdstring GetSigningID(bool resolve = false) const;
	
	// Tied mailboxes
	void AddTiedMailbox(const CMbox* mbox);
	void RemoveTiedMailbox(const CMbox* mbox);
	bool HasTiedMailbox(const CMbox* mbox) const;
	bool HasTiedMailbox(const cdstring& mboxname) const;

	// Tied calendars
	void AddTiedCalendar(const calstore::CCalendarStoreNode* node);
	void RemoveTiedCalendar(const calstore::CCalendarStoreNode* node);
	bool HasTiedCalendar(const calstore::CCalendarStoreNode* node) const;
	bool HasTiedCalendar(const cdstring& calname) const;

private:
	bool		mTemp;
	cdstring	mInherit;
	cdstring	mIdentity;
	cdstrbool	mSMTPAccount;
	cdstrbool	mFrom;
	cdstrbool	mReplyTo;
	cdstrbool	mSender;
	cdstrbool	mCalendar;
	cdstrbool	mCopyTo;
	bool		mCopyToNone;
	bool		mCopyToChoose;
	bool		mCopyReplied;
	cdstrbool	mHeader;
	cdstrbool	mSignature;
	bool		mSignatureFile;
	bool		mSignatureApp;
	
	cdstrbool	mAddTo;
	cdstrbool	mAddCC;
	cdstrbool	mAddBcc;
	
	bool		mUseDSN;
	CDSN		mDSN;

	bool		mUseSecurity;
	bool		mSign;
	bool		mEncrypt;
	ESignWith	mSignWith;
	cdstring	mSignOther;

	CFutureItems	mFuture;
	
	cdstrset	mTiedMailboxes;
	cdstrset	mTiedCalendars;

	void		_copy(const CIdentity& copy);
	void		AddPair(cdstring& all, const cdstrbool& item) const;			// Add a pair as text
	void		MakePair(char_stream& txt, cdstrbool& item);					// Make pair from text
	
	void		CheckSignatureType();
	
	const cdstrset&	GetTied(ETiedInfo type) const;
	cdstrset&	GetTied(ETiedInfo type);
};

class CIdentityList : public prefsvector<CIdentity>
{
public:
	CIdentityList() {}
	CIdentityList(const CIdentityList& copy) :
		prefsvector<CIdentity>(copy) {}
	virtual ~CIdentityList() {}

    CIdentityList& operator= (const CIdentityList& copy)
    	{ return static_cast<CIdentityList&>(prefsvector<CIdentity>::operator=(copy)); }
    bool operator== (const CIdentityList& other) const
    	{ return prefsvector<CIdentity>::operator==(other); }

	bool RenameAccount(const cdstring& old_acct, const cdstring& new_acct);

	CIdentity* GetIdentity(const cdstring& id) const;
	CIdentity* GetIdentity(const CAddress& addr) const;
	CIdentity* GetIdentity(const CCalendarAddress& addr) const;
	unsigned long GetIndex(const cdstring& id) const;

	const CIdentity* GetTiedMboxIdentity(const CMbox* mbox) const;
	const CIdentity* GetTiedMboxIdentity(const cdstring& mboxname) const;

	const CIdentity* GetTiedCalIdentity(const calstore::CCalendarStoreNode* node) const;
	const CIdentity* GetTiedCalIdentity(const cdstring& calname) const;

	cdstring GetTiedInfo(CIdentity::ETiedInfo type) const;
	bool SetTiedInfo(CIdentity::ETiedInfo type, char_stream& info, NumVersion vers_prefs);
};

class CTiedIdentityList
{
public:
	CTiedIdentityList()
		{ mList = NULL; }
	CTiedIdentityList(const CTiedIdentityList& copy)
		{ mList = copy.mList; mType = copy.mType; }
	~CTiedIdentityList() { mList = NULL; }

    CTiedIdentityList& operator= (const CTiedIdentityList& copy)
    	{ mList = copy.mList; mType = copy.mType; return *this; }
    bool operator== (const CTiedIdentityList& other) const
    	{ return *mList == *other.mList; }

	void SetIdentityList(CIdentityList* list)
		{ mList = list; }

	void SetType(CIdentity::ETiedInfo type)
	{
		mType = type;
	}

	const CIdentity* GetTiedMboxIdentity(const CMbox* mbox) const
		{ return mList->GetTiedMboxIdentity(mbox); }
	const CIdentity* GetTiedMboxIdentity(const cdstring& mboxname) const
		{ return mList->GetTiedMboxIdentity(mboxname); }

	const CIdentity* GetTiedCalIdentity(const calstore::CCalendarStoreNode* node) const
		{ return mList->GetTiedCalIdentity(node); }
	const CIdentity* GetTiedCalIdentity(const cdstring& calname) const
		{ return mList->GetTiedCalIdentity(calname); }

	cdstring GetInfo() const
		{ return mList->GetTiedInfo(mType); }
	bool SetInfo(char_stream& info, NumVersion vers_prefs)
		{ return mList->SetTiedInfo(mType, info, vers_prefs); }

private:
	CIdentityList* 			mList;
	CIdentity::ETiedInfo 	mType;
};

#endif
