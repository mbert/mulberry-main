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


// CLetterPartProp.cp : implementation file
//


#include "CLetterPartProp.h"

#include "CAttachment.h"
#include "CMIMESupport.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp dialog

CLetterPartProp::CLetterPartProp(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp message handlers

// Called during startup
void CLetterPartProp::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 405,275, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 405,275);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 50,20);
    assert( obj2 != NULL );

    mName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 305,20);
    assert( mName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Type:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,37, 50,20);
    assert( obj3 != NULL );

    mType =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,35, 200,20);
    assert( mType != NULL );

    mTypePopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,35, 35,20);
    assert( mTypePopup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("SubType:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,62, 66,20);
    assert( obj4 != NULL );

    mSubtype =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,60, 200,20);
    assert( mSubtype != NULL );

    mSubtypePopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,60, 35,20);
    assert( mSubtypePopup != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Encoding:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,87, 66,20);
    assert( obj5 != NULL );

    mEncodingPopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,85, 240,20);
    assert( mEncodingPopup != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Disposition:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,112, 72,20);
    assert( obj6 != NULL );

    mDispositionPopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,110, 240,20);
    assert( mDispositionPopup != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Charset:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,137, 72,20);
    assert( obj7 != NULL );

    mCharsetPopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,135, 240,20);
    assert( mCharsetPopup != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Description:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,162, 73,20);
    assert( obj8 != NULL );

    mDescription =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,160, 305,60);
    assert( mDescription != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 315,240, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,240, 70,25);
    assert( mCancelBtn != NULL );

// end JXLayout

	window->SetTitle("Draft Part Properties");
	SetButtons(mOKBtn, mCancelBtn);

	// Add type list
	cdstring menu;
	for(int index = IDS_TYPE_APPLICATION; index <= IDS_TYPE_VIDEO; index++)
	{
		cdstring str;
		str.FromResource(index);
		if (!menu.empty())
			menu += "|";
		menu += str;
	}
	// Add and set item
	mTypePopup->SetMenuItems(menu);
	
	// Add encoding to list
	menu = cdstring::null_str;
	for(int index = IDS_ENCODING_TEXT; index <= IDS_ENCODING_AD; index++)
	{
		cdstring str;
		str.FromResource(index);
		if (!menu.empty())
			menu += "|";
		menu += str;
		menu += " %r";
	}
	cdstring str;
	str.FromResource(IDS_AUTOMATIC);
	if (!menu.empty())
		menu += "|";
	menu += str;
	menu += " %r";
	// Add and set item
	mEncodingPopup->SetMenuItems(menu);

	// Add disposition to list
	menu = cdstring::null_str;
	for(int index = IDS_DISPOSITION_INLINE; index <= IDS_DISPOSITION_ATTACH; index++)
	{
		cdstring str;
		str.FromResource(index);
		if (!menu.empty())
			menu += "|";
		menu += str;
		menu += " %r";
	}
	// Add and set item
	mDispositionPopup->SetMenuItems(menu);

	// Add charset to list
	menu = cdstring::null_str;
	for(int index = IDS_CHARSET_USASCII; index <= IDS_CHARSET_ISO8858_10; index++)
	{
		cdstring str;
		str.FromResource(index);
		if (!menu.empty())
			menu += "|";
		menu += str;
		menu += " %r";
	}
	// Add and set item
	mCharsetPopup->SetMenuItems(menu);

	// Need to update for every character typed
	mType->ShouldBroadcastAllTextChanged(kTrue);

	ListenTo(mType);
	ListenTo(mTypePopup);
	ListenTo(mSubtypePopup);
	ListenTo(mEncodingPopup);
}

// Respond to clicks in the icon buttons
void CLetterPartProp::Receive(JBroadcaster* sender, const Message& message)
{
	if(message.Is(JXMenu::kItemSelected))
	{
    	JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		if (sender == mTypePopup)
		{
			// Set text in popup
			cdstring txt = mTypePopup->GetItemText(index).GetCString();
			mType->SetText(txt);

			// Force sync
			SyncSubtypePopup(txt);

			return;
		}
		else if (sender == mSubtypePopup)
		{
			// Set text in popup
			cdstring txt = mSubtypePopup->GetItemText(index).GetCString();
			mSubtype->SetText(txt);

			return;
		}
		else if (sender == mEncodingPopup)
		{
			if (mEncodingPopup->GetValue() != eMIMEMode)
			{
				// Encoding change must change type/subtype
				CMIMEContent fake;
				fake.SetTransferMode((ETransferMode) mEncodingPopup->GetValue());

				cdstring atxt = fake.GetContentTypeText();
				mType->SetText(atxt);

				atxt = fake.GetContentSubtypeText();
				mSubtype->SetText(atxt);
			}
			return;
		}
	}
	else if (message.Is(CTextInputField::kTextChanged))
	{
		if (sender == mType)
		{
			// Force sync
			cdstring txt = mType->GetText();
			SyncSubtypePopup(txt);
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
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
	SyncSubtypePopup(content.GetContentTypeText());

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

	if (mDispositionPopup->IsActive())
		content.SetContentDisposition((EContentDisposition) mDispositionPopup->GetValue());

	i18n::ECharsetCode charset = (i18n::ECharsetCode) (mCharsetPopup->GetValue() - 1);
	content.SetCharset(charset);
}

// Sync fields and text
void CLetterPartProp::SyncSubtypePopup(const cdstring& sel)
{
	bool encoding = true;
	bool disposition = true;
	bool charset = false;

	// Enable and set to correct popup
	mSubtypePopup->Activate();

	unsigned long start_index = IDS_SUBTYPE_OCTET;
	unsigned long end_index = IDS_SUBTYPE_OCTET;
	if (sel == cMIMEContentTypes[eContentText])
	{
		start_index = IDS_SUBTYPE_PLAIN;
		end_index = IDS_SUBTYPE_HTML;
	
		// Force charset display
		charset = true;
	}
	else if (sel == cMIMEContentTypes[eContentMultipart])
	{
		start_index = IDS_SUBTYPE_MIXED;
		end_index = IDS_SUBTYPE_ALTERNATIVE;
	
		// Turn off encoding and disposition
		encoding = false;
		disposition = false;
	}
	else if (sel == cMIMEContentTypes[eContentApplication])
	{
		start_index = IDS_SUBTYPE_OCTET;
		end_index = IDS_SUBTYPE_APPLEFILE;
	}
	else if (sel == cMIMEContentTypes[eContentMessage])
	{
		start_index = IDS_SUBTYPE_RFC822;
		end_index = IDS_SUBTYPE_EXTERNAL;
	
		// Turn off encoding and disposition
		encoding = false;
		disposition = false;
	}
	else if (sel == cMIMEContentTypes[eContentImage])
	{
		start_index = IDS_SUBTYPE_GIF;
		end_index = IDS_SUBTYPE_JPEG;
	}
	else if (sel == cMIMEContentTypes[eContentAudio])
	{
		start_index = IDS_SUBTYPE_BASIC;
		end_index = IDS_SUBTYPE_BASIC;
	}
	else if (sel == cMIMEContentTypes[eContentVideo])
	{
		start_index = IDS_SUBTYPE_MPEG;
		end_index = IDS_SUBTYPE_QT;
	}
	cdstring menu;
	for(unsigned long index = start_index; index <= end_index; index++)
	{
		cdstring str;
		str.FromResource(index);
		if (!menu.empty())
			menu += "|";
		menu += str;
	}
	// Add and set item
	mSubtypePopup->SetMenuItems(menu);

	mEncodingPopup->SetActive(JBoolean(encoding));
	mDispositionPopup->SetActive(JBoolean(encoding));
	mCharsetPopup->SetActive(JBoolean(encoding));
}

bool CLetterPartProp::PoseDialog(CMIMEContent& content)
{
	// Create the dialog and give it the message
	CLetterPartProp* dlog = new CLetterPartProp(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFields(content);
	
	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetFields(content);
		dlog->Close();
		return true;
	}

	return false;
}
