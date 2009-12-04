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


// Source for C3PaneViewPanel class

#include "C3PaneViewPanel.h"

#include "CBaseView.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneViewPanel::C3PaneViewPanel()
{
}

// Constructor from stream
C3PaneViewPanel::C3PaneViewPanel(LStream *inStream)
		: C3PanePanel(inStream)
{
}

// Default destructor
C3PaneViewPanel::~C3PaneViewPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

bool C3PaneViewPanel::TestClose()
{
	// Get the server view to do the test
	return GetBaseView()->TestClose();
}

void C3PaneViewPanel::DoClose()
{
	// Close the server view
	GetBaseView()->DoClose();
}

bool C3PaneViewPanel::HasFocus() const
{
	// Focus on the view
	return GetBaseView()->HasFocus();
}

void C3PaneViewPanel::Focus()
{
	// Focus on the view
	GetBaseView()->Focus();
}
