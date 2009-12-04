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

#ifndef __C3PaneCalendarToolbar__MULBERRY__
#define __C3PaneCalendarToolbar__MULBERRY__

#include "CToolbar.h"

// Classes
class C3PaneCalendarToolbar : public CToolbar
{
public:
					C3PaneCalendarToolbar(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~C3PaneCalendarToolbar();

	virtual void	OnCreate();
};

#endif
