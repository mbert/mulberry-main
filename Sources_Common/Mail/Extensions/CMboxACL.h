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


// ACL Support for mailboxes

#ifndef __CMBOXACL__MULBERRY__
#define __CMBOXACL__MULBERRY__

#include "CACL.h"

// Typedefs
class CMboxACL;
typedef std::vector<CMboxACL> CMboxACLList;

// CMboxACL: contains specific ACL item for a mailbox

class CMboxACL : public CACL
{
public:

	enum EMboxACL
	{
		eMboxACL_Lookup = 1L << 0,
		eMboxACL_Read = 1L << 1,
		eMboxACL_Seen = 1L << 2,
		eMboxACL_Write = 1L << 3,
		eMboxACL_Insert = 1L << 4,
		eMboxACL_Post = 1L << 5,
		eMboxACL_Create = 1L << 6,
		eMboxACL_Delete = 1L << 7,
		eMboxACL_Admin = 1L << 8
	};

	CMboxACL() {}
	CMboxACL(const CMboxACL& copy)						// Copy construct
		: CACL(copy) {}

	virtual 		~CMboxACL() {}
	
	CMboxACL& operator=(const CMboxACL& copy);			// Assignment with same type
	int operator==(const CMboxACL& test) const			// Compare with same type
		{ return CACL::operator==(test); }
	int operator!=(const CMboxACL& test) const			// Compare with same type
		{ return !operator==(test); }

	virtual cdstring	GetTextRights() const;		// Get full text form of rights
	virtual cdstring	GetFullTextRights() const;	// Get full text form of rights

private:
	virtual void	ParseRights(const char* txt, SACLRight& rights);		// Parse string to specified rigths location

};

#endif
