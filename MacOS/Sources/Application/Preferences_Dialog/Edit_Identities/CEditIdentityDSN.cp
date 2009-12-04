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


// Source for CEditIdentityDSN class

#include "CEditIdentityDSN.h"

#include "CIdentity.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LRadioButton.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditIdentityDSN::CEditIdentityDSN()
{
}

// Constructor from stream
CEditIdentityDSN::CEditIdentityDSN(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CEditIdentityDSN::~CEditIdentityDSN()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentityDSN::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentityDSNActive);
	mUseDSN = (LCheckBox*) FindPaneByID(paneid_EditIdentityDSNUse);
	mSuccess = (LCheckBox*) FindPaneByID(paneid_EditIdentityDSNSuccess);
	mFailure = (LCheckBox*) FindPaneByID(paneid_EditIdentityDSNFailure);
	mDelay = (LCheckBox*) FindPaneByID(paneid_EditIdentityDSNDelay);
	mFull = (LRadioButton*) FindPaneByID(paneid_EditIdentityDSNFull);
	mHeaders = (LRadioButton*) FindPaneByID(paneid_EditIdentityDSNHeaders);
}


// Set prefs
void CEditIdentityDSN::SetData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	mActive->SetValue(identity->GetUseDSN());
	mUseDSN->SetValue(identity->GetDSN().GetRequest());
	mSuccess->SetValue(identity->GetDSN().GetSuccess());
	mFailure->SetValue(identity->GetDSN().GetFailure());
	mDelay->SetValue(identity->GetDSN().GetDelay());
	mFull->SetValue(identity->GetDSN().GetFull());
	mHeaders->SetValue(!identity->GetDSN().GetFull());
}

// Force update of prefs
void CEditIdentityDSN::UpdateData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseDSN(mActive->GetValue() == 1);
	identity->SetDSN().SetRequest(mUseDSN->GetValue() == 1);
	identity->SetDSN().SetSuccess(mSuccess->GetValue() == 1);
	identity->SetDSN().SetFailure(mFailure->GetValue() == 1);
	identity->SetDSN().SetDelay(mDelay->GetValue() == 1);
	identity->SetDSN().SetFull(mFull->GetValue() == 1);
}
