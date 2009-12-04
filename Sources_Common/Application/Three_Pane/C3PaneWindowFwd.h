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


// Header for C3PaneWindow forward declarations class

#ifndef __C3PANEWINDOWFWD__MULBERRY__
#define __C3PANEWINDOWFWD__MULBERRY__

namespace N3Pane
{
	enum EPaneType
	{
		ePane_Empty = 0,
		ePane_List,
		ePane_Items,
		ePane_Preview
	};
	
	enum EGeometry
	{
		eNone = 0,

		eListVert,
		//			| mailbox
		//   server	|---------
		//			| message

		eListHoriz,
		//         server
		//  -------------------
		//	 mailbox | message

		eItemsVert,
		//   server  |
		//  ---------| mailbox
		//	 message |

		eItemsHoriz,
		//        mailbox
		//  -------------------
		//	 server  | message

		ePreviewVert,
		//   server  |
		//  ---------| message
		//	 mailbox |

		ePreviewHoriz,
		//   server  | mailbox
		//  -------------------
		//	      message

		eAllVert,
		//   server  | mailbox | message

		eAllHoriz
		//        server
		//  -------------------
		//        mailbox
		//  -------------------
		//	      message

	};
	
	enum EViewType
	{
		eView_Empty = 0,
		eView_Mailbox,
		eView_Contacts,
		eView_Calendar,
		//eView_IM,
		//eView_Bookmarks
		eView_Total
	};
}

#endif
