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

// CPrefsStrings.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 24-May-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// A list of sprint-able preferences for Mulberry.
//
// History:
// 24-May-1999: Created initial header and implementation.
//

#include "CPrefsStrings.h"

#ifndef MULBERRY_V2

#pragma mark ____________________________Version 1.x

	// %s #1 = server ip prefix
	// %s #2 = user id
	const char* cMailAccountPreference =
		"Preferences.Accounts.Mail Accounts="
		"(((imap IMAP \"%s.imap.cyrusoft.com\" true "
		"((\"Plain Text\" \"%s\" \"\" true false)))"
		"(. true ((INBOX. true false)) "
		"true false true true \"\")))";

#else

#pragma mark ____________________________Version 2.x

	// %s #1 = server ip prefix
	// %s #2 = user id
	const char* cMailAccountPreference =
		"Preferences.Accounts.Mail Accounts v2="
		"(((imap IMAP \"%s.imap.cyrusoft.com\" true "
		"((\"Plain Text\" \"%s\" \"\" true false)) "
		"(\"\" true true))"
		"(. true ((INBOX. true false)) "
		"true false false false false Default)))";

#endif
