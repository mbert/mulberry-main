/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for Envelope class

#ifndef __CENVELOPE__MULBERRY__
#define __CENVELOPE__MULBERRY__

#include "cdstring.h"

#include <time.h>

#define		DATE_DELIM		" \t,-:"

class CAddressList;

class CEnvelope
{
public:
	
	class CEnvelopeIndex
	{
	public:
		CEnvelopeIndex();
		CEnvelopeIndex(const CEnvelopeIndex& copy);
		~CEnvelopeIndex() {}
		
		unsigned long GetDateIndex() const
			{ return mDateStart; }
		void SetDateIndex(unsigned long index)
			{ mDateStart = index; }

		unsigned long GetZoneIndex() const
			{ return mZoneStart; }
		void SetZoneIndex(unsigned long index)
			{ mZoneStart = index; }

		unsigned long GetSubjectIndex() const
			{ return mSubjectStart; }
		void SetSubjectIndex(unsigned long index)
			{ mSubjectStart = index; }
		unsigned long GetSubjectLength() const
			{ return mSubjectLength; }
		void SetSubjectLength(unsigned long length)
			{ mSubjectLength = length; }

		unsigned long GetFromIndex() const
			{ return mFromStart; }
		void SetFromIndex(unsigned long index)
			{ mFromStart = index; }
		unsigned long GetFromLength() const
			{ return mFromLength; }
		void SetFromLength(unsigned long length)
			{ mFromLength = length; }

		unsigned long GetSenderIndex() const
			{ return mSenderStart; }
		void SetSenderIndex(unsigned long index)
			{ mSenderStart = index; }
		unsigned long GetSenderLength() const
			{ return mSenderLength; }
		void SetSenderLength(unsigned long length)
			{ mSenderLength = length; }

		unsigned long GetReplyToIndex() const
			{ return mReplyToStart; }
		void SetReplyToIndex(unsigned long index)
			{ mReplyToStart = index; }
		unsigned long GetReplyToLength() const
			{ return mReplyToLength; }
		void SetReplyToLength(unsigned long length)
			{ mReplyToLength = length; }

		unsigned long GetToIndex() const
			{ return mToStart; }
		void SetToIndex(unsigned long index)
			{ mToStart = index; }
		unsigned long GetToLength() const
			{ return mToLength; }
		void SetToLength(unsigned long length)
			{ mToLength = length; }

		unsigned long GetCCIndex() const
			{ return mCCStart; }
		void SetCCIndex(unsigned long index)
			{ mCCStart = index; }
		unsigned long GetCCLength() const
			{ return mCCLength; }
		void SetCCLength(unsigned long length)
			{ mCCLength = length; }

		unsigned long GetBccIndex() const
			{ return mBccStart; }
		void SetBccIndex(unsigned long index)
			{ mBccStart = index; }
		unsigned long GetBccLength() const
			{ return mBccLength; }
		void SetBccLength(unsigned long length)
			{ mBccLength = length; }

		unsigned long GetInReplyToIndex() const
			{ return mInReplyToStart; }
		void SetInReplyToIndex(unsigned long index)
			{ mInReplyToStart = index; }
		unsigned long GetInReplyToLength() const
			{ return mInReplyToLength; }
		void SetInReplyToLength(unsigned long length)
			{ mInReplyToLength = length; }

		unsigned long GetMessageIDIndex() const
			{ return mMessageIDStart; }
		void SetMessageIDIndex(unsigned long index)
			{ mMessageIDStart = index; }
		unsigned long GetMessageIDLength() const
			{ return mMessageIDLength; }
		void SetMessageIDLength(unsigned long length)
			{ mMessageIDLength = length; }

		void WriteIndexToStream(std::ostream& out, long offset = 0) const;
		void ReadIndexFromStream(std::istream& in, unsigned long vers);
	
	protected:
		unsigned long	mDateStart;
		unsigned long	mZoneStart;
		unsigned long	mSubjectStart;
		unsigned long	mSubjectLength;
		unsigned long	mFromStart;
		unsigned long	mFromLength;
		unsigned long	mSenderStart;
		unsigned long	mSenderLength;
		unsigned long	mReplyToStart;
		unsigned long	mReplyToLength;
		unsigned long	mToStart;
		unsigned long	mToLength;
		unsigned long	mCCStart;
		unsigned long	mCCLength;
		unsigned long	mBccStart;
		unsigned long	mBccLength;
		unsigned long	mInReplyToStart;
		unsigned long	mInReplyToLength;
		unsigned long	mMessageIDStart;
		unsigned long	mMessageIDLength;
	};

			CEnvelope();
			CEnvelope(char* theDate,
						char* subject,
						CAddressList* from,
						CAddressList* sender,
						CAddressList* reply_to,
						CAddressList* to,
						CAddressList* cc,
						CAddressList* bcc,
						char* in_reply_to,
						char* message_id);
			CEnvelope(const CEnvelope &copy);					// Copy constructor
			~CEnvelope();

	bool RecoverCompare(const CEnvelope& comp) const;			// Do reconstruct recovery test

	void	SetDate(char* theDate);								// Set numeric date from text
	cdstring	GetTextDate(bool for_display, bool long_date = false) const;	// Get date as text
	time_t	GetDate() const { return mDate; }					// Get date
	time_t	GetUTCDate() const;									// Get date based on UTC

	void SetSubject(const cdstring& subject)
		{ mSubject = subject; } 
	cdstring& GetSubject()
		{ return mSubject; }									// Get subject
	const cdstring&	GetSubject() const
		{ return mSubject; }									// Get subject
	const char*	GetThread(bool& re_subject) const;				// Get subject as thread
	unsigned long GetThreadHash() const;						// Get subject as thread
	cdstring GetMatchSubject() const;							// Get normalised subject without collapsing whitespace
	void	FakeSubject()										// Create a fake subject
		{ mSubject = "\1"; }
	bool	HasFakeSubject() const								// Has a valid fake subject
		{ return mSubject[(cdstring::size_type)0] != '\1'; }

	void	SetFrom(CAddressList* addr_list)					// Set from address list
		{ SetAddressField(mFrom, addr_list); }
	CAddressList* GetFrom() const								// Get from address list
		{ return mFrom; }

	void	SetSender(CAddressList* addr_list)					// Set sender address list
		{ SetAddressField(mSender, addr_list); }
	CAddressList* GetSender() const								// Get sender address list
		{ return mSender; }

	void	SetReplyTo(CAddressList* addr_list)					// Set reply to address list
		{ SetAddressField(mReply_to, addr_list); }
	CAddressList* GetReplyTo() const							// Get reply to address list
		{ return mReply_to; }

	void	SetTo(CAddressList* addr_list)						// Set to address list
		{ SetAddressField(mTo, addr_list); }
	CAddressList* GetTo() const									// Get to address list
		{ return mTo; }

	void	SetCC(CAddressList* addr_list)						// Set CC address list
		{ SetAddressField(mCc, addr_list); }
	CAddressList* GetCC() const									// Get CC address list
		{ return mCc; }

	void	SetBcc(CAddressList* addr_list)						// Set Bcc address list
		{ SetAddressField(mBcc, addr_list); }
	CAddressList* GetBcc() const								// Get Bcc address list
		{ return mBcc; }

	cdstring& GetInReplyTo()									// Get in reply to
		{ return mIn_reply_to; }
	const cdstring&	GetInReplyTo() const						// Get in reply to
		{ return mIn_reply_to; }

	cdstring& GetMessageID()									// Get message id
		{ return mMessage_id; }
	const cdstring&	GetMessageID() const						// Get message id
		{ return mMessage_id; }

	bool	HasReferences() const								// HasReferences
		{ return mReferences[(cdstring::size_type)0] != '\1'; }
	void	SetReferences(const cdstring& refs)					// Set References
		{ mReferences = refs; }
	const cdstring& GetReferences() const						// Get References
		{ return mReferences; }

	void GetSummary(std::ostream& out) const;						// Get summary headers to stream

	void WriteCacheToStream(std::ostream& out, CEnvelopeIndex& index) const;
	void WriteAddressListToStream(std::ostream& out, const CAddressList* list) const;

	void ReadCacheFromStream(std::istream& in, unsigned long vers);
	CAddressList* ReadAddressListFromStream(std::istream& in, unsigned long vers) const;

protected:
	mutable cdstring	mTxt_date;				// Message date as text
	mutable cdstring	mTxt_ldate;				// Message long date as text
	time_t				mDate;					// Message date in std-c format
	long				mZone;					// Offset from UTC
	cdstring			mSubject;				// Subject
	CAddressList*		mFrom;					// From address list
	CAddressList*		mSender;				// Sender's address list
	CAddressList*		mReply_to;				// Reply-to address list
	CAddressList*		mTo;					// To address list
	CAddressList*		mCc;					// cc address list
	CAddressList*		mBcc;					// bcc address list
	cdstring			mIn_reply_to;			// in reply-to address
	cdstring			mMessage_id;			// Message id
	cdstring			mReferences;			// References

private:
	cdstring			mSubjectNorm;			// Canonicallised subject
	unsigned long		mSubjectHash;			// Hashed subject

	void	InitEnvelope();						// Common init
	
	void	SetAddressField(CAddressList*& old_field, CAddressList* new_field);

	void		GenerateThreadData();							// Get ordered subject data
	cdstring	NormaliseSubject(bool matching) const;			// Get normalised subject data
};

#endif
