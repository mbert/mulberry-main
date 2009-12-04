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


// Source for CCancelDialog class

#include "CCancelDialog.h"

#include "CStaticText.h"

#include <LProgressBar.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCancelDialog::CCancelDialog()
{
	mLastSecs = 0;
}

// Constructor from stream
CCancelDialog::CCancelDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mLastSecs = 0;
}

// Default destructor
CCancelDialog::~CCancelDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCancelDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Find items
	mTime = (CStaticText*) FindPaneByID(paneid_CancelSecs);
	mDesc = (CStaticText*) FindPaneByID(paneid_CancelDesc);

	mBar = (LProgressBar*) FindPaneByID(paneid_CancelProgress);
	mBar->Start();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CCancelDialogBtns);
}

// Set descriptor text
void CCancelDialog::SetBusyDescriptor(const cdstring& desc)
{
	mDesc->SetText(desc);
}

// Called during idle
void CCancelDialog::SetTime(unsigned long secs)
{
	// Only if different
	if (mLastSecs != secs)
	{
		cdstring txt(secs);
		mTime->SetText(txt);
		mLastSecs = secs;
	}
	
	// Always ping the progress bar as this dialog will run with idlers turned
	// off so the pbar won't update
	EventRecord evt;
	mBar->SpendTime(evt);
}
