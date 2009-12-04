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


// ACL support

#include "CACL.h"

// CACL: contains specific ACL item
// This is an abstract base class that should be derived for different types of ACL models
// e.g. for mailboxes, address books, preferences etc

// Copy construct
CACL::CACL(const CACL& copy)
{
	mUID = copy.GetUID();
	mCurrent = copy.GetRights();
	mAllowed = copy.GetAllowedRights();
}

// Assignment with same type
CACL& CACL::operator=(const CACL& copy)
{
	if (this != &copy)
	{
		mUID = copy.GetUID();
		mCurrent= copy.GetRights();
		mAllowed = copy.GetAllowedRights();
	}

	return *this;
}
