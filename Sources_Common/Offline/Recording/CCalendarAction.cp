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


// Source for an Calendar Action class

#include "CCalendarAction.h"

#include "CCalendarStoreNode.h"
#include "CStringUtils.h"

#include <istream>

extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* CCalendarActionDescriptors[] =
	{"None",
	 "Create",
	 "Delete",
	 "Rename",
	 "Change",
	 NULL
	 };

void CCalendarAction::_copy(const CCalendarAction& copy)
{
	mAction = copy.mAction;
	mID = copy.mID;

	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		mData = static_cast<void*>(new SCalendarAction(copy.GetCalendarAction()));
		break;
	case eRename:
		mData = static_cast<void*>(new SCalendarActionRename(copy.GetCalendarActionRename()));
		break;
	default:
		mData = NULL;
	}
}

void CCalendarAction::_tidy()
{
	// Delete data items
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		delete &GetCalendarAction();
		break;
	case eRename:
		delete &GetCalendarActionRename();
		break;
	default:;
	}

	mData = NULL;
}

const cdstring& CCalendarAction::GetCalendarName() const
{
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		return GetCalendarAction().mName;
	case eRename:
		return GetCalendarActionRename().first.mName;
	default:
		return cdstring::null_str;
	}
}

void CCalendarAction::WriteToStream(std::ostream& out, bool text) const
{
	// Create string list of items
	if (text)
		out << "ID=" << mID << " ";
	else
		out.write(reinterpret_cast<const char*>(&mID), 4);
	out << CCalendarActionDescriptors[mAction] << std::endl;

	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		WriteCalendarAction(out, GetCalendarAction());
		break;
	case eRename:
		WriteCalendarAction(out, GetCalendarActionRename().first);
		out << GetCalendarActionRename().second << std::endl;
		break;
	default:;
	}
	
	if (text)
		out << std::endl;
}

void CCalendarAction::ReadFromStream(std::istream& in, unsigned long vers)
{
	// get rid of existing
	_tidy();

	// Get action ID
	in.read(const_cast<char*>(reinterpret_cast<const char*>(&mID)), 4);

	// Get action type
	cdstring temp;
	getline(in, temp);
	mAction = static_cast<ECalendarAction>(::strindexfind(temp, CCalendarActionDescriptors, eNone));

	// Create data
	switch(mAction)
	{
	case eCreate:
	case eDelete:
	case eChange:
		mData = static_cast<void*>(new SCalendarAction);
		ReadCalendarAction(in, const_cast<SCalendarAction&>(GetCalendarAction()));
		break;
	case eRename:
		mData = static_cast<void*>(new SCalendarActionRename);
		ReadCalendarAction(in, const_cast<SCalendarAction&>(GetCalendarActionRename().first));
		getline(in, const_cast<cdstring&>(GetCalendarActionRename().second));
		break;
	default:;
	}
}


void CCalendarAction::WriteCalendarAction(std::ostream& out, const SCalendarAction& action) const
{
	out << (action.mIsDir ? cValueBoolTrue : cValueBoolFalse) << std::endl;
	out << action.mName << std::endl;
}

void CCalendarAction::ReadCalendarAction(std::istream& in, SCalendarAction& action)
{
	{
		cdstring temp;
		getline(in, temp);
		action.mIsDir = (temp == cValueBoolTrue);
	}
	{
		cdstring temp;
		getline(in, temp);
		action.mName = temp;
	}
}

CCalendarAction::SCalendarAction::SCalendarAction(const calstore::CCalendarStoreNode& node)
{
	mIsDir = node.IsDirectory();
	mName = node.GetName();
}
