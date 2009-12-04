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


// Header for CToggleIconButton class

#ifndef __CTOGGLEICONBUTTON__MULBERRY__
#define __CTOGGLEICONBUTTON__MULBERRY__

#include <LBevelButton.h>

// Constants

// Classes
class CToggleIconButton : public LBevelButton
{
public:
	enum { class_ID = 'Tbtn' };
	
						CToggleIconButton(LStream *inStream);
	virtual				~CToggleIconButton();

	virtual	void		SetValue(SInt32 inValue);		// Prevent crash when deleting

protected:
	ResIDT				mNormIconSuiteID;			//	Resource ID of normal icon suite
	ResIDT				mPushIconSuiteID;			//	Resource ID of pushed icon suite
};

#endif
