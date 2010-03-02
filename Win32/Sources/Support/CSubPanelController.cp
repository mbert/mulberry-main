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


// CSubPanelController

// Class that implements a tab control and manages its panels as well

#include "CSubPanelController.h"

#include "CTabPanel.h"

#include "cdstring.h"

CSubPanelController::CSubPanelController()
{
	mCurrentPanel = nil;
	mCurrentIndex = -1;
}

CSubPanelController::~CSubPanelController()
{
	// Destroy all panels
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		DestroyPanel(*iter);
}

unsigned long CSubPanelController::AddPanel(CTabPanel* aPanel)
{
	mPanels.push_back(aPanel);
	InstallPanel(aPanel);
	return 0;
}

void CSubPanelController::RemovePanel(CTabPanel* aPanel)
{
	CTabPanelList::iterator found = std::find(mPanels.begin(), mPanels.end(), aPanel);
	
	if (found != mPanels.end())
	{
		DestroyPanel(aPanel);
		mPanels.erase(found);
	}
}

void CSubPanelController::RemovePanel(unsigned long index)
{
	DestroyPanel(mPanels.at(index));
	mPanels.erase(mPanels.begin() + index);
}

// Do DDX/DDV exchange
void CSubPanelController::DoDataExchange(CDataExchange* pDX)
{
	// Update all panel data
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->UpdateData(pDX->m_bSaveAndValidate);
}

// Set data
void CSubPanelController::SetContent(void* data)
{
	// Set panel data
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetContent(data);
		(*iter)->UpdateData(FALSE);	// Force CDialog to update items
	}
}

// Force update of data
void CSubPanelController::UpdateContent(void* data)
{
	// Update panel data (DDX/DDV already done)
	for(CTabPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->UpdateContent(data);
}

// Set data
void CSubPanelController::SetPanelContent(void* data)
{
	// Set panel data
	if (mCurrentPanel)
	{
		mCurrentPanel->SetContent(data);
		mCurrentPanel->UpdateData(FALSE);	// Force CDialog to update items
	}
}

// Force update of data
void CSubPanelController::UpdatePanelContent(void* data)
{
	// Update panel data
	if (mCurrentPanel)
	{
		mCurrentPanel->UpdateData(TRUE);	// Force CDialog to update items
		mCurrentPanel->UpdateContent(data);
	}
}

// Force update of display panel
void CSubPanelController::SetPanel(long index)
{
	// This window is always hidden
	ShowWindow(SW_HIDE);

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
	
	mCurrentIndex = index;
}

// Install panel
void CSubPanelController::InstallPanel(CTabPanel* aPanel)
{
	// Get display area
	CRect rect;
	GetWindowRect(rect);
	GetParent()->ScreenToClient(rect);

	// Must create the panel here
	aPanel->Create(rect, GetParent(), IDC_STATIC, nil);

	// Make sure its disabled and hidden
	aPanel->ShowWindow(SW_HIDE);
	aPanel->EnableWindow(false);
}

// Destroy panel
void CSubPanelController::DestroyPanel(CTabPanel* aPanel)
{
	aPanel->DestroyWindow();
	delete aPanel;
}
