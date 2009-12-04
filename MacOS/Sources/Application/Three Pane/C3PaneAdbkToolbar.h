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


// Header for C3PaneAdbkToolbar class

#ifndef __C3PANEADBKTOOLBAR__MULBERRY__
#define __C3PANEADBKTOOLBAR__MULBERRY__

#include "CToolbar.h"

// Constants
const	PaneIDT		paneid_3PaneAdbkToolbar = 1751;

// Resources
const	ResIDT		RidL_C3PaneAdbkToolbar = 1751;

// Classes
class LBevelButton;

class C3PaneAdbkToolbar : public CToolbar
{
public:
	enum { class_ID = 'TB32' };

					C3PaneAdbkToolbar();
					C3PaneAdbkToolbar(LStream *inStream);
	virtual 		~C3PaneAdbkToolbar();

protected:

	virtual void	FinishCreateSelf(void);
};

#endif
