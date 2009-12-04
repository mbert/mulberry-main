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


// Header for CUserActionOptions class

#ifndef __CUSERACTIONOPTIONS__MULBERRY__
#define __CUSERACTIONOPTIONS__MULBERRY__

#include <JXWidgetSet.h>

#include "CUserAction.h"

// Classes
class JXStaticText;
class JXTextCheckbox;
class JXTextButton;

class CUserActionOptions : public JXWidgetSet
{
public:
					CUserActionOptions(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CUserActionOptions();

	virtual void	OnCreate();					// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	void	SetData(const CUserAction& action);
	void	GetData(CUserAction& action);
	void	DisableItems();

protected:
// begin JXLayout1

    JXTextCheckbox* mSelect;
    JXTextCheckbox* mSingleClick;
    JXTextCheckbox* mDoubleClick;
    JXTextCheckbox* mUseKey;
    JXStaticText*   mKey;
    JXTextButton*   mChooseBtn;

// end JXLayout1

	unsigned char		mActualKey;
	CKeyModifiers		mActualMods;

	void GetKey();
};

#endif
