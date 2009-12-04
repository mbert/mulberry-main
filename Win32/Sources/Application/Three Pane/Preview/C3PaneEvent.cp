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


// Source for C3PaneEvent class

#include "C3PaneEvent.h"

#include "CMulberryApp.h"
#include "CUnicodeUtils.h"
#include "C3PaneWindow.h"

BEGIN_MESSAGE_MAP(C3PaneEvent, C3PanePanel)
	ON_WM_CREATE()
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnZoomPane)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneEvent::C3PaneEvent()
{
}

// Default destructor
C3PaneEvent::~C3PaneEvent()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneEvent::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneViewPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int cSmallIconBtnSize = 20;
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	CString s;

	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), this, IDC_3PANETOOLBAR_ZOOMPREVIEW, 0, IDI_3PANE_ZOOM);
	AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);

	// Header with alignment
	const int cCaptionHeight = 16;
	const int cHeaderHeight = cCaptionHeight + 8;
	const int cBtnVertOffset = (cHeaderHeight - cSmallIconBtnSize) / 2;
	mHeader.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(3, 0, width - cSmallIconBtnSize - 8, cHeaderHeight), this, IDC_STATIC);
	mHeader.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_TopWidth));

	// Icon and descriptor
	mIcon.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 16, 16), &mHeader, IDC_STATIC);
	mIcon.SetIconID(IDI_CALENDAR);
	CRect rect;
	mHeader.GetClientRect(rect);
	rect.left += 24;
	mDescriptor.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mDescriptor, CWndAlignment::eAlign_WidthHeight));

	// Create server view
	rect = CRect(0, cHeaderHeight, width, height);
	mEventView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mEventView, CWndAlignment::eAlign_WidthHeight));
	
	// Hide it until a mailbox is set
	ShowWindow(SW_HIDE);

	return 0;
}

CBaseView* C3PaneEvent::GetBaseView() const
{
	return const_cast<CEventPreview*>(&mEventView);
}

void C3PaneEvent::OnZoomPane()
{
	m3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMPREVIEW);
}

bool C3PaneEvent::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneEvent::IsSpecified() const
{
	return mEventView.HasData() != NULL;
}

void C3PaneEvent::SetComponent(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Give it to event view to display it
	mEventView.SetComponent(comp);
}

cdstring C3PaneEvent::GetTitle() const
{
	return mEventView.GetSummary();
}

void C3PaneEvent::SetTitle(const cdstring& title)
{
	CUnicodeUtils::SetWindowTextUTF8(&mDescriptor, title);
}

UINT C3PaneEvent::GetIconID() const
{
	return IDI_CALENDAR;
}
