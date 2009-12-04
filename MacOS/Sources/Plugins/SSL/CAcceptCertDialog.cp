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


// Source for CAcceptCertDialog class

#include "CAcceptCertDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CTextDisplay.h"
#include "CTextTable.h"

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAcceptCertDialog::CAcceptCertDialog()
{
}

// Constructor from stream
CAcceptCertDialog::CAcceptCertDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CAcceptCertDialog::~CAcceptCertDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAcceptCertDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CAcceptCertDialogBtns);
}

// Called during idle
void CAcceptCertDialog::SetUpDetails(const char* certificate, const cdstrvect& errors)
{
	// Give text to pane
	CTextTable* errs = (CTextTable*) FindPaneByID(paneid_AcceptCertErrors);
	errs->SetContents(errors);

	// Give text to pane
	CTextDisplay* cert = (CTextDisplay*) FindPaneByID(paneid_AcceptCertCert);
	cert->SetText(certificate);
}

int CAcceptCertDialog::PoseDialog(const char* certificate, const cdstrvect& errors)
{
	int result = 0;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_AcceptCertDialog, CMulberryApp::sApp);
	CAcceptCertDialog* dlog = (CAcceptCertDialog*) theHandler.GetDialog();

	// Set dlog info
	dlog->SetUpDetails(certificate, errors);

	theHandler.StartDialog();

	// Let DialogHandler process events
	MessageT hitMessage;
	while (true)
	{
		hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			result = eAcceptOnce;
			break;
		}
		else if (hitMessage == msg_Cancel)
		{
			result = eNoAccept;
			break;
		}
		else if (hitMessage == msg_AcceptCertAcceptSave)
		{
			result = eAcceptSave;
			break;
		}
	}

	return result;
}
