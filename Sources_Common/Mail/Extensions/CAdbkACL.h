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

#ifndef __CADBKACL__MULBERRY__
#define __CADBKACL__MULBERRY__

#include "CACL.h"

#include "XMLDocument.h"
#include "XMLName.h"
#include "XMLNode.h"

// Typedefs
class CAdbkACL;
typedef std::vector<CAdbkACL> CAdbkACLList;

// CAdbkACL: contains specific ACL item for a mailbox

class CAdbkACL : public CACL
{
public:

	enum EAdbkACL
	{
		eAdbkACL_Lookup = 1L << 0,
		eAdbkACL_Read = 1L << 1,
		eAdbkACL_Write = 1L << 2,
		eAdbkACL_Create = 1L << 3,
		eAdbkACL_Delete = 1L << 4,
		eAdbkACL_Admin = 1L << 5,
		eAdbkACL_All = 1L << 6
	};

	enum EAdbkPrincipalType
	{
		ePrincipal_unknown = 0,
		ePrincipal_href,
		ePrincipal_all,
		ePrincipal_authenticated,
		ePrincipal_unauthenticated,
		ePrincipal_property,
		ePrincipal_self	
	};

	CAdbkACL()
	{
		mType = ePrincipal_unknown;
		mInheritable = false;
		mCanChange = true;
	}
	CAdbkACL(const CAdbkACL& copy)						// Copy construct
		: CACL(copy)
	{
		mType = copy.mType;
		mInheritable = copy.mInheritable;
		mCanChange = copy.mCanChange;
		mPropName = copy.mPropName;
	}

	virtual 		~CAdbkACL() {}
	
	CAdbkACL& operator=(const CAdbkACL& copy);			// Assignment with same type
	int operator==(const CAdbkACL& test) const			// Compare with same type
		{ return CACL::operator==(test); }
	int operator!=(const CAdbkACL& test) const			// Compare with same type
		{ return !operator==(test); }

	virtual const cdstring&	GetDisplayUID() const;

	virtual cdstring	GetTextRights() const;		// Get full text form of rights
	virtual cdstring	GetFullTextRights() const;	// Get full text form of rights

	bool SamePrincipal(const CAdbkACL& acl) const;
	EAdbkPrincipalType GetPrincipalType() const
	{
		return mType;
	}
	void SetPrincipalType(EAdbkPrincipalType type)
	{
		mType = type;
	}
	void SetPrincipalType(EAdbkPrincipalType type, const cdstring& txt)
	{
		mType = type;
		SetUID(txt);
	}
	void SetPrincipalType(EAdbkPrincipalType type, const xmllib::XMLName& name)
	{
		mType = type;
		mPropName = name;
	}

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
	void GenerateACE(xmllib::XMLDocument* xmldoc, xmllib::XMLNode* aclnode, bool can_use_inheritable = false) const;

private:
	EAdbkPrincipalType	mType;
	bool				mInheritable;
	bool				mCanChange;
	xmllib::XMLName		mPropName;
	

	virtual void	ParseRights(const char* txt, SACLRight& rights);		// Parse string to specified rigths location

			void	MapRight(const cdstring& right, bool add);
};

#endif
