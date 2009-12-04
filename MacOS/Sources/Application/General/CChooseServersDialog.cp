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


// Source for CChooseServersDialog class

#include "CChooseServersDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CTextFieldX.h"

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CChooseServersDialog::CChooseServersDialog()
{
}

// Constructor from stream
CChooseServersDialog::CChooseServersDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CChooseServersDialog::~CChooseServersDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CChooseServersDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Make text edit field active
	CTextFieldX* theText = (CTextFieldX*) FindPaneByID(paneid_ChooseServerMail);
	SetLatentSub(theText);
}

bool CChooseServersDialog::PoseDialog(cdstring& mail, cdstring& smtp)
{
	bool result = false;

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_ChooseServerDialog, CMulberryApp::sApp);
		CChooseServersDialog* dlog = (CChooseServersDialog*) theHandler.GetDialog();

		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{					
			hitMessage = theHandler.DoDialog();
			
			if (hitMessage == msg_OK)
			{
				CTextFieldX* theText = (CTextFieldX*) dlog->FindPaneByID(paneid_ChooseServerMail);
				mail = theText->GetText();
				theText = (CTextFieldX*) dlog->FindPaneByID(paneid_ChooseServerSMTP);
				smtp = theText->GetText();
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
