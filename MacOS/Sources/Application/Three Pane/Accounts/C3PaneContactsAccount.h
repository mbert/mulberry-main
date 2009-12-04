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


// Header for C3PaneContactsAccount class

#ifndef __C3PANECONTACTSACCOUNT__MULBERRY__
#define __C3PANECONTACTSACCOUNT__MULBERRY__

#include "C3PaneViewPanel.h"

// Constants
const	PaneIDT		paneid_3PaneContactsAccount = 1712;

// Messages

// Resources

// Classes
class CAdbkManagerView;

class C3PaneContactsAccount : public C3PaneViewPanel
{
public:
	enum { class_ID = '3PNC' };

					C3PaneContactsAccount();
					C3PaneContactsAccount(LStream *inStream);
	virtual 		~C3PaneContactsAccount();

	virtual CBaseView*	GetBaseView() const;
	CAdbkManagerView*	GetAdbkManagerView() const
		{ return mContactsView; }

protected:
	CAdbkManagerView*	mContactsView;

	virtual void	FinishCreateSelf(void);
};

#endif
