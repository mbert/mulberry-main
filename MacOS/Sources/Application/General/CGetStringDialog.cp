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


// Source for CGetStringDialog class

#include "CGetStringDialog.h"

#include "CBalloonDialog.h"
#include "CHelpAttach.h"
#include "CMulberryApp.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CXStringResources.h"

#include "MyCFString.h"

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGetStringDialog::CGetStringDialog()
{
}

// Constructor from stream
CGetStringDialog::CGetStringDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CGetStringDialog::~CGetStringDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CGetStringDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CGetStringDialogBtns);

	// Make text edit field active
	CTextFieldX* theText = (CTextFieldX*) FindPaneByID(paneid_GetStringText);
	SetLatentSub(theText);
}

// Called during idle
void CGetStringDialog::SetUpDetails(const char* itxt, cdstring& change,
									const char* extra1, const char* extra2)
{
	cdstring txt;

	// Insert extra text and leave as c-str in case > 255 chars
	if (extra1 && !extra2)
	{
		// Load in from resource if there
		if (itxt != NULL)
		{
			cdstring temp1 = itxt;
			cdstring temp2;
			size_t temp2_reserve = temp1.length() + ::strlen(extra1) + 1;
			temp2.reserve(temp2_reserve);
			::snprintf(temp2.c_str_mod(), temp2_reserve, temp1.c_str(), extra1);
			txt = temp2;
		}
		else
			// Just use extra as main text
			txt = extra1;
	}
	else if (extra1 && extra2)
	{
		// Load in from resource if there
		if (itxt != NULL)
		{
			cdstring temp1 = itxt;
			cdstring temp2;
			size_t temp2_reserve = temp1.length() + ::strlen(extra1) + ::strlen(extra2) + 1;
			temp2.reserve(temp2_reserve);
			::snprintf(temp2.c_str_mod(), temp2_reserve, temp1.c_str(), extra1, extra2);
			txt = temp2;
		}
		else
		{
			// Just use extras as main text
			txt = extra1;
			txt += extra2;
		}
	}
	else
		// Set alert message
		txt = itxt;

	// Give text to pane and window title
	CStaticText* theCaption = (CStaticText*) FindPaneByID(paneid_GetStringCaption);
	theCaption->SetText(txt);
	MyCFString temp(txt, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// Give text to pane
	CTextFieldX* theText = (CTextFieldX*) FindPaneByID(paneid_GetStringText);
	theText->SetText(change);
	theText->SelectAll();

}

bool CGetStringDialog::PoseDialog(ResIDT strx, ResIDT text, cdstring& change,
										const char* extra1, const char* extra2, bool beep)
{
	cdstring temp;
	if (strx && text)
		temp.FromResource(strx, text);
	return _PoseDialog((strx && text) ? temp.c_str() : NULL, change, extra1, extra2, beep);
}

bool CGetStringDialog::PoseDialog(const char* txt, cdstring& change,
										const char* extra1, const char* extra2, bool beep)
{
	return _PoseDialog(rsrc::GetString(txt), change, extra1, extra2, beep);
}

bool CGetStringDialog::_PoseDialog(const char* txt, cdstring& change,
										const char* extra1, const char* extra2, bool beep)
{
	bool result = false;

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_GetStringDialog, CMulberryApp::sApp);
		CGetStringDialog* dlog = (CGetStringDialog*) theHandler.GetDialog();

		// Set dlog info
		dlog->SetUpDetails(txt, change, extra1, extra2);

		theHandler.StartDialog();
		if (beep) ::SysBeep(1);

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{
			hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				CTextFieldX* theText = (CTextFieldX*) dlog->FindPaneByID(paneid_GetStringText);
				change = theText->GetText();
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
