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


// Header for CCalendarPropPanel class

#ifndef __CCalendarPropPanel__MULBERRY__
#define __CCalendarPropPanel__MULBERRY__

#include <JXWidgetSet.h>

#include "ptrvector.h"

// Classes
namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CCalendarPropPanel : public JXWidgetSet
{
public:
		CCalendarPropPanel(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual 		~CCalendarPropPanel() {}

	virtual void	OnCreate() = 0;							// Defined in subclass
	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list) = 0;		// Set cal list - pure virtual
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto) = 0;			// Set protocol - pure virtual
	virtual void	ApplyChanges(void) = 0;											// Force update of values

protected:
	calstore::CCalendarStoreNodeList*	mCalList;									// List of selected calendars

};

#endif
