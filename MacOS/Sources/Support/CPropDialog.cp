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


// __________________________________________________________________________________________________
// C L A S S __ C P R O P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropDialog::CPropDialog()
{
	InitPropDialog();
}

// Constructor from stream
CPropDialog::CPropDialog(LStream *inStream)
		: LDialogBox(inStream)
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
	mPanelContainer = nil;
	mCurrentPanel = nil;
	mCurrentPanelNum = 0;
	mRidl = 0;
}

// Get details of sub-panes
void CPropDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Force setup of panels
	SetUpPanels();

	mPanelContainer = (LView*) FindPaneByID(paneid_PropPanel);

	// Link controls to this window
	if (mRidl)
		UReanimator::LinkListenerToBroadcasters(this, this, mRidl);
}


// Set input panel
void CPropDialog::SetPanel(short panel)
{
	ResIDT	panel_open;

	// First remove any existing panel
	delete mCurrentPanel;

	// Update to new panel id
	mCurrentPanelNum = panel;
	panel_open = mPanelList[panel-1];

	// Make panel area default so new panel is automatically added to it
	SetDefaultView(mPanelContainer);
	mPanelContainer->Hide();
	LCommander* defCommander;
	mSubCommanders.FetchItemAt(1, defCommander);
	SetDefaultCommander(defCommander);
	mCurrentPanel = (LView*) UReanimator::ReadObjects('PPob', panel_open);
	mCurrentPanel->FinishCreate();
	DoPanelInit();
	mPanelContainer->Show();
}

// Handle buttons
void CPropDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	// Only change if setting value
	if (*(long*) ioParam)
	{
		int panel = 1;
		for(ulvector::iterator iter = mMsgPanelList.begin(); iter != mMsgPanelList.end(); iter++, panel++)
		{
			if (*iter == inMessage)
				SetPanel(panel);
		}
	}
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
