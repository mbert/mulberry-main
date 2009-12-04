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


// Header for CMessageComparator class

#ifndef __CMESSAGECOMPARATORS__MULBERRY__
#define __CMESSAGECOMPARATORS__MULBERRY__

#include "CMessageList.h"

class CMessage;

typedef bool (*MessageCompare)(CMessage*, CMessage*);
typedef bool (*MessagePartition)(CMessage* inMsg);

class CMessageComparator
{
public:
	CMessageComparator(bool ascending = true);
	virtual ~CMessageComparator();

			void	SetAscending(bool ascending)
						{ mOrder = ascending ? 1 : -1; }
			bool	GetAscending()
						{ return mOrder == 1;}

	virtual MessageCompare	GetComparator() = 0;

	virtual void SortMessageList(CMessageList* list);
	virtual CMessageList::iterator LowerBound(CMessageList* list, const CMessage* aMsg);

	static bool TestCachedState(CMessage* inMsg1, CMessage* inMsg2, bool& result);

protected:
	long		mOrder;
	static long sOrder;
	static long sNumDiff;

};


class CNamedAddressComparator : public CMessageComparator
{
public:
	CNamedAddressComparator(bool ascending = true);
	virtual ~CNamedAddressComparator();

protected:
	static bool	CompareNames(CMessage* inMsg1,
								CMessage* inMsg2,
								char* name1,
								char* name2);			// Compare chosen names
};

class CMessageToComparator : public CNamedAddressComparator
{
public:
	CMessageToComparator(bool ascending = true);
	virtual ~CMessageToComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageFromComparator : public CNamedAddressComparator
{
public:
	CMessageFromComparator(bool ascending = true);
	virtual ~CMessageFromComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageReplyToComparator : public CNamedAddressComparator
{
public:
	CMessageReplyToComparator(bool ascending = true);
	virtual ~CMessageReplyToComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageSenderComparator : public CNamedAddressComparator
{
public:
	CMessageSenderComparator(bool ascending = true);
	virtual ~CMessageSenderComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageCcComparator : public CNamedAddressComparator
{
public:
	CMessageCcComparator(bool ascending = true);
	virtual ~CMessageCcComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageSmartComparator : public CNamedAddressComparator
{
public:
	CMessageSmartComparator(bool ascending = true);
	virtual ~CMessageSmartComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageSubjectComparator : public CMessageComparator
{
public:
	CMessageSubjectComparator(bool ascending = true);
	virtual ~CMessageSubjectComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageThreadComparator : public CMessageComparator
{
public:
	CMessageThreadComparator(bool ascending = true);
	virtual ~CMessageThreadComparator();

	virtual void SortMessageList(CMessageList* list);
	virtual MessageCompare	GetComparator()
						{ return nil; }

protected:
	static bool	PartitionFN(CMessage* inMsg);
	static const char* sPartitionThread;
	static unsigned long sPartitionThreadHash;
};

class CMessageMessageIDComparator : public CMessageComparator
{
public:
	CMessageMessageIDComparator(bool ascending = true);
	virtual ~CMessageMessageIDComparator();

	virtual void SortMessageList(CMessageList* list);
	virtual MessageCompare	GetComparator()
						{ return nil; }
};

class CMessageDateComparator : public CMessageComparator
{
public:
	CMessageDateComparator(bool ascending = true);
	virtual ~CMessageDateComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

	static MessageCompare	GetDateComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageInternalDateComparator : public CMessageComparator
{
public:
	CMessageInternalDateComparator(bool ascending = true);
	virtual ~CMessageInternalDateComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageSizeComparator : public CMessageComparator
{
public:
	CMessageSizeComparator(bool ascending = true);
	virtual ~CMessageSizeComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageFlagsComparator : public CMessageComparator
{
public:
	CMessageFlagsComparator(bool ascending = true);
	virtual ~CMessageFlagsComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageNumberComparator : public CMessageComparator
{
public:
	CMessageNumberComparator(bool ascending = true);
	virtual ~CMessageNumberComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageAttachmentComparator : public CMessageComparator
{
public:
	CMessageAttachmentComparator(bool ascending = true);
	virtual ~CMessageAttachmentComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessagePartsComparator : public CMessageComparator
{
public:
	CMessagePartsComparator(bool ascending = true);
	virtual ~CMessagePartsComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageMatchComparator : public CMessageComparator
{
public:
	CMessageMatchComparator(bool ascending = true);
	virtual ~CMessageMatchComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

class CMessageDisconnectedComparator : public CMessageComparator
{
public:
	CMessageDisconnectedComparator(bool ascending = true);
	virtual ~CMessageDisconnectedComparator();

	virtual MessageCompare	GetComparator()
						{ return CompareFN; }

protected:
	static bool	CompareFN(CMessage* inMsg1, CMessage* inMsg2);

};

#endif
