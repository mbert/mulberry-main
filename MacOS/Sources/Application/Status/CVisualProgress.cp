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


// Source for CProgress classes

#include "CVisualProgress.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CStatusWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C M U L T I U S E R D I A L O G
// __________________________________________________________________________________________________

const long cBarScale = 1000;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CProgressDialog::CProgressDialog()
{
	mBarPane = nil;
}

// Constructor from stream
CProgressDialog::CProgressDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mBarPane = nil;
}

// Default destructor
CProgressDialog::~CProgressDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CProgressDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get bar
	mBarPane = (CBarPane*) FindPaneByID(paneid_ProgressBar);

	// Do user id
	mTitle = (CStaticText*) FindPaneByID(paneid_ProgressTitle);

}

CBalloonDialog* CProgressDialog::StartDialog(const cdstring& rsrc)
{
	CBalloonDialog* theHandler = new CBalloonDialog(paneid_ProgressDialog, CMulberryApp::sApp);
	CProgressDialog* progress = (CProgressDialog*) theHandler->GetDialog();

	// Get the progress description
	cdstring status;
	status.FromResource(rsrc);
	progress->SetDescriptor(status);
	progress->SetIndeterminate();
	theHandler->StartDialog();
	
	return theHandler;
}

void CProgressDialog::EndDialog(CBalloonDialog* dlog)
{
	delete dlog;
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C B A R P A N E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CBarPane::CBarPane(LStream *inStream)
		: LProgressBar(inStream)
{
}

// Default destructor
CBarPane::~CBarPane()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CBarPane::SetCount(unsigned long count)
{
	// Do inherited
	CProgress::SetCount(count);
	LProgressBar::SetValue(count);
}

void CBarPane::SetTotal(unsigned long total)
{
	// Do inherited
	CProgress::SetTotal(total);
	LProgressBar::SetMaxValue(total);
}
