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


// Header for CMessageList class

#ifndef __CMESSAGELISTFWD__MULBERRY__
#define __CMESSAGELISTFWD__MULBERRY__

//namespace NMessage
//{
	// Types
	enum ESortMessageBy
	{
		cSortMessageTo = 1,
		cSortMessageFrom,
		cSortMessageReplyTo,
		cSortMessageSender,
		cSortMessageCc,
		cSortMessageSubject,
		cSortMessageThread,
		cSortMessageDateSent,
		cSortMessageDateReceived,
		cSortMessageSize,
		cSortMessageFlags,
		cSortMessageNumber,
		cSortMessageSmart,
		cSortMessageAttachment,
		cSortMessageParts,
		cSortMessageMatching,
		cSortMessageDisconnected,
		cSortMessageMax = cSortMessageDisconnected
	};

	enum EShowMessageBy
	{
		cShowMessageAscending = 1,
		cShowMessageDescending,
		cShowMessageMax = cShowMessageDescending
	};

	enum EThreadMessageBy
	{
		cThreadMessageAny = 0,
		cThreadMessageSubject,
		cThreadMessageReferences,
		cThreadMessageMax = cThreadMessageReferences
	};
//}
#endif
