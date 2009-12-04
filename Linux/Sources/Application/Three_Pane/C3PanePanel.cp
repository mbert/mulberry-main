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


// Source for C3PanePanel class

#include "C3PanePanel.h"

#include "C3PaneWindow.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PanePanel::C3PanePanel(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	m3PaneWindow = NULL;
	mViewType = N3Pane::eView_Empty;
}

// Default destructor
C3PanePanel::~C3PanePanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PanePanel::OnCreate()
{
	// See if we are a child of a 3-pane
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(GetWindow()->GetDirector());
}

void C3PanePanel::SetViewType(N3Pane::EViewType view)
{
	// Set the pane contents
	mViewType = view;
}
