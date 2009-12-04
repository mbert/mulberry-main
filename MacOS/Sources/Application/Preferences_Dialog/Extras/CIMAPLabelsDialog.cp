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


// Source for CIMAPLabelsDialog class

#include "CIMAPLabelsDialog.h"

#include "CBalloonDialog.h"
#include "CCharSpecials.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextFieldX.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIMAPLabelsDialog::CIMAPLabelsDialog()
{
}

// Constructor from stream
CIMAPLabelsDialog::CIMAPLabelsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CIMAPLabelsDialog::~CIMAPLabelsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CIMAPLabelsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		mNames[i] = (CStaticText*) FindPaneByID(paneid_IMAPLabelsNames[i]);
		mLabels[i] = (CTextFieldX*) FindPaneByID(paneid_IMAPLabelsLabels[i]);
	}

	SetLatentSub(mLabels[0]);
}

// Called during idle
void CIMAPLabelsDialog::SetDetails(const cdstrvect& names, const cdstrvect& labels)
{
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		mNames[i]->SetText(names[i]);
		mLabels[i]->SetText(labels[i]);
	}
}

// Called during idle
bool CIMAPLabelsDialog::GetDetails(cdstrvect& labels)
{
	// First make sure all labels are valid
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		cdstring label = mLabels[i]->GetText();
		if (!ValidLabel(label))
			return false;
	}

	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		labels[i] = mLabels[i]->GetText();
	}
	
	return true;
}

bool CIMAPLabelsDialog::ValidLabel(const cdstring& tlabel)
{
	// Cannot be empty
	if (tlabel.empty())
		return false;

	// Cannot start with backslash
	if (tlabel[0UL] == '\\')
		return false;
	
	// Must be an atom
	const char* p = tlabel.c_str();
	while(*p)
	{
		if (cINETChar[(unsigned char)*p] != 0)
			return false;
		p++;
	}
	
	return true;
}

bool CIMAPLabelsDialog::PoseDialog(const cdstrvect& names, cdstrvect& labels)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_IMAPLabelsDialog, CMulberryApp::sApp);
	CIMAPLabelsDialog* dlog = (CIMAPLabelsDialog*) theHandler.GetDialog();
	dlog->SetDetails(names, labels);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			if (dlog->GetDetails(labels))
			{
				result = true;
				break;
			}
			else
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::InvalidIMAPLabel");
			}
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
