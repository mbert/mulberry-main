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


// ACL support for address books

#include "CAdbkACL.h"

#include "CWebDAVDefinitions.h"
#include "CXStringResources.h"

const char cAdbkACLFlags[] = "lrwcda";

// CAdbkACL: contains specific ACL item for a address book

// Assignment with same type
CAdbkACL& CAdbkACL::operator=(const CAdbkACL& copy)
{
	if (this != &copy)
	{
		CACL::operator=(copy);
		mType = copy.mType;
		mCanChange = copy.mCanChange;
		mPropName = copy.mPropName;
	}

	return *this;
}

const cdstring&	CAdbkACL::GetDisplayUID() const
{
	switch(mType)
	{
	case ePrincipal_href:
	default:
		return mUID;
	case ePrincipal_all:
		return rsrc::GetString("UI::CalendarProp::ACLPrincipal::All");
	case ePrincipal_authenticated:
		return rsrc::GetString("UI::CalendarProp::ACLPrincipal::Authenticated");
	case ePrincipal_unauthenticated:
		return rsrc::GetString("UI::CalendarProp::ACLPrincipal::Unauthenticated");
	case ePrincipal_property:
		return rsrc::GetString("UI::CalendarProp::ACLPrincipal::Property");
	case ePrincipal_self:
		return rsrc::GetString("UI::CalendarProp::ACLPrincipal::Self");
	}
}

// Parse string to specified rigths location
void CAdbkACL::ParseRights(const char* txt, SACLRight& rights)
{
	// Null existing rights
	rights.SetRight(SACLRight::eACL_AllRights, false);

	// Look for specific characters in string
	long pos = 0;
	const char* p = cAdbkACLFlags;
	if (txt)
	{
		while(*p)
		{
			if (::strchr(txt, *p) != nil)
				rights.SetRight(1L << pos, true);

			p++;
			pos++;
		}
	}
}

// Get text form of rights
cdstring CAdbkACL::GetTextRights() const
{
	cdstring rights;

	// Look for specific rights
	long pos = 0;
	const char* p = cAdbkACLFlags;
	while(*p)
	{
		if (HasRight(1L << pos))
			rights += *p;

		p++;
		pos++;
	}

	// Must quote empty string
	rights.quote();

	return rights;
}
// Get full text form of rights
cdstring CAdbkACL::GetFullTextRights() const
{
	cdstring txt = mUID + ' ';

	return txt + GetTextRights();
}

bool CAdbkACL::SamePrincipal(const CAdbkACL& acl) const
{
	if (mType != acl.mType)
		return false;
	
	switch(mType)
	{
	case ePrincipal_unknown:
	default:
		return false;
	case ePrincipal_href:
		return mUID == acl.mUID;
	case ePrincipal_all:
	case ePrincipal_authenticated:
	case ePrincipal_unauthenticated:
	case ePrincipal_self:
		return true;
	case ePrincipal_property:
		return mPropName == acl.mPropName;
	}
}

bool CAdbkACL::AllRights() const
{
	return HasRight(eAdbkACL_All) &&
			HasRight(eAdbkACL_Lookup) &&
			HasRight(eAdbkACL_Read) &&
			HasRight(eAdbkACL_Write) &&
			HasRight(eAdbkACL_Create) &&
			HasRight(eAdbkACL_Delete) &&
			HasRight(eAdbkACL_Admin);
}

bool CAdbkACL::ParseACE(const xmllib::XMLNode* acenode)
{
	// Get the principal
	const xmllib::XMLNode* principal = acenode->GetChild(http::webdav::cProperty_principal);
	if (principal == NULL)
		return false;
	
	// Determine principal info
	const xmllib::XMLNode* child = NULL;
	if ((child = principal->GetChild(http::webdav::cProperty_href)) != NULL)
	{
		cdstring href(child->Data());
		href.DecodeURL();
		SetPrincipalType(CAdbkACL::ePrincipal_href, href);
	}
	else if ((child = principal->GetChild(http::webdav::cProperty_all)) != NULL)
	{
		SetPrincipalType(CAdbkACL::ePrincipal_all);
	}
	else if ((child = principal->GetChild(http::webdav::cProperty_authenticated)) != NULL)
	{
		SetPrincipalType(CAdbkACL::ePrincipal_authenticated);
	}
	else if ((child = principal->GetChild(http::webdav::cProperty_unauthenticated)) != NULL)
	{
		SetPrincipalType(CAdbkACL::ePrincipal_unauthenticated);
	}
	else if ((child = principal->GetChild(http::webdav::cProperty_property)) != NULL)
	{
		if (child->Children().size() == 1)
		{
			xmllib::XMLName propname(*child->Children().front());
			SetPrincipalType(CAdbkACL::ePrincipal_property, propname);
		}
		else
			SetPrincipalType(CAdbkACL::ePrincipal_property);
	}
	else if ((child = principal->GetChild(http::webdav::cProperty_self)) != NULL)
	{
		SetPrincipalType(CAdbkACL::ePrincipal_self);
	}
	
	// Determine rights
	child = acenode->GetChild(http::webdav::cProperty_grant);
	if (child == NULL)
		child = acenode->GetChild(http::webdav::cProperty_deny);
	if (child != NULL)
	{
		ParsePrivilege(child, child->CompareFullName(http::webdav::cProperty_grant));
	}

	// Determine protected/inherited state
	if ((acenode->GetChild(http::webdav::cProperty_protected) != NULL) ||
		(acenode->GetChild(http::webdav::cProperty_inherited) != NULL))
		SetCanChange(false);
	
	// Determine inheritable/non-inheritable
	if (acenode->GetChild(http::slide::cInheritable_comp) != NULL)
		SetInheritable(true);
	else if (acenode->GetChild(http::slide::cNonInheritable_comp) != NULL)
		SetInheritable(false);
	
	return true;
}

void CAdbkACL::ParsePrivilege(const xmllib::XMLNode* parent, bool add)
{
	// parent node contains one of more privilege nodes which we parse
	for(xmllib::XMLNodeList::const_iterator iter1 = parent->Children().begin(); iter1 != parent->Children().end(); iter1++)
	{
		xmllib::XMLNode* privilege = *iter1;

		// Look for privilege
		if (!privilege->CompareFullName(http::webdav::cProperty_privilege))
			continue;
		
		// Now get rights within the privilege
		for(xmllib::XMLNodeList::const_iterator iter2 = privilege->Children().begin(); iter2 != privilege->Children().end(); iter2++)
		{
			xmllib::XMLNode* right = *iter2;
			cdstring name = right->Name();
			MapRight(name, add);
		}
	}
}

void CAdbkACL::MapRight(const cdstring& right, bool add)
{
	// Cache the mappings for this type of client
	typedef std::map<cdstring, unsigned long> CMapRights;
	static CMapRights sMapRights;
	
	if (sMapRights.empty())
	{
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_read.Name(), eAdbkACL_Lookup | eAdbkACL_Read));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_write.Name(), eAdbkACL_Write));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_write_properties.Name(), 0));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_write_content.Name(), eAdbkACL_Write));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_unlock.Name(), 0));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_read_acl.Name(), 0));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_read_current_user_privilege_set.Name(), 0));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_write_acl.Name(), eAdbkACL_Admin));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_bind.Name(), eAdbkACL_Create));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_unbind.Name(), eAdbkACL_Delete));
		sMapRights.insert(CMapRights::value_type(http::webdav::cProperty_privilege_all.Name(), eAdbkACL_Lookup |
																			eAdbkACL_Read |
																			eAdbkACL_Write |
																			eAdbkACL_Create |
																			eAdbkACL_Delete |
																			eAdbkACL_Admin |
																			eAdbkACL_All));
	}
	
	// Find entry in map
	CMapRights::const_iterator found = sMapRights.find(right);
	if (found != sMapRights.end())
	{
		SetRight((*found).second, add);
	}
}

void CAdbkACL::GenerateACE(xmllib::XMLDocument* xmldoc, xmllib::XMLNode* aclnode) const
{
	// Structure of ace is:
	//
	//   <DAV:ace>
	//     <DAV:principal>...</DAV:principal>
	//	   <DAV:grant>...</DAV:grant>
	//   </DAV:ace>

	// <DAV:ace> element
	xmllib::XMLNode* ace = new xmllib::XMLNode(xmldoc, aclnode, http::webdav::cProperty_ace);
	
	// <DAV:principal> element
	xmllib::XMLNode* principal = new xmllib::XMLNode(xmldoc, ace, http::webdav::cProperty_principal);
	
	// Principal type
	switch(GetPrincipalType())
	{
	case CAdbkACL::ePrincipal_href:
	default:
	{
		// <DAV:href> element
		xmllib::XMLNode* href = new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_href);
		cdstring txt(GetUID());
		if (txt.find('%') == cdstring::npos)
			txt.EncodeURL('/');
		href->SetData(txt);
		break;
	}
	case CAdbkACL::ePrincipal_all:
	{
		// <DAV:all> element
		new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_all);
		break;
	}
	case CAdbkACL::ePrincipal_authenticated:
	{
		// <DAV:authenticated> element
		new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_authenticated);
		break;
	}
	case CAdbkACL::ePrincipal_unauthenticated:
	{
		// <DAV:unauthenticated> element
		new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_unauthenticated);
		break;
	}
	case CAdbkACL::ePrincipal_property:
	{
		// <DAV:property> element - the UID is the property element name
		xmllib::XMLNode* property = new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_property);
		/* xmllib::XMLNode* propname = */ new xmllib::XMLNode(xmldoc, property, GetPropName());
		break;
	}
	case CAdbkACL::ePrincipal_self:
	{
		// <DAV:self> element
		new xmllib::XMLNode(xmldoc, principal, http::webdav::cProperty_self);
		break;
	}
	}
	
	// Do grant rights for each one set
	
	// <DAV:grant> element
	xmllib::XMLNode* grant = new xmllib::XMLNode(xmldoc, ace, http::webdav::cProperty_grant);
	
	// Special check for "DAV:all"
	if (AllRights() ||
		HasRight(CAdbkACL::eAdbkACL_Lookup) &&
		HasRight(CAdbkACL::eAdbkACL_Read) &&
		HasRight(CAdbkACL::eAdbkACL_Write) &&
		HasRight(CAdbkACL::eAdbkACL_Create) &&
		HasRight(CAdbkACL::eAdbkACL_Delete) &&
		HasRight(CAdbkACL::eAdbkACL_Admin))
		new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_all);
	else
	{
		if (HasRight(CAdbkACL::eAdbkACL_Lookup) || HasRight(CAdbkACL::eAdbkACL_Read))
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_read);
		if (HasRight(CAdbkACL::eAdbkACL_Write))
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_write);
		if (HasRight(CAdbkACL::eAdbkACL_Create))
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_bind);
		if (HasRight(CAdbkACL::eAdbkACL_Delete))
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_unbind);
		if (HasRight(CAdbkACL::eAdbkACL_Admin))
		{
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_read_acl);
			new xmllib::XMLNode(xmldoc, new xmllib::XMLNode(xmldoc, grant, http::webdav::cProperty_privilege), http::webdav::cProperty_privilege_write_acl);
		}
	}
}
