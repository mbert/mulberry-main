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


BEGIN_MESSAGE_MAP(CAddressPanelBase, CTabPanel)
	//{{AFX_MSG_MAP(CAddressPersonalPanel)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Constructor from stream
CAddressPanelBase::CAddressPanelBase(UINT nIDTemplate)
	: CTabPanel(nIDTemplate)
{
	mAddress = NULL;
}

// Default destructor
CAddressPanelBase::~CAddressPanelBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CAddressPanelBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create alignment details
	CRect rect;
	GetWindowRect(rect);
	InitResize(rect.Width(), rect.Height());

	return 0;
}

// Set data
BOOL CAddressPanelBase::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	return TRUE;
}

// Resize sub-views
void CAddressPanelBase::OnSize(UINT nType, int cx, int cy)
{
	CTabPanel::OnSize(nType, cx, cy);
	
	// Do alignment changes
	SizeChanged(cx, cy);
}

void CAddressPanelBase::SetContent(void* data)
{
	mAddress = reinterpret_cast<CAdbkAddress*>(data);

	// Set the data in the panel
	SetFields(mAddress);
}

bool CAddressPanelBase::UpdateContent(void* data)
{
	// Read data from panel
	return GetFields(reinterpret_cast<CAdbkAddress*>(data));
}
