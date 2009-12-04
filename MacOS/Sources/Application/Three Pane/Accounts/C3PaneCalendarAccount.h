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


// Header for C3PaneCalendarAccount class

#ifndef __C3PANECALENDARACCOUNT__MULBERRY__
#define __C3PANECALENDARACCOUNT__MULBERRY__

#include "C3PaneViewPanel.h"

// Constants
const	PaneIDT		paneid_3PaneCalendarAccount = 1713;

// Messages

// Resources

// Classes
class CCalendarStoreView;

class C3PaneCalendarAccount : public C3PaneViewPanel
{
public:
	enum { class_ID = '3PND' };

					C3PaneCalendarAccount();
					C3PaneCalendarAccount(LStream *inStream);
	virtual 		~C3PaneCalendarAccount();

	virtual CBaseView*	GetBaseView() const;
	CCalendarStoreView*	GetCalendarStoreView() const
		{ return mCalendarStoreView; }

protected:
	CCalendarStoreView*	mCalendarStoreView;

	virtual void	FinishCreateSelf(void);
};

#endif
