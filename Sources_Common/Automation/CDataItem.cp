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


// Header for CDataItem class

#include "CDataItem.h"

#include "CActionItem.h"
#include "CStringUtils.h"
#include "COSStringMap.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Specialisations of CDataItem

// Get text expansion for prefs
template<> cdstring CDataItem<cdstring>::GetInfo() const
{
	cdstring temp = mData;
	temp.quote();
	temp.ConvertFromOS();
	return temp;
}

// Convert text to items
template<> void CDataItem<cdstring>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	info.get(mData, true);
}

// Get text expansion for prefs
template<> cdstring CDataItem<COSStringMap>::GetInfo() const
{
	return mData.GetInfo();
}

// Convert text to items
template<> void CDataItem<COSStringMap>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	mData.SetInfo(info, vers_prefs);
}

// Get text expansion for prefs
template<> cdstring CDataItem<unsigned long>::GetInfo() const
{
	cdstring temp(mData);
	temp.quote();
	return temp;
}

// Convert text to items
template<> void CDataItem<unsigned long>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	info.get(mData);
}

// Get text expansion for prefs
template<> cdstring CDataItem<bool>::GetInfo() const
{
	return mData ? cValueBoolTrue : cValueBoolFalse;
}

// Convert text to items
template<> void CDataItem<bool>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	info.get(mData);
}

#pragma mark ____________________________Action Items

const char* cFlagDescriptors[] = {"None",
									"Recent",		// 1L << 0
									"Answered",		// 1L << 1
									"Flagged",		// 1L << 2
									"Deleted",		// 1L << 3
									"Seen",			// 1L << 4
									"Draft",		// 1L << 5
									"MDNSent",		// 1L << 6
									"",				// 1L << 7
									"",				// 1L << 8
									"",				// 1L << 9
									"",				// 1L << 10
									"",				// 1L << 11
									"Label1",		// 1L << 12
									"Label2",		// 1L << 13
									"Label3",		// 1L << 14
									"Label4",		// 1L << 15
									"Label5",		// 1L << 16
									"Label6",		// 1L << 17
									"Label7",		// 1L << 18
									"Label8",		// 1L << 19
									NULL
									};

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionFlags>::GetInfo() const
{
	cdstring all;

	// Add each flag descriptor to list
	cdstrvect items;
	unsigned long ctr = 0;
	const char** p = cFlagDescriptors;
	p++;
	while(*p)
	{
		if (mData.GetFlags() & (1L << ctr) && *p)
			items.push_back(*p);
		ctr++;
		p++;
	}
		
	// Use sexpression
	all.CreateSExpression(items);
	all += cSpace;

	all += (mData.IsSet() ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionFlags>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	// Get sexpression flags list
	NMessage::EFlags flags = NMessage::eNone;
	cdstrvect items;
	cdstring::ParseSExpression(info, items, false);
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		// Start on the second item in the descriptors
		const char** p = cFlagDescriptors;
		p++;
		unsigned long ctr = 0;
		while(*p)
		{
			if (!::strcmpnocase(*p, *iter))
			{
				flags = static_cast<NMessage::EFlags>(flags | (1L << ctr));
				break;
			}
			p++;
			ctr++;
		}
	}
	mData.SetFlags(flags);

	bool is_set;
	info.get(is_set);
	mData.Set(is_set);
}

const char* cReplyDescriptors[] = {"To", "From", "All", NULL};

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionReply>::GetInfo() const
{
	cdstring all;

	all += (mData.CreateDraft() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += cReplyDescriptors[mData.GetReplyType()];
	all += cSpace;

	all += (mData.Quote() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring temp = mData.GetText();
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	temp = mData.GetIdentity();
	temp.quote();
	all += temp;
	all += cSpace;

	all += (mData.UseTiedIdentity() ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionReply>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	bool is_set;
	info.get(is_set);
	mData.SetCreateDraft(is_set);

	cdstring temp;
	info.get(temp);
	if (temp == cReplyDescriptors[NMessage::eReplyTo])
		mData.SetReplyType(NMessage::eReplyTo);
	else if (temp == cReplyDescriptors[NMessage::eReplyFrom])
		mData.SetReplyType(NMessage::eReplyFrom);
	else if (temp == cReplyDescriptors[NMessage::eReplyAll])
		mData.SetReplyType(NMessage::eReplyAll);

	info.get(is_set);
	mData.SetQuote(is_set);
	
	info.get(temp, true);
	mData.SetText(temp);

	info.get(temp);
	mData.SetIdentity(temp);

	info.get(is_set);
	mData.SetTiedIdentity(is_set);
}

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionForward>::GetInfo() const
{
	cdstring all;

	all += (mData.CreateDraft() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring temp;
	temp = mData.Addrs().mTo;
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mData.Addrs().mCC;
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mData.Addrs().mBcc;
	temp.quote();
	all += temp;
	all += cSpace;

	all += (mData.Quote() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mData.Attach() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	temp = mData.GetText();
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	temp = mData.GetIdentity();
	temp.quote();
	all += temp;
	all += cSpace;

	all += (mData.UseTiedIdentity() ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionForward>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	bool is_set;
	info.get(is_set);
	mData.SetCreateDraft(is_set);

	info.get(mData.Addrs().mTo);

	info.get(mData.Addrs().mCC);

	info.get(mData.Addrs().mBcc);

	info.get(is_set);
	mData.SetQuote(is_set);
	
	info.get(is_set);
	mData.SetAttach(is_set);
	
	cdstring temp;
	info.get(temp, true);
	mData.SetText(temp);

	info.get(temp);
	mData.SetIdentity(temp);

	info.get(is_set);
	mData.SetTiedIdentity(is_set);
}

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionBounce>::GetInfo() const
{
	cdstring all;

	all += (mData.CreateDraft() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring temp;
	temp = mData.Addrs().mTo;
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mData.Addrs().mCC;
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mData.Addrs().mBcc;
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mData.GetIdentity();
	temp.quote();
	all += temp;
	all += cSpace;

	all += (mData.UseTiedIdentity() ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionBounce>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	bool is_set;
	info.get(is_set);
	mData.SetCreateDraft(is_set);

	info.get(mData.Addrs().mTo);

	info.get(mData.Addrs().mCC);

	info.get(mData.Addrs().mBcc);

	cdstring temp;
	info.get(temp);
	mData.SetIdentity(temp);

	info.get(is_set);
	mData.SetTiedIdentity(is_set);
}

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionReject>::GetInfo() const
{
	cdstring all;

	all += (mData.CreateDraft() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mData.ReturnMessage() ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring temp = mData.GetIdentity();
	temp.quote();
	all += temp;
	all += cSpace;

	all += (mData.UseTiedIdentity() ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionReject>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	bool is_set;
	info.get(is_set);
	mData.SetCreateDraft(is_set);

	info.get(is_set);
	mData.SetReturnMessage(is_set);
	
	cdstring temp;
	info.get(temp);
	mData.SetIdentity(temp);

	info.get(is_set);
	mData.SetTiedIdentity(is_set);
}

// Get text expansion for prefs
template<> cdstring CDataItem<CActionItem::CActionVacation>::GetInfo() const
{
	cdstring all;

	all += cdstring(mData.GetDays());
	all += cSpace;

	cdstring temp = mData.GetSubject();
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	temp = mData.GetText();
	temp.ConvertFromOS();
	temp.quote();
	all += temp;
	all += cSpace;

	temp.CreateSExpression(mData.GetAddresses());
	all += temp;

	return all;
}

// Convert text to items
template<> void CDataItem<CActionItem::CActionVacation>::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	unsigned long days;
	info.get(days);
	mData.SetDays(days);

	cdstring temp;
	info.get(temp, true);
	mData.SetSubject(temp);

	info.get(temp, true);
	mData.SetText(temp);

	cdstrvect addrs;
	cdstring::ParseSExpression(info, addrs, false);
	mData.SetAddresses(addrs);
}
