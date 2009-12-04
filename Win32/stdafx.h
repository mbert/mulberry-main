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

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// public headers
#include "afxwin.h"
#include "afxdlgs.h"
#include "afxext.h"
#ifndef _AFX_NO_OLE_SUPPORT
	#include "afxole.h"
#ifndef _AFXCTL
	#include "afxodlgs.h"
#else
	#define _OLE_QUIET
	#include <ole2ui.h>
#endif
#endif
#ifndef _AFX_NO_DB_SUPPORT
	#include "afxdb.h"
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
	#include <afxcmn.h>
#endif

#include <afxtempl.h>
#include <afximpl.h>