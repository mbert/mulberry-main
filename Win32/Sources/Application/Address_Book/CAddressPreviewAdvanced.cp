/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Source for CAddressPreviewAdvanced class

#include "CAddressPreviewAdvanced.h"

#include "CAdbkAddress.h"
#include "CAddressBook.h"
#include "CMulberryApp.h"
#include "CUnicodeUtils.h"
#include "C3PaneWindow.h"

#include "resource1.h"

// Static members

BEGIN_MESSAGE_MAP(CAddressPreviewAdvanced, CAddressPreviewBase)
	ON_WM_CREATE()
	ON_WM_SIZE()

	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnZoomPane)
END_MESSAGE_MAP()

// Default constructor
CAddressPreviewAdvanced::CAddressPreviewAdvanced()
{
	mAddress = NULL;
}

// Default destructor
CAddressPreviewAdvanced::~CAddressPreviewAdvanced()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CAddressPreviewAdvanced::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CAddressPreviewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Get window that owns this one
	CWnd* super = GetParent();
	while(super && !dynamic_cast<CView*>(super))
		super = super->GetParent();

	bool is3Pane = (super == C3PaneWindow::s3PaneWindow);

	const int cSmallIconBtnSize = 20;
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Header with alignment
	const int cCaptionHeight = 16;
	const int cHeaderHeight = cCaptionHeight + 8;
	const int cBtnVertOffset = (cHeaderHeight - cSmallIconBtnSize) / 2;
	mHeader.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width - cSmallIconBtnSize - 8, cHeaderHeight), this, IDC_STATIC);
	mHeader.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_TopWidth));

	// Icon and descriptor
	mIcon.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 16, 16), &mHeader, IDC_STATIC);
	mIcon.SetIconID(IDI_3PANEPREVIEWADDRESS);
	CRect rect;
	mHeader.GetClientRect(rect);
	rect.left += 24;
	mDescriptor.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mDescriptor, CWndAlignment::eAlign_WidthHeight));

	CString s;
	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), this, IDC_3PANETOOLBAR_ZOOMPREVIEW, 0, IDI_3PANE_ZOOM);
	AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);
	
	// No zoom/close if not 3-pane
	if (!is3Pane)
		mZoom.ShowWindow(SW_HIDE);

	// Tabs with alignment
	mFields.Create(CRect(0, cHeaderHeight, width, height), this);
	AddAlignment(new CWndAlignment(&mFields, CWndAlignment::eAlign_WidthHeight));

	return 0;
}

// Resize sub-views
void CAddressPreviewAdvanced::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CWnd::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

void CAddressPreviewAdvanced::Close(void)
{
	// If it exists and its changed, update it
	if (mAdbk && mAddress && GetFields(mAddress))
	{
		// Need to set address to NULL before the update as the update
		// will cause a refresh and make this close again
		CAdbkAddress* temp = mAddress;
		mAddress = NULL;
		mAdbk->UpdateAddress(temp, true);
	}
}

void CAddressPreviewAdvanced::Focus()
{
	//mFields.Focus();
}

void CAddressPreviewAdvanced::OnZoomPane()
{
	C3PaneWindow::s3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMPREVIEW);
}

void CAddressPreviewAdvanced::SetAddress(CAdbkAddress* addr)
{
	// Add to each panel
	mAddress = addr;
	cdstring title;
	if (addr)
		title = addr->GetName();
	CUnicodeUtils::SetWindowTextUTF8(mDescriptor, title);
	SetFields(addr);
}

// Address removed by someone else
void CAddressPreviewAdvanced::ClearAddress()
{
	mAddress = NULL;
	SetAddress(NULL);
}

// Set fields in dialog
void CAddressPreviewAdvanced::SetFields(const CAdbkAddress* addr)
{
    mFields.SetAddress(addr);
}

// Get fields from dialog
bool CAddressPreviewAdvanced::GetFields(CAdbkAddress* addr)
{
	// Nothing to do right now
    return mFields.GetAddress(addr);
}
