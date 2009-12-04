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


// Source for CCalendarPaneOptions class

#include "CCalendarPaneOptions.h"

#include "CCalendarViewOptions.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarPaneOptions::CCalendarPaneOptions(LStream *inStream)
		: CCommonViewOptions(inStream)
{
}

// Default destructor
CCalendarPaneOptions::~CCalendarPaneOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CCalendarPaneOptions::FinishCreateSelf()
{
	// Do inherited
	CCommonViewOptions::FinishCreateSelf();
}

void CCalendarPaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CCalendarViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);
}

void CCalendarPaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CCalendarViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);
}
