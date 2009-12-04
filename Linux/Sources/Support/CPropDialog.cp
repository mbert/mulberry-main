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


// Source for CPropDialog class

#include "CPropDialog.h"

#include <JXCardFile.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R O P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropDialog::CPropDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	InitPropDialog();
}

// Default destructor
CPropDialog::~CPropDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Standard init
void CPropDialog::InitPropDialog(void)
{
	mPanelContainer = NULL;
	mCurrentPanel = NULL;
	mCurrentPanelNum = 0;
}

// Get details of sub-panes
void CPropDialog::OnCreate()
{
	// Force setup of panels
	SetUpPanels();
}


// Set input panel
void CPropDialog::SetPanel(short panel)
{
	// Update to new panel id
	mPanelContainer->ShowCard(panel);
	mCurrentPanelNum = panel;
	mCurrentPanel = mPanelList[panel - 1];
	DoPanelInit();
}

// Handle success OK
bool CPropDialog::DoOK(void)
{
	return true;
}

// Handle Cancel
void CPropDialog::DoCancel(void)
{
}
