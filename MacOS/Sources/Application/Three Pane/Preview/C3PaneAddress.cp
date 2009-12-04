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
#include "CAddressView.h"
#include "CGroup.h"
#include "CResources.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneAddress::C3PaneAddress()
{
}

// Constructor from stream
C3PaneAddress::C3PaneAddress(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
}

// Default destructor
C3PaneAddress::~C3PaneAddress()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneAddress::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mAddressView = (CAddressView*) FindPaneByID(paneid_3PaneView);
	
	// Hide it until an address is set
	Hide();
}

CBaseView* C3PaneAddress::GetBaseView() const
{
	return mAddressView;
}

bool C3PaneAddress::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneAddress::IsSpecified() const
{
	return (mAddressView->GetAddress() != NULL) ||
			(mAddressView->GetGroup() != NULL);
}

void C3PaneAddress::SetAddress(CAddressBook* adbk, CAdbkAddress* addr)
{
	// Give it to address view to display it
	mAddressView->SetAddress(adbk, addr);
}

void C3PaneAddress::SetGroup(CAddressBook* adbk, CGroup* grp)
{
	// Give it to address view to display it
	mAddressView->SetGroup(adbk, grp);
}

cdstring C3PaneAddress::GetTitle() const
{
	CAdbkAddress* addr = (mAddressView ? mAddressView->GetAddress() : NULL);
	CGroup* grp = (mAddressView ? mAddressView->GetGroup() : NULL);
	if (addr)
		return addr->GetName();
	else if (grp)
		return grp->GetName();
	else
		return cdstring::null_str;
}

ResIDT C3PaneAddress::GetIconID() const
{
	CAdbkAddress* addr = (mAddressView ? mAddressView->GetAddress() : NULL);
	CGroup* grp = (mAddressView ? mAddressView->GetGroup() : NULL);
	if (addr || !grp)
		return ICNx_3PanePreviewAddress;
	else
		return ICNx_3PanePreviewGroup;
}
