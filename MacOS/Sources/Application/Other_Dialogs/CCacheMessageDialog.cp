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


// Source for CCacheMessageDialog class

#include "CCacheMessageDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CTextFieldX.h"

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCacheMessageDialog::CCacheMessageDialog()
{
}

// Constructor from stream
CCacheMessageDialog::CCacheMessageDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CCacheMessageDialog::~CCacheMessageDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCacheMessageDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mNumber = (CTextFieldX*) FindPaneByID(paneid_CacheMessageNumber);
	mNumber->SetNumber(1);
	SetLatentSub(mNumber);

}

// Get the details
void CCacheMessageDialog::GetDetails(unsigned long& goto_num)
{
	goto_num = mNumber->GetNumber();
}

bool CCacheMessageDialog::PoseDialog(unsigned long& goto_num)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog dlog(paneid_CacheMessageDialog, CMulberryApp::sApp);

	// Run modal loop and look for OK
	if (dlog.DoModal() == msg_OK)
	{
		static_cast<CCacheMessageDialog*>(dlog.GetDialog())->GetDetails(goto_num);
		result = true;
	}
	
	return result;
}
