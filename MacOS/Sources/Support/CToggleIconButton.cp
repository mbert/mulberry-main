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


// Source for CToggleIconButton class

#include "CToggleIconButton.h"


// __________________________________________________________________________________________________
// C L A S S __ C 3 D I C O N B U T T O N
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CToggleIconButton::CToggleIconButton(LStream *inStream)
	: LBevelButton(inStream)
{
	*inStream >> mNormIconSuiteID;
	*inStream >> mPushIconSuiteID;
}

// Default destructor
CToggleIconButton::~CToggleIconButton()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set value AFTER all other changes to prevent delete error
void CToggleIconButton::SetValue(SInt32 inValue)
{
	// Only change the value if we have too
	if (mValue != inValue)
	{
		// Set the icon resource id
		ControlButtonContentInfo controlInfo;
		controlInfo.contentType = kControlContentIconSuiteRes;
		controlInfo.u.resID = inValue ? mPushIconSuiteID : mNormIconSuiteID;
		SetContentInfo(controlInfo);

		// Get the value setup last
		LBevelButton::SetValue(inValue);
	}
}
