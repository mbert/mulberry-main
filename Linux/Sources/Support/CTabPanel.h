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


// CTabPanel

// Class that implements a tab panel in a CTabCtrl

#ifndef __CTABPANEL__MULBERRY__
#define __CTABPANEL__MULBERRY__

#include <JXWidgetSet.h>

class CTabPanel : public JXWidgetSet
{
public:
	CTabPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void	OnCreate() = 0;
	virtual void	SetData(void* data) = 0;			// Set data
	virtual bool	ValidateData(void* data)			// Validate and update data
		{ UpdateData(data); return true; }
	virtual bool	UpdateData(void* data) = 0;			// Force update of data
};

#endif
