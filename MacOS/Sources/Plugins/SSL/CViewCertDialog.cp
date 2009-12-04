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


// Source for CViewCertDialog class

#include "CViewCertDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CTextDisplay.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CViewCertDialog::CViewCertDialog()
{
}

// Constructor from stream
CViewCertDialog::CViewCertDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CViewCertDialog::~CViewCertDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set items
void CViewCertDialog::SetUpDetails(const cdstring& cert)
{
	// Give text to pane
	CTextDisplay* certtext = (CTextDisplay*) FindPaneByID(paneid_ViewCertCert);
	certtext->SetText(cert);
}

void CViewCertDialog::PoseDialog(const cdstring& cert)
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_ViewCertDialog, CMulberryApp::sApp);
	CViewCertDialog* dlog = (CViewCertDialog*) theHandler.GetDialog();

	// Set dlog info
	dlog->SetUpDetails(cert);

	theHandler.StartDialog();

	// Let DialogHandler process events
	MessageT hitMessage;
	while (true)
	{
		hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
			break;
	}
}
