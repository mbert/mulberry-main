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


// Source for CLetterPartProp class

#include "CLetterPartProp.h"

#include "CMIMEContent.h"
#include "CMulberryCommon.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C L E T T E R P A R T P R O P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CLetterPartProp::CLetterPartProp()
{
}

// Constructor from stream
CLetterPartProp::CLetterPartProp(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CLetterPartProp::~CLetterPartProp()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CLetterPartProp::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get fields
	mName = (CTextFieldX*) FindPaneByID(paneid_LetterPartName);

	mType = (CTextFieldX*) FindPaneByID(paneid_LetterPartType);
	mType->AddListener(this);
	mTypePopup = (LPopupButton*) FindPaneByID(paneid_LetterPartTypePopup);

	mSubtype = (CTextFieldX*) FindPaneByID(paneid_LetterPartSubtype);
	mSubtypePopup = (LPopupButton*) FindPaneByID(paneid_LetterPartSubtypePopup);

	mEncodingPopup = (LPopupButton*) FindPaneByID(paneid_LetterPartEncodingPopup);

	mDescription = (CTextFieldX*) FindPaneByID(paneid_LetterPartDescription);

	mDispositionPopup = (LPopupButton*) FindPaneByID(paneid_LetterPartDispositionPopup);
	mCharsetPopup = (LPopupButton*) FindPaneByID(paneid_LetterPartCharsetPopup);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CLetterPartProp);

	// Make first edit field active
	SetLatentSub(mName);

}

// Respond to clicks in the icon buttons
void CLetterPartProp::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_TypePopup:
		// Set text in popup
		mType->SetText(::GetPopupMenuItemTextUTF8(mTypePopup));

		// Force sync
		SyncSubtypePopup(*(long*) ioParam);
		SyncEncodingPopup(*(long*) ioParam);
		SyncDispositionPopup(*(long*) ioParam);
		SyncCharsetPopup(*(long*) ioParam);
		break;

	case msg_SubtypePopup:
		// Set text in popup
		mSubtype->SetText(::GetPopupMenuItemTextUTF8(mSubtypePopup));
		break;

	case paneid_LetterPartType:
		if (((CTextFieldX*) ioParam) == mType)
			// Force sync
			SyncTypePopup();
		break;

	case msg_EncodingPopup:
		if (mEncodingPopup->GetValue() != eMIMEMode)
		{
			// Encoding change must change type/subtype
			CMIMEContent fake;
			fake.SetTransferMode((ETransferMode) mEncodingPopup->GetValue());

			mType->SetText(fake.GetContentTypeText());

			mSubtype->SetText(fake.GetContentSubtypeText());

			// Sync popups with text fields
			SyncTypePopup();
		}
		break;
	}
}

// Set fields in dialog
void CLetterPartProp::SetFields(CMIMEContent& content)
{
	mName->SetText(content.GetMappedName());

	mType->SetText(content.GetContentTypeText());

	mSubtype->SetText(content.GetContentSubtypeText());

	mEncodingPopup->SetValue(content.GetTransferMode());

	mDescription->SetText(content.GetContentDescription());

	mDispositionPopup->SetValue(content.GetContentDisposition());

	i18n::ECharsetCode charset = content.GetCharset();
	mCharsetPopup->SetValue(charset + 1);

	// Sync popups with text fields
	SyncTypePopup();

	mName->SelectAll();
}

// Set fields in dialog
void CLetterPartProp::GetFields(CMIMEContent& content)
{
	// Set transfer mode first as this might force change of content
	ETransferMode mode = (ETransferMode) mEncodingPopup->GetValue();
	content.SetTransferMode(mode);

	content.SetMappedName(mName->GetText());

	// Only set type/subtype if not special transfer mode
	switch(mode)
	{
	case eNoTransferMode:
	case eTextMode:
	case eMIMEMode:
		{
			content.SetContentType(mType->GetText());
			content.SetContentSubtype(mSubtype->GetText());
		}
		break;
	default:;
	}

	content.SetContentDescription(mDescription->GetText());

	if (mDispositionPopup->IsEnabled())
		content.SetContentDisposition((EContentDisposition) mDispositionPopup->GetValue());

	i18n::ECharsetCode charset = (i18n::ECharsetCode) (mCharsetPopup->GetValue() - 1);
	content.SetCharset(charset);
}

// Sync fields and text
void CLetterPartProp::SyncTypePopup(void)
{

	// Sync type popup
	cdstring txt = mType->GetText();

	// Init to no mark and loop over all menu items
	StopListening();
	mTypePopup->SetValue(0);
	StartListening();
	MenuHandle menuH = mTypePopup->GetMacMenuH();
	for(short i = 1; i <= ::CountMenuItems(menuH); i++)
	{
		// Get menu item text
		cdstring temp = ::GetMenuItemTextUTF8(menuH, i);

		// Check for match and set menu value
		if (temp == txt)
		{
			StopListening();
			mTypePopup->SetValue(i);
			StartListening();
			SyncSubtypePopup(i);
			break;
		}
	}
}

// Sync fields and text
void CLetterPartProp::SyncSubtypePopup(long type)
{
	if (type)
	{
		// Enable and set to correct popup
		mSubtypePopup->Enable();
		mSubtypePopup->SetMenuID(MENU_MIMETypes + type);

		// Sync with text
		cdstring txt = mSubtype->GetText();
		StopListening();
		mSubtypePopup->SetValue(0);
		StartListening();
		MenuHandle menuH = mSubtypePopup->GetMacMenuH();
		for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		{
			// Get menu item text
			cdstring temp(::GetMenuItemTextUTF8(menuH, i));

			// Check for match and set menu value
			if (temp == txt)
			{
				StopListening();
				mSubtypePopup->SetValue(i);
				StartListening();
				break;
			}
		}
	}
	else
		mSubtypePopup->Disable();
}

// Sync fields and text
void CLetterPartProp::SyncEncodingPopup(long type)
{
	// Handle charset
	switch(type)
	{
	case cMIMEMessage:
	case cMIMEMultipart:
		mEncodingPopup->Disable();
		break;
	default:
		mEncodingPopup->Enable();
	}
}

// Sync fields and text
void CLetterPartProp::SyncDispositionPopup(long type)
{
	// Handle charset
	switch(type)
	{
	case cMIMEMessage:
	case cMIMEMultipart:
		mDispositionPopup->Disable();
		break;
	default:
		mDispositionPopup->Enable();
	}
}

// Sync fields and text
void CLetterPartProp::SyncCharsetPopup(long type)
{
	// Handle charset
	if (type == cMIMEText)
		mCharsetPopup->Enable();
	else
		mCharsetPopup->Disable();
}