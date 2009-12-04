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


// CPrefsSubTab

#include "CPrefsSubTab.h"

#include "CPrefsTabSubPanel.h"

// Constructor from stream
CPrefsSubTab::CPrefsSubTab(LStream *inStream)
		: CTabs(inStream)
{
	mCurrentPanel = nil;
}

// Default destructor
CPrefsSubTab::~CPrefsSubTab()
{
	// Remove all sub-panes (must remove from place holder first to prevent double delete)
	RemovePanel();
	for(std::vector<CPrefsTabSubPanel*>::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		delete *iter;
}

void CPrefsSubTab::AddPanel(unsigned long id)
{
	// Make panel area default so new panel is automatically added to it
	SetDefaultView(this);
	CPrefsTabSubPanel* panel = (CPrefsTabSubPanel*) UReanimator::ReadObjects('PPob', id);
	panel->FinishCreate();
	panel->Hide();
	panel->Disable();
	mPanels.push_back(panel);
}

void CPrefsSubTab::ChangePanel(unsigned long id, unsigned long index)
{
	// Make panel area default so new panel is automatically added to it
	SetDefaultView(this);
	CPrefsTabSubPanel* panel = (CPrefsTabSubPanel*) UReanimator::ReadObjects('PPob', id);
	panel->FinishCreate();
	panel->Hide();
	panel->Disable();
	if (mCurrentPanel == mPanels[index - 1])
		mCurrentPanel = nil;
	delete mPanels[index - 1];
	mPanels[index - 1] = panel;

	SetDisplayPanel(index);
}

// Toggle display of IC
void CPrefsSubTab::ToggleICDisplay(bool IC_on)
{
	// Set sub-panes
	for(std::vector<CPrefsTabSubPanel*>::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->ToggleICDisplay(IC_on);
}

// Set data
void CPrefsSubTab::SetData(void* data)
{
	// Set sub-panes
	for(std::vector<CPrefsTabSubPanel*>::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->SetData(data);
}

// Force update of data
void CPrefsSubTab::UpdateData(void* data)
{
	// Update sub-panes
	for(std::vector<CPrefsTabSubPanel*>::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
		(*iter)->UpdateData(data);
}

// Set auth panel
void CPrefsSubTab::SetDisplayPanel(unsigned long index)
{
	// Update any existing panel and remove it
	RemovePanel();
	if (mCurrentPanel)
	{
		mCurrentPanel->Hide();
		mCurrentPanel->Disable();
		//DrawSelf();
	}

	mCurrentPanel = mPanels.at(index - 1);
	InstallPanel(mCurrentPanel);
	mCurrentPanel->Enable();
	mCurrentPanel->Show();
	//DrawSelf();
}
