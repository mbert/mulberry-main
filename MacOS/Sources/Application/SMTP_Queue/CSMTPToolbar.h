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


// Header for CSMTPToolbar class

#ifndef __CSMTPTOOLBAR__MULBERRY__
#define __CSMTPTOOLBAR__MULBERRY__

#include "CToolbar.h"

// Constants
const	PaneIDT		paneid_SMTPToolbar1 = 1755;

// Resources

// Classes
class CSMTPToolbar : public CToolbar
{
	friend class CSMTPView;

public:
	enum { class_ID = 'TB3o' };

	enum
	{
		eBroadcast_SecureState = 'otss'
	};

					CSMTPToolbar();
					CSMTPToolbar(LStream *inStream);
	virtual 		~CSMTPToolbar();

	virtual void	ListenTo_Message(long msg, void* param);

protected:
	virtual void	FinishCreateSelf(void);
};

#endif
