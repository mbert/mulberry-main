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


// Header for CPrefsSIEVEAccount class

#ifndef __CPREFSSIEVEACCOUNT__MULBERRY__
#define __CPREFSSIEVEACCOUNT__MULBERRY__

#include "CPrefsAccountPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsSIEVEAccount = 5039;

// Mesages

// Resources

// Classes
class CPrefsSIEVEAccount : public CPrefsAccountPanel
{
public:
	enum { class_ID = 'Acsi' };

					CPrefsSIEVEAccount();
					CPrefsSIEVEAccount(LStream *inStream);
	virtual 		~CPrefsSIEVEAccount();

protected:
	virtual void	InitTabs();						// Do odds & ends
};

#endif
