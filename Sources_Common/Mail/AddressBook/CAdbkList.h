/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CAdbkList.h

// Class to manage hierarchic list of address books. Based on template class.

#ifndef __CADBKLIST__MULBERRY__
#define __CADBKLIST__MULBERRY__

#include "ptr_hierarchy.h"

#include "CAddressBook.h"
#include "cdstring.h"

// Classes
class CAddressBook;
class cdstring;

typedef ptr_hierarchy<CAddressBook, cdstring> CAdbkList;
typedef std::vector<CAddressBook*> CFlatAdbkList;

#endif
