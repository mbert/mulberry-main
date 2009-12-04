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


// Source for CSMTPToolbar class

#include "CSMTPToolbar.h"

#include "CSMTPAccountPopup.h"

BEGIN_MESSAGE_MAP(CSMTPToolbar, CToolbar)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPToolbar::CSMTPToolbar()
{
}

// Default destructor
CSMTPToolbar::~CSMTPToolbar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CSMTPToolbar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolbar::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetType(CToolbarManager::e1PaneSMTP);
	BuildToolbar();

	return 0;
}

void CSMTPToolbar::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	// Change icon visibility
	case eBroadcast_SecureState:
	{
		// Get account popup toolbar item if present
		CSMTPAccountPopup* popup = static_cast<CSMTPAccountPopup*>(GetItem(CToolbarManager::eToolbar_Accounts));
		if (popup != NULL)
			popup->GetSecureIndicator()->ShowWindow(param ? SW_SHOW : SW_HIDE);
		break;
	}

	default:
		CToolbar::ListenTo_Message(msg, param);
		break;
	}
}		
