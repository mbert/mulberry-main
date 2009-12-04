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


// CTabController

#include "CTabController.h"

#include "CTabPanel.h"

#include <LGATabsControlImp.h>
#include <LMultiPanelView.h>

// Constructor from stream
CTabController::CTabController(LStream *inStream) : LTabsControl(inStream)
{
	mPanels = NULL;
}

// Default destructor
CTabController::~CTabController()
{
}

void CTabController::FinishCreateSelf()
{
	// Do inherited
	LTabsControl::FinishCreateSelf();

	// Get panel host
	mPanels = (LMultiPanelView*) FindPaneByID('PLAC');
}

void CTabController::AddPanel(unsigned long id)
{
	mPanels->AddPanel(id, NULL, LArray::index_Last);
	mPanels->CreatePanel(mPanels->GetPanelCount());
	GetPanel(mPanels->GetPanelCount())->Hide();
}

CTabPanel* CTabController::GetPanel(unsigned long index)
{
	return static_cast<CTabPanel*>(mPanels->GetPanel(index));
}

CTabPanel* CTabController::GetCurrentPanel()
{
	return static_cast<CTabPanel*>(mPanels->GetCurrentPanel());
}

void CTabController::ChangePanel(unsigned long id, unsigned long index)
{
	mPanels->SetPanel(id, NULL, index, true);
	mPanels->CreatePanel(index);
	GetPanel(index)->Hide();
}

// Set data
void CTabController::SetData(void* data)
{
	// Set sub-panes
	for(int i = 1; i <= mPanels->GetPanelCount(); i++)
		static_cast<CTabPanel*>(mPanels->GetPanel(i))->SetData(data);
	
	// Show first panel if non currently visible
	if (!mPanels->GetCurrentIndex())
		mPanels->SwitchToPanel(1);
}

// Force update of data
bool CTabController::UpdateData(void* data)
{
	// Update sub-panes
	bool result = false;
	for(int i = 1; i <= mPanels->GetPanelCount(); i++)
		result |= static_cast<CTabPanel*>(mPanels->GetPanel(i))->UpdateData(data);
	
	return result;
}

// Set auth panel
void CTabController::SetDisplayPanel(unsigned long index)
{
	// Change to panel
	mPanels->SwitchToPanel(index);
}

void CTabController::RemoveTabButtonAt(ArrayIndexT inAtIndex)
{
	if (UEnvironment::HasFeature (env_HasAppearance))
	{
		// get Max value
		SInt32 outMaxValue = GetMaxValue();
		
		// Move info for buttons at higher pos down by one
		for(SInt32 i = inAtIndex; i < outMaxValue; i++)
		{
			ControlTabInfoRec info;
			info.version = kControlTabInfoVersionZero;
			(mControlSubPane->GetControlImp())->GetDataTag(i + 1, kControlTabInfoTag, sizeof(ControlTabInfoRec), &info, NULL);
			(mControlSubPane->GetControlImp())->SetDataTag(i, kControlTabInfoTag, sizeof(ControlTabInfoRec), &info);
		}
		
		// Reset maximum to remove button
		SetMaxValue(outMaxValue - 1);
	}
	else
	{
		static_cast<LGATabsControlImp*>((mControlSubPane->GetControlImp()))->RemoveTabButtonAt(inAtIndex);
	}
}