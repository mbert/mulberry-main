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

// Class that implements a tab control and manages its panels as well

#include "CTabController.h"

#include "CTabPanel.h"

CTabController::CTabController(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h)
	: JXTabs(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Add a card of our type
void CTabController::AppendCard(CTabPanel* card, const char* title)
{
	JXTabs::AppendCard(card, title);
	card->OnCreate();
}

// Set data
void CTabController::SetData(void* data)
{
	// Set panel data
	for(JIndex index = 1; index <= GetTabCount(); index++)
		static_cast<CTabPanel*>(mCards->GetCard(index))->SetData(data);
}

// Force update of data
bool CTabController::UpdateData(void* data)
{
	// Update panel data
	bool result = false;
	for(JIndex index = 1; index <= GetTabCount(); index++)
		result |= static_cast<CTabPanel*>(mCards->GetCard(index))->UpdateData(data);

	return result;
}
