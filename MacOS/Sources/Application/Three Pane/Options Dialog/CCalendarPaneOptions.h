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


// Header for CCalendarPaneOptions class

#ifndef __CCALENDARPANEOPTIONS__MULBERRY__
#define __CCALENDARPANEOPTIONS__MULBERRY__

#include "CCommonViewOptions.h"

#include "C3PaneOptions.h"

// Constants
const	PaneIDT		paneid_CalendarViewOptions = 1794;

// Messages

// Resources

// Classes
class CCalendarViewOptions;

class CCalendarPaneOptions : public CCommonViewOptions
{
public:
	enum { class_ID = 'CVop' };

					CCalendarPaneOptions(LStream *inStream);
	virtual 		~CCalendarPaneOptions();

	void	SetData(const CUserAction& listPreview,
					const CUserAction& listFullView,
					const CUserAction& itemsPreview,
					const CUserAction& itemsFullView,
					const CCalendarViewOptions& options,
					bool is3pane);
	void	GetData(CUserAction& listPreview,
					CUserAction& listFullView,
					CUserAction& itemsPreview,
					CUserAction& itemsFullView,
					CCalendarViewOptions& options);

protected:
	virtual void	FinishCreateSelf(void);
};

#endif
