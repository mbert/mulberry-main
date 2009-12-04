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


// Source for an Mail Action class

#include "CMailAction.h"

#include "CStringUtils.h"

#include <istream>

extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* cMailActionDescriptors[] =
	{"None",
	 "Create",
	 "Delete",
	 "Rename",
	 "Subscribe",
	 "Unsubscribe",
	 "Select",
	 "Deselect",
	 "Flag",
	 "Copy",
	 "Append ",
	 "Expunge",
	 NULL
	 };

void CMailAction::_copy(const CMailAction& copy)
{
	mAction = copy.mAction;
	mID = copy.mID;

	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eSubscribe:
	case eUnsubscribe:
	case eSelect:
	case eDeselect:
		mData = static_cast<void*>(new SNameUIDAction(copy.GetNameUIDAction()));
		break;
	case eRename:
		mData = static_cast<void*>(new SRenameAction(copy.GetRenameAction()));
		break;
	case eFlag:
		mData = static_cast<void*>(new SFlagAction(copy.GetFlagAction()));
		break;
	case eCopy:
		mData = static_cast<void*>(new SCopyAction(copy.GetCopyAction()));
		break;
	case eAppend:
		mData = static_cast<void*>(new SNameUIDAction(copy.GetNameUIDAction()));
		break;
	case eExpunge:
		mData = static_cast<void*>(new SExpungeAction(copy.GetExpungeAction()));
		break;
	default:
		mData = NULL;
	}
}

void CMailAction::_tidy()
{
	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eSubscribe:
	case eUnsubscribe:
	case eSelect:
	case eDeselect:
		delete &GetNameUIDAction();
		break;
	case eRename:
		delete &GetRenameAction();
		break;
	case eFlag:
		delete &GetFlagAction();
		break;
	case eCopy:
		delete &GetCopyAction();
		break;
	case eAppend:
		delete &GetNameUIDAction();
		break;
	case eExpunge:
		delete &GetExpungeAction();
		break;
	default:;
	}

	mData = NULL;
}

// Remove non-matching uids
bool CMailAction::RemoveUIDs(const ulvector& leave)
{
	switch(mAction)
	{
	case eCreate:
	case eRename:
	case eDelete:
	case eSubscribe:
	case eUnsubscribe:
	case eSelect:
	case eDeselect:
	default:;
		return true;

	case eFlag:
		// Remove all server UIDs
		const_cast<ulvector&>(GetFlagAction().mUids.first).clear();
		return !GetFlagAction().mUids.second.size();

	case eCopy:
		// Remove all server UID maps
		const_cast<ulmap&>(GetCopyAction().second.first).clear();
		return !GetCopyAction().second.second.size();

	case eAppend:
		return false;

	case eExpunge:
		// Remove all server UIDs
		const_cast<ulvector&>(GetExpungeAction().first).clear();
		return !GetExpungeAction().second.size();
	}
	
	return true;
}

void CMailAction::WriteToStream(std::ostream& out, bool text) const
{
	// Create string list of items
	if (text)
		out << "ID=" << mID << " ";
	else
		out.write(reinterpret_cast<const char*>(&mID), 4);
	out << cMailActionDescriptors[mAction] << std::endl;

	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eSubscribe:
	case eUnsubscribe:
	case eSelect:
	case eDeselect:
		out << GetNameUIDAction().first << std::endl;
		if (text)
			out << GetNameUIDAction().second;
		else
			out.write(reinterpret_cast<const char*>(&GetNameUIDAction().second), 4);
		out << std::endl;
		break;
	case eRename:
		out << GetRenameAction().first.first << std::endl;
		out << GetRenameAction().first.second << std::endl;
		if (text)
			out << GetRenameAction().second;
		else
			out.write(reinterpret_cast<const char*>(&GetRenameAction().second), 4);
		out << std::endl;
		break;
	case eFlag:
		WriteUIDS(out, GetFlagAction().mUids.first, text);
		WriteUIDS(out, GetFlagAction().mUids.second, text);
		if (text)
			out << GetFlagAction().mFlags;
		else
			out.write(reinterpret_cast<const char*>(&GetFlagAction().mFlags), 4);
		if (text)
			out << " set=" << (GetFlagAction().mSet ? cValueBoolTrue : cValueBoolFalse);
		else
		{
			char set = GetFlagAction().mSet ? 1 : 0;
			out.write(&set, 1);
		}
		out << std::endl;
		break;
	case eCopy:
		out << GetCopyAction().first << std::endl;
		WriteUIDMap(out, GetCopyAction().second.first, text);
		WriteUIDMap(out, GetCopyAction().second.second, text);
		break;
	case eAppend:
		out << GetAppendAction().mName << std::endl;
		if (text)
		{
			out << GetAppendAction().mUid << std::endl;
			out << GetAppendAction().mFlags;
		}
		else
		{
			// Write a null between the uid and flag so we can tell we have the new format
			out.write(reinterpret_cast<const char*>(&GetAppendAction().mUid), 4);
			char temp = 0;
			out.write(&temp, 1);
			out.write(reinterpret_cast<const char*>(&GetAppendAction().mFlags), 4);
		}
		out << std::endl;
		break;
	case eExpunge:
		WriteUIDS(out, GetExpungeAction().first, text);
		WriteUIDS(out, GetExpungeAction().second, text);
		break;
	default:;
	}
	
	if (text)
		out << std::endl;
}

void CMailAction::ReadFromStream(std::istream& in, unsigned long vers)
{
	// get rid of existing
	_tidy();

	// Get action ID
	in.read(const_cast<char*>(reinterpret_cast<const char*>(&mID)), 4);

	// Get action type
	cdstring temp;
	getline(in, temp);
	mAction = static_cast<EMailAction>(::strindexfind(temp, cMailActionDescriptors, eNone));

	// Create data
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eSubscribe:
	case eUnsubscribe:
	case eSelect:
	case eDeselect:
		mData = static_cast<void*>(new SNameUIDAction);
		getline(in, const_cast<cdstring&>(GetNameUIDAction().first));
		in.read(const_cast<char*>(reinterpret_cast<const char*>(&GetNameUIDAction().second)), 4);
		in.ignore();
		break;
	case eRename:
		mData = static_cast<void*>(new SRenameAction);
		getline(in, const_cast<cdstring&>(GetRenameAction().first.first));
		getline(in, const_cast<cdstring&>(GetRenameAction().first.second));
		in.read(const_cast<char*>(reinterpret_cast<const char*>(&GetRenameAction().second)), 4);
		in.ignore();
		break;
	case eFlag:
		mData = static_cast<void*>(new SFlagAction);
		ReadUIDS(in, vers, const_cast<ulvector&>(GetFlagAction().mUids.first));
		ReadUIDS(in, vers, const_cast<ulvector&>(GetFlagAction().mUids.second));
		in.read(const_cast<char*>(reinterpret_cast<const char*>(&GetFlagAction().mFlags)), 4);
		char set;
		in.read(&set, 1);
		*const_cast<bool*>(&GetFlagAction().mSet) = set;
		in.ignore();
		break;
	case eCopy:
		mData = static_cast<void*>(new SCopyAction);
		getline(in, const_cast<cdstring&>(GetCopyAction().first));
		ReadUIDMap(in, vers, const_cast<ulmap&>(GetCopyAction().second.first));
		ReadUIDMap(in, vers, const_cast<ulmap&>(GetCopyAction().second.second));
		break;
	case eAppend:
		// Format of this changed in v2.2
		{
			mData = static_cast<void*>(new SAppendAction);
			getline(in, const_cast<cdstring&>(GetAppendAction().mName));
			in.read(const_cast<char*>(reinterpret_cast<const char*>(&GetAppendAction().mUid)), 4);

			// Check next character to see if its a lineend (old format) and ignore
			// Otherwise read a flag
			char test = 0;
			in.read(&test, 1);
			if (test != '\n')
			{
				in.read(const_cast<char*>(reinterpret_cast<const char*>(&GetAppendAction().mFlags)), 4);
				in.ignore();
			}
		}
		break;
	case eExpunge:
		mData = static_cast<void*>(new SExpungeAction);
		ReadUIDS(in, vers, const_cast<ulvector&>(GetExpungeAction().first));
		ReadUIDS(in, vers, const_cast<ulvector&>(GetExpungeAction().second));
		break;
	default:;
	}
}

void CMailAction::WriteUIDS(std::ostream& out, const ulvector& uids, bool text) const
{
	unsigned long temp = uids.size();
	if (text)
		out << temp << ": ";
	else
		out.write(reinterpret_cast<const char*>(&temp), 4);
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		if (text)
		{
			if (iter != uids.begin())
				out << ",";
			out << *iter;
		}
		else
			out.write(reinterpret_cast<const char*>(&(*iter)), 4);
	}
	out << std::endl;
}

void CMailAction::ReadUIDS(std::istream& in, unsigned long vers, ulvector& uids)
{
	unsigned long size;
	in.read(reinterpret_cast<char*>(&size), 4);
	for(unsigned long i = 0; i < size; i++)
	{
		unsigned long temp;
		in.read(reinterpret_cast<char*>(&temp), 4);
		uids.push_back(temp);
	}
	in.ignore();
}

void CMailAction::WriteUIDMap(std::ostream& out, const ulmap& uids, bool text) const
{
	unsigned long temp = uids.size();
	if (text)
		out << temp << ": ";
	else
		out.write(reinterpret_cast<const char*>(&temp), 4);
	for(ulmap::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		if (text)
		{
			if (iter != uids.begin())
				out << ",";
			out << (*iter).first << " " << (*iter).second;
		}
		else
		{
			out.write(reinterpret_cast<const char*>(&(*iter).first), 4);
			out.write(reinterpret_cast<const char*>(&(*iter).second), 4);
		}
	}
	out << std::endl;
}

void CMailAction::ReadUIDMap(std::istream& in, unsigned long vers, ulmap& uids)
{
	unsigned long size;
	in.read(reinterpret_cast<char*>(&size), 4);
	for(unsigned long i = 0; i < size; i++)
	{
		unsigned long temp1;
		unsigned long temp2;
		in.read(reinterpret_cast<char*>(&temp1), 4);
		in.read(reinterpret_cast<char*>(&temp2), 4);
		uids.insert(ulmap::value_type(temp1, temp2));
	}
	in.ignore();
}
