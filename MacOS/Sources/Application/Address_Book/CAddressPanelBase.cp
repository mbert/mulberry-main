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


// Source for CAddressPanelBase class

#include "CAddressPanelBase.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressPanelBase::CAddressPanelBase(LStream *inStream)
		: CTabPanel(inStream)
{
	mAddress = NULL;
}

// Default destructor
CAddressPanelBase::~CAddressPanelBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CAddressPanelBase::SetData(void* data)
{
	mAddress = reinterpret_cast<CAdbkAddress*>(data);

	// Set the data in the panel
	SetFields(mAddress);
}

bool CAddressPanelBase::UpdateData(void* data)
{
	// Read data from panel
	return GetFields(reinterpret_cast<CAdbkAddress*>(data));
}
