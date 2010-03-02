/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CTabController

// Class that implements a tab control and manages its panels as well

#include "CTabController.h"

#include "CMulberryCommon.h"
#include "CTabPanel.h"

#include "cdstring.h"

BEGIN_MESSAGE_MAP(CTabController, CTabCtrl)
	//{{AFX_MSG_MAP(CTabController)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTabController::CTabController()
{
	mCurrentPanel = nil;
}

CTabController::~CTabController()
{
	// Destroy all panels
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		DestroyPanel(*iter);
}

// Resize sub-views
void CTabController::OnSize(UINT nType, int cx, int cy)
{
	CTabCtrl::OnSize(nType, cx, cy);
	
	// Resize each panel to same as this
	CRect rect;
	GetWindowRect(rect);
	GetParent()->ScreenToClient(rect);
	AdjustRect(false, rect);
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->MoveWindow(rect);
}

unsigned long CTabController::AddPanel(CTabPanel* aPanel, bool expand)
{
	mPanels.push_back(aPanel);
	InstallPanel(aPanel);
	if (expand)
		::ExpandChildToFit(this, aPanel, true, true);
	return 0;
}

void CTabController::RemovePanel(CTabPanel* aPanel)
{
	CTabPanelList::iterator found = std::find(mPanels.begin(), mPanels.end(), aPanel);
	
	if (found != mPanels.end())
	{
		DestroyPanel(aPanel);
		int item = found - mPanels.begin();
		mPanels.erase(found);
		DeleteItem(item);
	}
}

void CTabController::RemovePanel(unsigned long index)
{
	RemovePanel(mPanels.at(index));
}

// Install panel
void CTabController::SetPanelTitle(unsigned long index, const char* title)
{
	// Insert item first before getting display area
	CString _temp = cdstring(title).win_str();
	TC_ITEM tc;
	tc.mask = TCIF_TEXT;
	tc.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
	SetItem(index, &tc);
}

// Do DDX/DDV exchange
void CTabController::DoDataExchange(CDataExchange* pDX)
{
	// Update all panel data
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->UpdateData(pDX->m_bSaveAndValidate);
}

// Set data
void CTabController::SetContent(void* data)
{
	// Set panel data
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetContent(data);
		(*iter)->UpdateData(FALSE);	// Force CDialog to update items
	}
}

// Force update of data
bool CTabController::UpdateContent(void* data)
{
	// Update panel data (DDX/DDV already called)
	bool result = false;
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		result |= (*iter)->UpdateContent(data);
	
	return result;
}

// Force update of display panel
void CTabController::SetPanel(long index)
{
	if (mCurrentPanel)
	{
		mCurrentPanel->ShowWindow(SW_HIDE);
		mCurrentPanel->EnableWindow(false);
		mCurrentPanel->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
	
	if (index >= 0)
	{
		mCurrentPanel = mPanels.at(index);
		mCurrentPanel->SetWindowPos(GetNextWindow(GW_HWNDPREV), 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		mCurrentPanel->EnableWindow(true);
		mCurrentPanel->ShowWindow(SW_SHOW);
	}
	else
		mCurrentPanel = nil;
}

// Install panel
void CTabController::InstallPanel(CTabPanel* aPanel)
{
	// Insert item first before getting display area
	CString _temp;
	TC_ITEM tc;
	tc.mask = TCIF_TEXT;
	tc.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
	InsertItem(mPanels.size() - 1, &tc);

	// Get display area
	CRect rect;
	GetWindowRect(rect);
	GetParent()->ScreenToClient(rect);
	AdjustRect(false, rect);

	// Must create the panel here
	aPanel->Create(rect, GetParent(), IDC_STATIC, nil);

	// Use title as tab title (must do after Create)
	CString panel_title;
	aPanel->GetWindowText(panel_title);

	// Set tab item text
	tc.mask = TCIF_TEXT;
	tc.pszText = const_cast<TCHAR*>((const TCHAR*) panel_title);
	SetItem(mPanels.size() - 1, &tc);
	
	// Make sure its disabled and hidden
	aPanel->ShowWindow(SW_HIDE);
	aPanel->EnableWindow(false);
}

// Destroy panel
void CTabController::DestroyPanel(CTabPanel* aPanel)
{
	aPanel->DestroyWindow();
	delete aPanel;
}
