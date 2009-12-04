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


// Source for C3PaneAddress class

#include "C3PaneAddress.h"

#include "CAdbkAddress.h"
#include "CGroup.h"

BEGIN_MESSAGE_MAP(C3PaneAddress, C3PanePanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneAddress::C3PaneAddress()
{
}

// Default destructor
C3PaneAddress::~C3PaneAddress()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneAddress::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneViewPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create server view
	CRect rect = CRect(0, 0, width, height);
	mAddressView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mAddressView, CWndAlignment::eAlign_WidthHeight));
	
	// Hide it until a mailbox is set
	ShowWindow(SW_HIDE);

	return 0;
}

CBaseView* C3PaneAddress::GetBaseView() const
{
	return const_cast<CAddressView*>(&mAddressView);
}

bool C3PaneAddress::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneAddress::IsSpecified() const
{
	return (mAddressView.GetAddress() != NULL) ||
			(mAddressView.GetGroup() != NULL);
}

void C3PaneAddress::SetAddress(CAddressBook* adbk, CAdbkAddress* addr)
{
	// Give it to address view to display it
	mAddressView.SetAddress(adbk, addr);
}

void C3PaneAddress::SetGroup(CAddressBook* adbk, CGroup* grp)
{
	// Give it to address view to display it
	mAddressView.SetGroup(adbk, grp);
}

cdstring C3PaneAddress::GetTitle() const
{
	CAdbkAddress* addr = mAddressView.GetAddress();
	CGroup* grp = mAddressView.GetGroup();
	if (addr)
		return addr->GetName();
	else if (grp)
		return grp->GetName();
	else
		return cdstring::null_str;
}

UINT C3PaneAddress::GetIconID() const
{
	CAdbkAddress* addr = mAddressView.GetAddress();
	CGroup* grp = mAddressView.GetGroup();
	if (addr || !grp)
		return IDI_3PANEPREVIEWADDRESS;
	else
		return IDI_3PANEPREVIEWGROUP;
}
