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


// Source for C3PaneContactsAccount class

#include "C3PaneContactsAccount.h"

#include "CAdbkManagerTable.h"
#include "CAddressBookManager.h"

BEGIN_MESSAGE_MAP(C3PaneContactsAccount, C3PanePanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneContactsAccount::C3PaneContactsAccount()
{
}

// Default destructor
C3PaneContactsAccount::~C3PaneContactsAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneContactsAccount::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneViewPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create server view
	CRect rect = CRect(0, 0, width, height);
	mContactsView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mContactsView, CWndAlignment::eAlign_WidthHeight));
	mContactsView.GetTable()->SetManager(CAddressBookManager::sAddressBookManager);
	mContactsView.ResetState();
	mContactsView.ResetTable();

	return 0;
}

CBaseView* C3PaneContactsAccount::GetBaseView() const
{
	return const_cast<CAdbkManagerView*>(&mContactsView);
}
