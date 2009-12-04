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


// Source for CMDNPromptDialog class

#include "CMDNPromptDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CStaticText.h"

#include <LCheckBox.h>

// __________________________________________________________________________________________________
// C L A S S __ C O P E N M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMDNPromptDialog::CMDNPromptDialog()
{
}

// Constructor from stream
CMDNPromptDialog::CMDNPromptDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CMDNPromptDialog::~CMDNPromptDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMDNPromptDialog::FinishCreateSelf()
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mAddress = (CStaticText*) FindPaneByID(paneid_MDNPromptAddress);
	mSave = (LCheckBox*) FindPaneByID(paneid_MDNPromptSave);
}

// Set the dialogs info
void CMDNPromptDialog::SetDetails(const cdstring& addr)
{
	mAddress->SetText(addr);
	mSave->SetValue(0);
}

// Set the dialogs info
void CMDNPromptDialog::GetDetails(bool& save)
{
	save = mSave->GetValue();
}

bool CMDNPromptDialog::PoseDialog(const cdstring& addr)
{
	bool result = false;

	CBalloonDialog dlog(paneid_MDNPromptDialog, CMulberryApp::sApp);
	CMDNPromptDialog* mdlog = static_cast<CMDNPromptDialog*>(dlog.GetDialog());
	mdlog->SetDetails(addr);
	dlog.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = dlog.DoDialog();

		if (hitMessage == msg_OK)
		{
			bool save;
			mdlog->GetDetails(save);

			// Set preferences if required
			if (save)
				CPreferences::sPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);

			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
		{
			bool save;
			mdlog->GetDetails(save);

			// Set preferences if required
			if (save)
				CPreferences::sPrefs->mMDNOptions.SetValue(eMDNNeverSend);

			result = false;
			break;
		}
	}

	return result;
}
