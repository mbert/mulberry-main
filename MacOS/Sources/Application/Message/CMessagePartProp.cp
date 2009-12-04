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


// Source for CMessagePartProp class

#include "CMessagePartProp.h"

#include "CAttachment.h"
#include "CMulberryCommon.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E P A R T P R O P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessagePartProp::CMessagePartProp()
{
}

// Constructor from stream
CMessagePartProp::CMessagePartProp(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CMessagePartProp::~CMessagePartProp()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMessagePartProp::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get fields
	mCancelAll = (LPushButton*) FindPaneByID(paneid_MessagePartCancelAll);

	mName = (CTextFieldX*) FindPaneByID(paneid_MessagePartName);
	mType = (CTextFieldX*) FindPaneByID(paneid_MessagePartType);
	mEncoding = (CTextFieldX*) FindPaneByID(paneid_MessagePartEncoding);
	mID = (CTextFieldX*) FindPaneByID(paneid_MessagePartID);
	mDisposition = (CTextFieldX*) FindPaneByID(paneid_MessagePartDisposition);
	mDescription = (CTextFieldX*) FindPaneByID(paneid_MessagePartDescription);
	mParameters = (CTextDisplay*) FindPaneByID(paneid_MessagePartParameters);
	mOpenWith = (CTextFieldX*) FindPaneByID(paneid_MessagePartOpenWith);

	// Make first edit field active
	SetLatentSub(mName);

}

// Set fields in dialog
void CMessagePartProp::SetFields(CAttachment& attach, bool multi)
{
	// Hide cancel if only one
	if (multi)
		mCancelAll->Show();
	else
		mCancelAll->Hide();

	CMIMEContent& content = attach.GetContent();

	mName->SetText(attach.GetMappedName(true, true));

	cdstring temp;
	temp += content.GetContentTypeText();
	temp += "/";
	temp += content.GetContentSubtypeText();
	mType->SetText(temp);

	mEncoding->SetText(content.GetTransferEncodingText());

	mID->SetText(content.GetContentId());

	mDisposition->SetText(content.GetContentDispositionText());

	mDescription->SetText(content.GetContentDescription());

	for(int i = 0; i < content.CountContentParameters(); i++)
	{
		cdstring params = content.GetContentParameterText(i);
		params += "\r";
		mParameters->InsertUTF8(params);
	}

	mOpenWith->SetText(CMIMESupport::MapMIMEToApp(attach));

	mName->SelectAll();
}
