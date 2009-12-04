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


// Source for CPickPopup class

#include "CPickPopup.h"

#include <LControlImp.h>

// __________________________________________________________________________________________________
// C L A S S __ C S I Z E P O P U P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CPickPopup::CPickPopup(LStream *inStream)
		: LPopupButton(inStream)
{
}

// Default destructor
CPickPopup::~CPickPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set size
void CPickPopup::SetValue(SInt32 inValue)
{
	// Do default
	LPopupButton::SetValue(inValue);

	// Force value off
	SetMinValue(0);
	mValue = 0;
	mControlImp->SetValue(0);
}
