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


// Header for Message class

#ifndef __CMESSAGEFWD__MULBERRY__
#define __CMESSAGEFWD__MULBERRY__

#include "cdstring.h"

namespace NMessage
{
	enum EFlags
	{
		eNone = 		0,
		
		// IMAP flags
		eRecent = 			1L << 0,
		eAnswered = 		1L << 1,
		eFlagged = 			1L << 2,
		eDeleted = 			1L << 3,
		eSeen = 			1L << 4,
		eDraft = 			1L << 5,
		eMDNSent =			1L << 6,
		ePartial =			1L << 7,			// Partially cached message
		eError =			1L << 8,			// Local message in error state
		
		// Labels
		eLabel1 =			1L << 12,
		eLabel2 =			1L << 13,
		eLabel3 =			1L << 14,
		eLabel4 =			1L << 15,
		eLabel5 =			1L << 16,
		eLabel6 =			1L << 17,
		eLabel7 =			1L << 18,
		eLabel8 =			1L << 19,
		eMaxLabels =		8,

		eUserFlags = eAnswered | eFlagged | eDeleted | eSeen | eDraft,
		eLabels = eLabel1 | eLabel2 | eLabel3 | eLabel4 | eLabel5 | eLabel6 | eLabel7 | eLabel8,
		eIMAPFlags = eRecent | eAnswered | eFlagged | eDeleted | eSeen | eDraft | eMDNSent | eLabels,
		eLocalFlags = eRecent | eAnswered | eFlagged | eDeleted | eSeen | eDraft | eMDNSent | ePartial | eError | eLabels,
		eServerFlags = eRecent | eAnswered | eFlagged | eDeleted | eSeen | eDraft | eMDNSent | ePartial | eError | eLabels,
		eAllPermanent = eAnswered | eFlagged | eDeleted | eSeen | eDraft | eMDNSent | ePartial | eLabels,
		
		// SMTP Flags
		eSendingNow =		eAnswered,
		eHold = 			eMDNSent,
		ePriority =			eFlagged,
		eSendError =		eError,

		// Pseudo flags
		eCheckRecent = 		1L << 20,			// Indicates new message found during a CHECK
		eMboxRecent = 		1L << 21,			// Indicates an unseen new message since mbox opened
		eSearch = 			1L << 22,			// Indicates a message match a search
		eFullLocal =		1L << 23,			// Messages completely cached locally
		ePartialLocal =		1L << 24,			// Message partially cached locally

		// Smart address flags
		eSmartFrom = 		1L << 25,
		eSmartTo =	 		1L << 26,
		eSmartCC =	 		1L << 27,
		eSmartCheck = 		1L << 28,

		// State flags
		eHasText = 			1L << 29,
		eHasTextChecked =	1L << 30,
		
		eFake =				1L << 31			// Fake message used for threading
	};
	
	enum EReplyType
	{
		eReplyTo = 0,
		eReplySender,
		eReplyFrom,
		eReplyAll
	};
	
	enum EDraftType
	{
		eDraftNew = 0,
		eDraftReply,
		eDraftForward,
		eDraftBounce,
		eDraftSendAgain,
		eDraftMDN
	};

	struct SAddressing
	{
		cdstring mTo;
		cdstring mCC;
		cdstring mBcc;
		
		SAddressing() {}
		SAddressing(const SAddressing& copy)
			{ mTo = copy.mTo; mCC = copy.mCC; mBcc = copy.mBcc; }
	};
}

#endif
