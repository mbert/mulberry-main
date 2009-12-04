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


// Header for Message class

#ifndef __CMESSAGE__MULBERRY__
#define __CMESSAGE__MULBERRY__

#include "SBitFlags.h"
#include "CAttachment.h"
#include "CEnvelope.h"
#include "CMessageFwd.h"
#include "CStreamTypeFwd.h"

#include <time.h>

//typedef bool (CMessage::*MessageFlagTest)() const;

// Constants

class CAttachment;
class CDSN;
class CMbox;
class CMessageList;
class CIdentity;
class CProgress;
class CStreamAttachment;

class CMessageCryptoInfo
{
public:
	CMessageCryptoInfo()
		{ mSuccess = false; mDidSignature = false; mSignatureOK = false; mDidDecrypt = false; mBadPassphrase = false; }
	CMessageCryptoInfo(const CMessageCryptoInfo& copy)
		{ mSuccess = copy.mSuccess; mDidSignature = copy.mDidSignature; mSignatureOK = copy.mSignatureOK; mDidDecrypt = copy.mDidDecrypt; mBadPassphrase = copy.mBadPassphrase;
		  mSignedBy = copy.mSignedBy; mEncryptedTo = copy.mEncryptedTo; mError = copy.mError; }
	~CMessageCryptoInfo() {}
	
	bool GetSuccess() const
		{ return mSuccess; }
	void SetSuccess(bool success)
		{ mSuccess = success; }

	bool GetDidSignature() const
		{ return mDidSignature; }
	void SetDidSignature(bool didsig)
		{ mDidSignature = didsig; }

	bool GetSignatureOK() const
		{ return mSignatureOK; }
	void SetSignatureOK(bool sigok)
		{ mSignatureOK = sigok; }

	bool GetDidDecrypt() const
		{ return mDidDecrypt; }
	void SetDidDecrypt(bool diddecrypt)
		{ mDidDecrypt = diddecrypt; }

	bool GetBadPassphrase() const
		{ return mBadPassphrase; }
	void SetBadPassphrase(bool badpass)
		{ mBadPassphrase = badpass; }

	const cdstrvect& GetSignedBy() const
		{ return mSignedBy; }
	cdstrvect& GetSignedBy()
		{ return mSignedBy; }

	const cdstrvect& GetEncryptedTo() const
		{ return mEncryptedTo; }
	cdstrvect& GetEncryptedTo()
		{ return mEncryptedTo; }

	const cdstring& GetError() const
		{ return mError; }
	void SetError(const cdstring& error)
		{ mError = error; }

private:
	bool		mSuccess;
	bool		mDidSignature;
	bool		mSignatureOK;
	bool		mDidDecrypt;
	bool		mBadPassphrase;
	
	cdstrvect	mSignedBy;
	cdstrvect	mEncryptedTo;
	
	cdstring	mError;
};

class CMessage
{
	// I N S T A N C E  V A R I A B L E S

protected:
	class CMessageCache
	{
		friend class CMessage;
		friend class CLocalMessage;

		// Used when cached
		CEnvelope*					mEnvelope;					// Envelope
		unsigned long				mSize;						// RFC822 Size
		cdstring					mTxt_date;					// Message date as text
		cdstring					mTxt_ldate;					// Message long date as text
		time_t						mInternalDate;				// Message internal date in std-c format
		long						mInternalZone;				// Offset from UTC
		char*						mHeader;					// RFC822 Header
		CAttachment*				mBody;						// Primary attachment
		CMessage*					mOwner;						// Owner if sub-message
		CMessageCryptoInfo*			mCryptoInfo;				// Info about verified/decrypted message

		CMessageCache();
		CMessageCache(const CMessageCache& copy);
		~CMessageCache();
	};

	class CThreadCache
	{
		friend class CMessage;
		
		unsigned long		mDepth;
		const CMessage*		mParent;
		const CMessage*		mChild;
		const CMessage*		mPrevious;
		const CMessage*		mNext;
		
		CThreadCache()
			{ mDepth = 0; mParent = mChild = mPrevious = mNext = NULL; }
		CThreadCache(const CThreadCache& copy)
			{ mDepth = copy.mDepth;
			  mParent = copy.mParent;
			  mChild = copy.mChild;
			  mPrevious = copy.mPrevious;
			  mNext = copy.mNext; }
		~CThreadCache() {}
	};

	// Data permanently stored for all messages
	CMbox*						mMbox;						// Ptr to owning mbox
	unsigned long				mNumber;					// Message number in mbox
	unsigned long				mUID;						// Message UID
	SBitFlags					mFlags;						// Message flags
	CMessageCache*				mCache;						// Cached data
	CThreadCache*				mThread;					// Cached thread data
	static bool					sAllowClear;				// Indicates if delete flag clears

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
	explicit CMessage(CMbox* owner);				// Constructed from owning mbox only
	explicit CMessage(CMessage* owner);				// Constructed from owning mbox only
	CMessage(const CMessage &copy);					// Copy constructor
	explicit CMessage(const CIdentity* identity,
				const CDSN* dsn,
				CAddressList* toText,
				CAddressList* ccList,
				CAddressList* bccList,
				const char* subjectText,
				CAttachment* body,
				const CMessageList* replies = NULL,
				const char* bounced = NULL);		// Create RFC822 bounced message from fields
	virtual ~CMessage();

	// O T H E R  M E T H O D S
private:
	void	InitMessage();

public:
	void	SetMessageNumber(unsigned long number)
				{ mNumber = number; }							// Set message number
	unsigned long	GetMessageNumber() const;					// Get position in mbox

	// Thread methods
	void ClearThreadInfo()
		{ if (mThread) { delete mThread; mThread = NULL; } }
	void SetThreadInfo(unsigned long depth, const CMessage* parent, const CMessage* child, const CMessage* prev, const CMessage* next);
	void SetThreadDepth(unsigned long depth) const
		{ if (mThread) mThread->mDepth = depth; }
	unsigned long GetThreadDepth() const
		{ return mThread ? mThread->mDepth : 0; }
	void ThreadPromote();
	void SetThreadParent(const CMessage* msg) const
		{ if (mThread) mThread->mParent = msg; }
	const CMessage* GetThreadParent() const
		{ return mThread ? mThread->mParent : NULL; }
	void SetThreadChild(const CMessage* msg) const
		{ if (mThread) mThread->mChild = msg; }
	const CMessage* GetThreadChild() const
		{ return mThread ? mThread->mChild : NULL; }
	void SetThreadPrevious(const CMessage* msg) const
		{ if (mThread) mThread->mPrevious = msg; }
	const CMessage* GetThreadPrevious() const
		{ return mThread ? mThread->mPrevious : NULL; }
	void SetThreadNext(const CMessage* msg) const
		{ if (mThread) mThread->mNext = msg; }
	const CMessage* GetThreadNext() const
		{ return mThread ? mThread->mNext : NULL; }
	void MakeFake();
	void MakeFakeSubject();
	const CMessage* FindChildMsg() const;

	// Cache state methods
	bool CacheMessage();										// Create the cache
	void ValidateCache();										// Make sure cached data is consistent otherwise uncache
	void UncacheMessage();										// Remove the cached data
	bool IsCached()	const										// Is it cached data
		{ return mCache; }
	bool IsFullyCached() const									// Is it fully cached data
		{ return (mCache != NULL) &&
					(mCache->mEnvelope != NULL) &&
					(mCache->mBody != NULL); }
	void ReadCache();											// Read in message cache info from server

	cdstring GetURL(bool full = false) const;					// Get message URL

	void SetUID(unsigned long uid, bool dont_sync = false);		// Set UID
	void RemapUID(unsigned local_uid, unsigned long uid);		// Change UID based on local UID
	unsigned long	GetUID() const;								// Get UID

	// Flag methods
	bool SetFlags(SBitFlags& new_flags);						// Set flags
	void ChangeFlags(NMessage::EFlags new_flags, bool set);		// Change flags
	const SBitFlags& GetFlags() const							// Get flags
		{ return mFlags; }
	SBitFlags& GetFlags()										// Get flags
		{ return mFlags; }
	void ReadFlags();											// Read in message flags from server

	bool IsRecent() const;										// Check seen flag

	bool IsCheckRecent() const;									// Check new recent pseudo flag
	void ClearCheckRecent()
		{ GetFlags().Set(NMessage::eCheckRecent, false); }

	bool IsMboxRecent() const;									// Check new recent pseudo flag

	bool HasFlag(NMessage::EFlags flag) const;					// Check for flag
	bool IsUnseen() const;										// Check seen flag
	bool IsDeleted() const;										// Check deleted flag
	bool IsAnswered() const;									// Check answered flag
	bool IsFlagged() const;										// Check flagged flag
	bool IsDraft() const;										// Check draft flag
	bool IsMDNSent() const;										// Check MDNSent flag
	bool IsPartial() const;										// Check for partial message
	bool IsError() const;										// Check for error message
	bool HasLabel(unsigned long index) const;					// Check for label
	bool IsFake() const;										// Check for faked message

	bool IsSendNow() const										// Check for send now message
		{ return IsAnswered(); }
	bool IsHold() const											// Check for hold message
		{ return IsMDNSent(); }
	bool IsPriority() const										// Check for priority message
		{ return IsFlagged(); }
	bool IsSendError() const									// Check for send error message
		{ return IsError(); }

	bool IsSearch() const;										// Check search pseudo flag
	void ClearSearch()
		{ GetFlags().Set(NMessage::eSearch, false); }

	bool IsFullLocal() const;									// Check full local pseudo flag
	bool IsPartialLocal() const;								// Check partial local pseudo flag

	bool IsSmartFrom() const;									// Check for smart From address
	bool IsSmartTo() const;										// Check for smart To address
	bool IsSmartCC() const;										// Check for smart CC address
	bool IsSmartList() const;									// Check for smart list address
	void ResetSmart();											// Smart addresses reset

	
protected:
	void TestSmartAddress();									// Do smart address checks

public:
	void SetMbox(CMbox* anMbox)									// Set new owner
		{mMbox = anMbox;}
	const CMbox* GetMbox() const								// Return owner
		{ return mMbox; }
	CMbox* GetMbox()											// Return owner
		{ return mMbox; }

	void SetOwner(CMessage* owner)									// Set new owner message
		{ if (mCache) mCache->mOwner = owner; }
	const CMessage* GetOwner() const								// Return owner message
		{ return mCache ? mCache->mOwner : NULL; }
	CMessage* GetOwner()											// Return owner message
		{ return mCache ? mCache->mOwner : NULL; }
	bool	IsSubMessage() const									// Is it a sub-message
		{ return mCache ? (mCache->mOwner != NULL) : false; }

	void	SetEnvelope(CEnvelope* env)								// Set envelope
		{ if (mCache) { delete mCache->mEnvelope; mCache->mEnvelope = env; } }
	const CEnvelope*	GetEnvelope() const						// Return envelope
		{ return mCache ? mCache->mEnvelope : NULL; }
	CEnvelope*	GetEnvelope()										// Return envelope
		{ return mCache ? mCache->mEnvelope : NULL; }

	void SetSize(unsigned long size)							// Set size of message
		{ if (mCache) mCache->mSize = size; }
	unsigned long GetSize() const								// Return size of message
		{ return mCache ? mCache->mSize : 0; }
	void ReadSize();													// Read in size of message
	unsigned long GetPartSize(const CAttachment* attach = NULL) const;	// Get size of a part
	unsigned long CheckSizeWarning(bool all = false) const;				// Check if greater than size warning

	bool	IsMultipart() const;								// Determine if a multipart message

	void	SetInternalDate(char* theDate, bool unixd = false);	// Set numeric internal date from text
	cdstring	GetTextInternalDate(bool for_display,					// Get internal date as text
										bool long_date = false) const;
	time_t	GetInternalDate() const											// Get internal date
		{ return mCache ? mCache->mInternalDate : 0; }
	time_t	GetUTCInternalDate() const;								// Get internal date based on UTC
	long	GetInternalZone() const									// Get internal zone
		{ return mCache ? mCache->mInternalZone : 0; }

	void	SetHeader(char* txt)									// Set pointer to header
		{ if (mCache) {delete mCache->mHeader; mCache->mHeader = txt;} }
	char*	GetHeader();											// Return pointer to header
	bool	HasHeader() const										// Check for cached header
		{return mCache ? (mCache->mHeader != NULL) : false;}
	void	ReadHeader();											// Read in the message header

	bool	GetHeaderField(const cdstring& field, cdstring& result);	// Fetch specific header field

	const char*	ReadPart(CAttachment* attach = NULL,
							bool peek = false);						// Read in a part
	void	ReadAttachment(CAttachment* attach,						// Read in an attachment
							bool peek = false,
							bool filter = true);
	void	GetRawBody(std::ostream& out, bool peek = false);		// Read in entire raw body
	bool	HasText() const;									// Are there any text parts?
	bool	HasData(const CAttachment* attach = NULL) const;	// Does the part already have data?
	CAttachment*	FirstDisplayPart()							// Find first part to display
		{ return (GetBody() ? GetBody()->FirstDisplayPart() : NULL); }

	unsigned long	CountParts() const							// Count number of parts (not multiparts)
		{ return (GetBody() ? GetBody()->CountParts() : 0); }
	bool	HasUniqueTextPart()									// Check number of unique text parts (account for alternatives)
		{ return (GetBody() ? GetBody()->HasUniqueTextPart() : 0); }
	bool	HasUniquePart()										// Check number of unique text parts (account for alternatives, sigs)
		{ return (GetBody() ? GetBody()->HasUniquePart() : 0); }

	void	SetBody(CAttachment* body, bool delete_it = true);	// Set a new body
	void	ReplaceBody(CStreamAttachment* body);				// Set a new body and adjust headers
	const CAttachment* GetBody() const							// Get the body
		{ return mCache ? mCache->mBody : NULL; }
	CAttachment* GetBody()										// Get the body
		{ return mCache ? mCache->mBody : NULL; }

	void	ExtractPart(CAttachment* part, bool view);			// Extract part to disk

	char* 	GetRFC822CRLF() const;								// Return pointer to complete RFC822 message with CRLFs
	virtual void	WriteHeaderToStream(costream& stream);		// Write message header to output stream
	virtual void	WriteToStream(costream& stream,
							bool dummy_files,
							CProgress* progress,
							bool always_mbox_copy = false);		// Write message to output stream

	unsigned long	ClearMessage();								// Clear cached info
	
	unsigned long CountDigest() const;							// Get number of messages in digest
	unsigned long GetDigestNumber() const;						// Get position in digest
	CMessage* GetPrevDigest() const;							// Get previous message in digest
	CMessage* GetNextDigest() const;							// Get next message in digest
	
	// MDN methods
	bool			MDNRequested();
	CAttachment*	CreateMDNSeenBody(const CIdentity* id,		// Generate the MDN message body
										bool automatic);
	void			GetMDNData(cdstring& mdn_to,				// Get MDN address details
								cdstring& orcpt);

	// Reject methods
	bool			CanReject();								// Can reject this message?
	CAttachment*	CreateRejectDSNBody(bool return_msg);		// Generate the reject message body
	void			GetDeliveryData(cdstring& return_path,		// Get delivery details
									cdstring& received_from,
									cdstring& received_by,
									cdstring& received_for,
									cdstring& received_date);
	// Crypto
	const CMessageCryptoInfo* GetCryptoInfo() const
		{ return mCache ? mCache->mCryptoInfo : NULL; }
	CMessageCryptoInfo* GetCryptoInfo()
		{ return mCache ? mCache->mCryptoInfo : NULL; }
	void MakeCryptoInfo()
		{ if (mCache && (mCache->mCryptoInfo == NULL)) mCache->mCryptoInfo = new CMessageCryptoInfo(); }
};

#endif
