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


// Source for CSaveAsRemotePrefsDialog class

#include "CSaveAsRemotePrefsDialog.h"

#include "CMulberryApp.h"
#include "CPreferenceKeys.h"
#include "CRemotePrefsSets.h"
#include "CTextField.h"
#include "CTextTable.h"

#include <LGAPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C S A V E A S R E M O T E P R E F S D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSaveAsRemotePrefsDialog::CSaveAsRemotePrefsDialog()
{
}

// Constructor from stream
CSaveAsRemotePrefsDialog::CSaveAsRemotePrefsDialog(LStream *inStream)
		: LGADialogBox(inStream)
{
}

// Default destructor
CSaveAsRemotePrefsDialog::~CSaveAsRemotePrefsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSaveAsRemotePrefsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LGADialogBox::FinishCreateSelf();

	// Get items
	mOKBtn = (LGAPushButton*) FindPaneByID(paneid_SaveAsRemotePrefsOKBtn);
	mOKBtn->Disable();
	mRemoteList = (CTextTable*) FindPaneByID(paneid_SaveAsRemotePrefsList);
	mRemoteList->SetSingleSelection();
	mRemoteList->AddListener(this);

	cdstrvect items;

	// Add default name first
	items.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		items.push_back(*iter);

	mRemoteList->SetContents(items);

	// Get name item
	mName = (CTextField*) FindPaneByID(paneid_SaveAsRemotePrefsName);
	mName->AddListener(this);
}

void CSaveAsRemotePrefsDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case paneid_SaveAsRemotePrefsName:
		{
			// Now check that there's text in required field
			Str255 copyStr;
			
			mName->GetDescriptor(copyStr);
			if (*copyStr)
				mOKBtn->Enable();
			else
				mOKBtn->Disable();
		}
		break;

	case msg_SaveAsRemotePrefsListClick:
	case msg_SaveAsRemotePrefsListDblClick:
		{
			cdstrvect strs;
			mRemoteList->GetSelection(strs);
			if (strs.size())
				SetName(strs.front());
		}
		break;

	default:
		// Pass up
		LGADialogBox::ListenToMessage(inMessage, ioParam);
	}

}

// Get details from dialog
void CSaveAsRemotePrefsDialog::SetName(const char* name)
{
	LStr255	copyStr;

	copyStr = name;
	mName->SetDescriptor(copyStr);

	mName->GetSuperCommander()->SetLatentSub(mName);
	LCommander::SwitchTarget(mName);
	mName->SelectAll();
	
	if (*name)
		mOKBtn->Enable();
	else
		mOKBtn->Disable();
}

// Get details from dialog
cdstring CSaveAsRemotePrefsDialog::GetDetails(void)
{
	Str255		copyStr;

	// Copy info
	mName->GetDescriptor(copyStr);

	return copyStr;
}

