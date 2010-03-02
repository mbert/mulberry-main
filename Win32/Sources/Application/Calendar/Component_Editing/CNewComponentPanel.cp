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

#include "CNewComponentPanel.h"

void CNewComponentPanel::SetComponent(const iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded)
{
	if (dynamic_cast<const iCal::CICalendarVEvent*>(&vcomponent) != NULL)
		SetEvent(static_cast<const iCal::CICalendarVEvent&>(vcomponent), expanded);
	else if (dynamic_cast<const iCal::CICalendarVToDo*>(&vcomponent) != NULL)
		SetToDo(static_cast<const iCal::CICalendarVToDo&>(vcomponent), expanded);
}

void CNewComponentPanel::GetComponent(iCal::CICalendarComponentRecur& vcomponent)
{
	if (dynamic_cast<iCal::CICalendarVEvent*>(&vcomponent) != NULL)
		GetEvent(static_cast<iCal::CICalendarVEvent&>(vcomponent));
	else if (dynamic_cast<iCal::CICalendarVToDo*>(&vcomponent) != NULL)
		GetToDo(static_cast<iCal::CICalendarVToDo&>(vcomponent));
}
