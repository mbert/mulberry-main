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

#include "CAdbkAction.h"

#include "CStringUtils.h"

#include <istream>
#include <stdlib.h>

const char* cAdbkActionDescriptors[] =
	{"None",
	 "Create",
	 "Delete",
	 "Rename",
	 "Change",
	 "Store Address",
	 "Change Address",
	 "Delete Address",
	 "Store Group ",
	 "Change Group ",
	 "Delete Group",
	 NULL
	 };

void CAdbkAction::_copy(const CAdbkAction& copy)
{
	mAction = copy.mAction;
	mID = copy.mID;

	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		mData = static_cast<void*>(new cdstring(copy.GetName()));
		break;
	case eRename:
		mData = static_cast<void*>(new cdstrpair(copy.GetNamePair()));
		break;
	case eStoreAddress:
	case eChangeAddress:
	case eDeleteAddress:
	case eStoreGroup:
	case eChangeGroup:
	case eDeleteGroup:
		mData = static_cast<void*>(new cdstrvect(copy.GetList()));
		break;
	default:
		mData = NULL;
	}
}

void CAdbkAction::_tidy()
{
	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		delete &GetName();
		break;
	case eRename:
		delete &GetNamePair();
		break;
	case eStoreAddress:
	case eChangeAddress:
	case eDeleteAddress:
	case eStoreGroup:
	case eChangeGroup:
	case eDeleteGroup:
		delete &GetList();
		break;
	default:;
	}

	mData = NULL;
}

void CAdbkAction::WriteToStream(std::ostream& out, bool text) const
{
	// Create string list of items
	if (text)
		out << "ID=" << mID << " ";
	else
		out.write(reinterpret_cast<const char*>(&mID), 4);
	out << cAdbkActionDescriptors[mAction] << std::endl;

	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		out << GetName() << std::endl;
		break;
	case eRename:
		out << GetNamePair().first << std::endl;
		out << GetNamePair().second << std::endl;
		break;
	case eStoreAddress:
	case eChangeAddress:
	case eDeleteAddress:
	case eStoreGroup:
	case eChangeGroup:
	case eDeleteGroup:
		out << GetList().size() << std::endl;
		for(cdstrvect::const_iterator iter = GetList().begin(); iter != GetList().end(); iter++)
			out << *iter << std::endl;
		break;
	default:;
	}
	
	if (text)
		out << std::endl;
}

void CAdbkAction::ReadFromStream(std::istream& in, unsigned long vers)
{
	// get rid of existing
	_tidy();

	// Get action ID
	in.read(const_cast<char*>(reinterpret_cast<const char*>(&mID)), 4);

	// Get action type
	cdstring temp;
	getline(in, temp);
	mAction = static_cast<EAdbkAction>(::strindexfind(temp, cAdbkActionDescriptors, eNone));

	// Create data
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		mData = static_cast<void*>(new cdstring);
		getline(in, const_cast<cdstring&>(GetName()));
		break;
	case eRename:
		mData = static_cast<void*>(new cdstrpair);
		getline(in, const_cast<cdstring&>(GetNamePair().first));
		getline(in, const_cast<cdstring&>(GetNamePair().second));
		break;
	case eStoreAddress:
	case eChangeAddress:
	case eDeleteAddress:
	case eStoreGroup:
	case eChangeGroup:
	case eDeleteGroup:
	{
		cdstring strsize;
		getline(in, strsize);
		unsigned long size = ::strtoul(strsize.c_str(), NULL, 10);

		cdstrvect* vect = new cdstrvect;
		mData = static_cast<void*>(vect);
		for(unsigned long i = 0; i < size; i++)
		{
			cdstring temp;
			getline(in, temp);
			vect->push_back(temp);
		}

		break;
	}
	default:;
	}
}
