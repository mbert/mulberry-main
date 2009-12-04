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


// Header for Mailbox class

#ifndef __CMBOXFWD__MULBERRY__
#define __CMBOXFWD__MULBERRY__

#include <vector>

namespace NMbox
{
	enum EFlags
	{
		// Mbox status
		eNone = 					0,
		eOpen =						1L << 0,		// Connection open but data structures may not be
		eFullOpen =					1L << 1,		// Connection and data structures in place
		eBeingOpened =				1L << 2,		// In the process of being opened (private)
		eSubscribed =				1L << 3,
		eReadOnly =					1L << 4,
		eExamine =					1L << 5,
		eNoCache =					1L << 6,
		eAutoCheck =				1L << 7,
		eCopyTo =					1L << 8,
		eAppendTo =					1L << 9,
		ePuntOnClose =				1L << 10,
		eAutoSync =					1L << 11,
		eDeleteOnClose =			1L << 12,
		eLocalMbox =				1L << 13,
		eCachedMbox =				1L << 14,
		eSynchronising =			1L << 15,		// Mailbox is being synchronised
		eNoCabinet =				1L << 16,		// Must not show up in any cabinets
		eExternalSort =				1L << 17,		// Sorting on server
		eOpenSomewhere =			1L << 18,		// Mailbox of this name is open somewhere
		
		// IMAP flags - do not change these values as they
		// may be cached in disconnected mailboxlist files
		eNoInferiors =				1L << 24,
		eNoSelect =					1L << 25,
		eMarked =					1L << 26,
		eUnMarked =					1L << 27,
		
		eIMAPFlags =				eNoInferiors | eNoSelect | eMarked | eUnMarked,

		eError =					1L << 28,		// Pseudo flag for display
		eHasInferiors =				1L << 29,		// Pseudo flag for display
		eIsExpanded =				1L << 30,		// Pseudo flag for display
		eHasExpanded =				1L << 31		// Pseudo flag for display
	};

	enum EViewMode
	{
		eViewMode_All,
		eViewMode_AllMatched,
		eViewMode_ShowMatch
	};
}

typedef std::vector<std::pair<unsigned long, unsigned long> > threadvector;

#endif
