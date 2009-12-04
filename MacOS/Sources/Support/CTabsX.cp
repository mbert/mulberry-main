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


// CTabsX

#include "CTabsX.h"

#include "CWindowStates.h"

#include <LControlImp.h>
#include "MyCFString.h"

void CTabsX::AddTabButton(const char* name, short icon)
{
	// Get Max value
	SInt32 outMaxValue = GetMaxValue();
	
	// Reset maximum to add button
	SetMaxValue(++outMaxValue);

	ControlTabInfoRecV1 info;
	info.version = kControlTabInfoVersionOne;
	info.iconSuiteID = icon;
	MyCFString temp(name, kCFStringEncodingUTF8);
	info.name = temp;
	GetControlImp()->SetDataTag(outMaxValue, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info);
	Refresh();
}

void CTabsX::SelectTabButtonAt(unsigned long index, bool silent)
{
	if (silent)
		StopBroadcasting();
	SetValue(index + 1);
	if (silent)
		StartBroadcasting();
}

void CTabsX::ChangeTabButtonAt(unsigned long index, const char* name, short icon)
{
	// Temporarily make it invisible during changes
	Boolean	saveVis = ::IsControlVisible(GetControlImp()->GetMacControl());
	::SetControlVisibility(GetControlImp()->GetMacControl(), false, false);

	// Change the name and icon of the tab item
	ControlTabInfoRecV1 info;
	info.version = kControlTabInfoVersionOne;
	GetControlImp()->GetDataTag(index + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info, NULL);
	info.iconSuiteID = icon;
	MyCFString temp(name, kCFStringEncodingUTF8);
	info.name = temp;
	GetControlImp()->SetDataTag(index + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info);
	
	// Make it visible and refresh
	::SetControlVisibility(GetControlImp()->GetMacControl(), saveVis, false);
	Refresh();
}

void CTabsX::ChangeTabButtonIconAt(unsigned long index, short icon)
{
	// Temporarily make it invisible during changes
	Boolean	saveVis = ::IsControlVisible(GetControlImp()->GetMacControl());
	::SetControlVisibility(GetControlImp()->GetMacControl(), false, false);

	// Change the icon of the tab item
	ControlTabInfoRecV1 info;
	info.version = kControlTabInfoVersionOne;
	GetControlImp()->GetDataTag(index + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info, NULL);
	info.iconSuiteID = icon;
	GetControlImp()->SetDataTag(index + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info);
	
	// Make it visible and refresh
	::SetControlVisibility(GetControlImp()->GetMacControl(), saveVis, false);
	Refresh();
}

void CTabsX::RemoveTabButtonAt(unsigned long index)
{
	// Temporarily make it invisible during changes
	Boolean	saveVis = ::IsControlVisible(GetControlImp()->GetMacControl());
	::SetControlVisibility(GetControlImp()->GetMacControl(), false, false);

	// get Max value
	SInt32 outMaxValue = GetMaxValue();
	
	// Move info for buttons at higher pos down by one
	for(SInt32 i = index + 1; i < outMaxValue; i++)
	{
		ControlTabInfoRecV1 info;
		info.version = kControlTabInfoVersionOne;
		GetControlImp()->GetDataTag(i + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info, NULL);
		GetControlImp()->SetDataTag(i, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info);
	}
	
	// Reset maximum to remove button
	// NB Must not broadcast a change in the value - this can happen if the last
	// item is selected when the remove occurs. We do not broadcast because
	// the controlling view will handle the selection change itself if required
	StopBroadcasting();
	SetMaxValue(outMaxValue - 1);
	StartBroadcasting();
	
	// Make it visible and refresh
	::SetControlVisibility(GetControlImp()->GetMacControl(), saveVis, false);
	Refresh();
}

void CTabsX::MoveTabButtonAt(unsigned long oldindex, unsigned long newindex)
{
	// Temporarily make it invisible during changes
	Boolean	saveVis = ::IsControlVisible(GetControlImp()->GetMacControl());
	::SetControlVisibility(GetControlImp()->GetMacControl(), false, false);

	// Check current selected item
	unsigned long selected = GetValue() - 1;

	// Get tab details for the one being removed
	ControlTabInfoRecV1 oldinfo;
	oldinfo.version = kControlTabInfoVersionOne;
	GetControlImp()->GetDataTag(oldindex + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &oldinfo, NULL);
	
	// Now shift others into old index
	long direction = (newindex > oldindex ? 1 : -1);
	for(unsigned long i = oldindex; i != newindex; i += direction)
	{
		ControlTabInfoRecV1 info;
		info.version = kControlTabInfoVersionOne;
		GetControlImp()->GetDataTag(i + 1 + direction, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info, NULL);
		GetControlImp()->SetDataTag(i + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &info);
	}
	
	// Write old one into new position
	GetControlImp()->SetDataTag(newindex + 1, kControlTabInfoTag, sizeof(ControlTabInfoRecV1), &oldinfo);
	
	// Now change the value if it moved
	if ((selected >= std::min(oldindex, newindex)) && (selected <= std::max(oldindex, newindex)))
	{
		StopBroadcasting();
		if (selected == oldindex)
			SetValue(newindex + 1);
		else
			SetValue(selected + 1 - direction);
		StartBroadcasting();
	}
	
	// Make it visible and refresh
	::SetControlVisibility(GetControlImp()->GetMacControl(), saveVis, false);
	Refresh();
}
