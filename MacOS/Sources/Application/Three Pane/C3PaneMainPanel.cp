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


// Source for C3PaneMainPanel class

#include "C3PaneMainPanel.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneMainPanel::C3PaneMainPanel()
{
	mSubView = NULL;
}

// Constructor from stream
C3PaneMainPanel::C3PaneMainPanel(LStream *inStream)
		: C3PanePanel(inStream)
{
	mSubView = NULL;
}

// Default destructor
C3PaneMainPanel::~C3PaneMainPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneMainPanel::FinishCreateSelf()
{
	// Do inherited
	C3PanePanel::FinishCreateSelf();
}

bool C3PaneMainPanel::TestClose()
{
	// Pass down to sub view
	return (mSubView ? mSubView->TestClose() : true);
}

void C3PaneMainPanel::DoClose()
{
	// Pass down to sub view
	if (mSubView)
		mSubView->DoClose();
}
