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


// ACL Support for address books

#ifndef __CCalendarACL__MULBERRY__
#define __CCalendarACL__MULBERRY__

#include "CACL.h"

#include "XMLDocument.h"
#include "XMLName.h"
#include "XMLNode.h"

// Typedefs
class CCalendarACL;
typedef std::vector<CCalendarACL> CCalendarACLList;

// CCalendarACL: contains specific ACL item for a mailbox

class CCalendarACL : public CACL
{
public:

	enum ECalACL
	{
		eCalACL_ReadFreeBusy = 1L << 0,
		eCalACL_Read = 1L << 1,
		eCalACL_Write = 1L << 2,
		eCalACL_Schedule = 1L << 3,
		eCalACL_Create = 1L << 4,
		eCalACL_Delete = 1L << 5,
		eCalACL_Admin = 1L << 6,
		eCalACL_All = 1L << 7
	};

	enum ECalPrincipalType
	{
		ePrincipal_unknown = 0,
		ePrincipal_href,
		ePrincipal_all,
		ePrincipal_authenticated,
		ePrincipal_unauthenticated,
		ePrincipal_property,
		ePrincipal_self	
	};

	CCalendarACL()
	{
		mType = ePrincipal_unknown;
		mInheritable = false;
		mCanChange = true;
	}
	CCalendarACL(const CCalendarACL& copy)						// Copy construct
		: CACL(copy)
	{
		mType = copy.mType;
		mInheritable = copy.mInheritable;
		mCanChange = copy.mCanChange;
		mPropName = copy.mPropName;
	}

	virtual 		~CCalendarACL() {}
	
	CCalendarACL& operator=(const CCalendarACL& copy);			// Assignment with same type
	int operator==(const CCalendarACL& test) const			// Compare with same type
		{ return CACL::operator==(test); }
	int operator!=(const CCalendarACL& test) const			// Compare with same type
		{ return !operator==(test); }

	virtual const cdstring&	GetDisplayUID() const;

	virtual cdstring	GetTextRights() const;		// Get full text form of rights
	virtual cdstring	GetFullTextRights() const;	// Get full text form of rights

	bool SamePrincipal(const CCalendarACL& acl) const;
	ECalPrincipalType GetPrincipalType() const
	{
		return mType;
	}
	void SetPrincipalType(ECalPrincipalType type)
	{
		mType = type;
	}
	void SetPrincipalType(ECalPrincipalType type, const cdstring& txt)
	{
		mType = type;
		SetUID(txt.c_str());
	}
	void SetPrincipalType(ECalPrincipalType type, const xmllib::XMLName& name)
	{
		mType = type;
		mPropName = name;
	}
	void SetSmartUID(const cdstring& txt);

	const xmllib::XMLName& GetPropName() const
	{
		return mPropName;
	}

	bool IsInheritable() const
	{
		return mInheritable;
	}
	void SetInheritable(bool inheritable)
	{
		mInheritable = inheritable;
	}

	bool CanChange() const
	{
		return mCanChange;
	}
	void SetCanChange(bool can_change)
	{
		mCanChange = can_change;
	}
	
	bool AllRights() const;

	bool ParseACE(const xmllib::XMLNode* acenode);
	void ParsePrivilege(const xmllib::XMLNode* parent, bool add);
	void GenerateACE(xmllib::XMLDocument* xmldoc, xmllib::XMLNode* aclnode) const;

private:
	ECalPrincipalType	mType;
	bool				mInheritable;
	bool				mCanChange;
	xmllib::XMLName		mPropName;
	

	virtual void	ParseRights(const char* txt, SACLRight& rights);		// Parse string to specified rigths location

			void	MapRight(const cdstring& right, bool add);

};

#endif
