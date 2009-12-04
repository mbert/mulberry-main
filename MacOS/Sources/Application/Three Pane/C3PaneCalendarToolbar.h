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


// Header for C3PaneCalendarToolbar class

#ifndef __C3PANECALENDARTOOLBAR__MULBERRY__
#define __C3PANECALENDARTOOLBAR__MULBERRY__

#include "CToolbar.h"

// Constants
const	PaneIDT		paneid_3PaneCalendarToolbar = 1753;

// Resources
const	ResIDT		RidL_C3PaneCalendarToolbar = 1753;

// Classes
class C3PaneCalendarToolbar : public CToolbar
{
public:
	enum { class_ID = 'TB33' };

					C3PaneCalendarToolbar();
					C3PaneCalendarToolbar(LStream *inStream);
	virtual 		~C3PaneCalendarToolbar();

protected:

	virtual void	FinishCreateSelf(void);
};

#endif
