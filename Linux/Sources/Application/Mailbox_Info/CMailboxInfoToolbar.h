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


// Header for CMailboxInfoToolbar class

#ifndef __CMAILBOXINFOTOOLBAR__MULBERRY__
#define __CMAILBOXINFOTOOLBAR__MULBERRY__

#include "CToolbar.h"

// Classes
class CMailboxToolbarPopup;

class CMailboxInfoToolbar : public CToolbar
{
	friend class CMailboxInfoView;

public:
	enum
	{
		eBroadcast_IconState = 'mtis'
	};

	CMailboxInfoToolbar(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual 		~CMailboxInfoToolbar();

	virtual void	OnCreate();

	virtual void	ListenTo_Message(long msg, void* param);

	CMailboxToolbarPopup*	GetCopyBtn() const;
};

#endif
