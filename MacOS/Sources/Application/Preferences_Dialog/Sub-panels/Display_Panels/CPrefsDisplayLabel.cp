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


// Source for CPrefsDisplayLabel class

#include "CPrefsDisplayLabel.h"

#include "CIMAPLabelsDialog.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LGAColorSwatchControl.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayLabel::CPrefsDisplayLabel()
{
}

// Constructor from stream
CPrefsDisplayLabel::CPrefsDisplayLabel(LStream *inStream)
		: CPrefsDisplayPanel(inStream)
{
}

// Default destructor
CPrefsDisplayLabel::~CPrefsDisplayLabel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayLabel::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsDisplayPanel::FinishCreateSelf();

	// Get controls
	for(int i = 0; i < NMessage::eMaxLabels; i++)
		GetControls(mLabels[i], paneid_DLColour[i], paneid_DLUseColour[i], paneid_DLBkgColour[i], paneid_DLUseBkgColour[i], paneid_DLBold[i], paneid_DLItalic[i], paneid_DLStrike[i], paneid_DLUnderline[i], paneid_DLName[i]);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDisplayLabelBtns);
}

// Handle buttons
void CPrefsDisplayLabel::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch(inMessage)
	{
	case msg_DLIMAPLabels:
		OnIMAPLabels();
		break;
	default:
		// Enable/disable colour buttons
		for(int i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (inMessage == msg_DLUseColour[i])
			{
				if (*(long*) ioParam)
					mLabels[i].mColor->Enable();
				else
					mLabels[i].mColor->Disable();
			}
			else if (inMessage == msg_DLUseBkgColour[i])
			{
				if (*(long*) ioParam)
					mLabels[i].mBkgColor->Enable();
				else
					mLabels[i].mBkgColor->Disable();
			}
		}
		break;
	}
}

// Set prefs
void CPrefsDisplayLabel::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		SetStyle(mLabels[i], *copyPrefs->mLabels.GetValue()[i]);
		mIMAPLabels.push_back(copyPrefs->mIMAPLabels.GetValue()[i]);
	}
}

// Force update of prefs
void CPrefsDisplayLabel::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits2 traits;

	// Copy info from panel into prefs
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		*copyPrefs->mLabels.Value()[i] = GetStyle(mLabels[i], traits);
		copyPrefs->mIMAPLabels.Value()[i] = mIMAPLabels[i];
	}
}

// Set IMAP labels
void CPrefsDisplayLabel::OnIMAPLabels()
{
	// Get current names
	cdstrvect names;
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		names.push_back(mLabels[i].mName->GetText());
	}
	
	CIMAPLabelsDialog::PoseDialog(names, mIMAPLabels);
}
