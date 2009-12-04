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


// Source for CPrefsEditMappingDialog class

#include "CPrefsEditMappingDialog.h"

#include "CBalloonDialog.h"
#include "CDesktopIcons.h"
#include "CMIMEMap.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LPushButton.h>
#include <LPopupButton.h>

#include <UStandardDialogs.h>


// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsEditMappingDialog::CPrefsEditMappingDialog()
	: mTypeArray(sizeof(OSType), NULL, true)
{
	mCurrentCreator = 0;
}

// Constructor from stream
CPrefsEditMappingDialog::CPrefsEditMappingDialog(LStream *inStream)
		: LDialogBox(inStream),
		  mTypeArray(sizeof(OSType), NULL, true)
{
	mCurrentCreator = 0;
}

// Default destructor
CPrefsEditMappingDialog::~CPrefsEditMappingDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsEditMappingDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Set fields
	mMIMEType = (CTextFieldX*) FindPaneByID(paneid_MIMEType);

	mMIMESubtype = (CTextFieldX*) FindPaneByID(paneid_MIMESubtype);

	mApplication = (CTextFieldX*) FindPaneByID(paneid_Application);

	// Popup
	mTypeMenu = (LPopupButton*) FindPaneByID(paneid_FileType);

	// Set buttons
	mChooseBtn = (LPushButton*) FindPaneByID(paneid_ChooseBtn);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsEditMappingDialogBtns);
}

// Handle buttons
void CPrefsEditMappingDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_ChooseApplication:
			DoChooseApplication();
			break;
	}
}

// Set mappings table
void CPrefsEditMappingDialog::SetMapping(const CMIMEMap& mapping)
{
	cdstring type;
	if (mapping.GetMIMEType())
		type = mapping.GetMIMEType();
	mMIMEType->SetText(type);

	cdstring subtype;
	if (mapping.GetMIMESubtype())
		subtype = mapping.GetMIMESubtype();
	mMIMESubtype->SetText(subtype);

	LStr255 name;
	mCurrentCreator = mapping.GetFileCreator();
	if (mCurrentCreator != '????')
		// Get name from database
		CDesktopIcons::GetAPPLName(mCurrentCreator, name);
	else
		// App name is blank
		name = "????";
	cdstring temp(name);
	mApplication->SetText(temp);

	mOriginalFileType = mapping.GetFileType();

	ResetPopupMenu(mCurrentCreator);
}

void CPrefsEditMappingDialog::GetMapping(CMIMEMap& mapping)
{
	mapping.SetMIMEType(mMIMEType->GetText());

	mapping.SetMIMESubtype(mMIMESubtype->GetText());
	
	mapping.SetFileCreator(mCurrentCreator);

	OSType new_type = 0;
	MenuHandle mhand = mTypeMenu->GetMacMenuH();
	if (::CountMenuItems(mhand) > 0)
		mTypeArray.FetchItemAt(mTypeMenu->GetValue(), &new_type);
	mapping.SetFileType(new_type);
}

// Choose an application
void CPrefsEditMappingDialog::DoChooseApplication(void)
{
	// Set value
	PPx::FSObject fspec;
	if (PP_StandardDialogs::AskChooseOneFile('APPL', fspec, kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews | kNavSupportPackages))
	{
#if PP_Target_Carbon
		// Special behaviour for bundles
		if (fspec.IsFolder())
		{
			// Convert to CFURL
			CFURLRef url = ::CFURLCreateFromFSRef(kCFAllocatorDefault, &fspec.UseRef());
			if (url)
			{
				// Get the bundle - release URL as no longer required
				CFBundleRef bundle = ::CFBundleCreate(kCFAllocatorDefault, url);
				::CFRelease(url);

				if (bundle)
				{
					// Get localised and top-level info.plist from bundle
					CFDictionaryRef ldict = ::CFBundleGetLocalInfoDictionary(bundle);
					CFDictionaryRef dict = ::CFBundleGetInfoDictionary(bundle);
					if (ldict == NULL)
						ldict = dict;

					if (dict != NULL)
					{
						// Get application name from bundle
						CFStringRef str = static_cast<CFStringRef>(::CFDictionaryGetValue(ldict, kCFBundleNameKey));
						if (str)
						{
							cdstring appl;
							appl.reserve(256);
							if (::CFStringGetCString (str, appl.c_str_mod(), 256, kCFStringEncodingUTF8))
								mApplication->SetText(appl);
						}
						else
						{
							CFStringRef str = static_cast<CFStringRef>(::CFDictionaryGetValue(dict, kCFBundleNameKey));
							if (str)
							{
								cdstring appl;
								appl.reserve(256);
								if (::CFStringGetCString (str, appl.c_str_mod(), 256, kCFStringEncodingUTF8))
									mApplication->SetText(appl);
							}
						}

						// Get application signature from bundle
						CFStringRef bsig = CFSTR("CFBundleSignature");
						str = static_cast<CFStringRef>(::CFDictionaryGetValue(dict, bsig));
						if (str)
						{
							cdstring sig;
							sig.reserve(256);
							if (::CFStringGetCString (str, sig.c_str_mod(), 256, kCFStringEncodingUTF8))
								mCurrentCreator = *(const OSType*)sig.c_str();
						}
						else
							mCurrentCreator = 0;
 					}
 
					// Clean-up
					::CFRelease(bundle);
				}
			}
		}
		else
#endif
		{
			FinderInfo info;
			fspec.GetFinderInfo(&info, NULL, NULL);

			LStr255 name;
			mCurrentCreator = info.file.fileCreator;
			CDesktopIcons::GetAPPLName(mCurrentCreator, name);
			cdstring temp(name);
			mApplication->SetText(temp);
		}

		ResetPopupMenu(mCurrentCreator);
	}
}

// Reset popup menu for new creator
void CPrefsEditMappingDialog::ResetPopupMenu(OSType creator)
{
	// Remove all existsing items from menu
	MenuHandle mhand = mTypeMenu->GetMacMenuH();
	short num = ::CountMenuItems(mhand) + 1;
	while(--num)
		::DeleteMenuItem(mhand, num);

	// Empty list
	mTypeArray.RemoveItemsAt(mTypeArray.GetCount(), LArray::index_First);

	// If emtpy creator do nothing
	if (creator == '????') return;

	// Get new items from database
	CDesktopIcons::GetTypeList(creator, mTypeArray);

	// Add all items to menu
	LArrayIterator iterator(mTypeArray, LArrayIterator::from_Start);

	OSType type;
	short pos = 1;
	while(iterator.Next(&type))
	{
		::InsertMenuItem(mhand, "\p?", pos);
		cdstring ftype((const char*)&type, 4);
		::SetMenuItemTextUTF8(mhand, pos++, ftype);
	}
	mTypeMenu->SetMaxValue(pos - 1);

	// Try to find the existing value
	pos = mTypeArray.FetchIndexOf(&mOriginalFileType);
	if (pos == LArray::index_Bad)
	{
		OSType new_type;
		mTypeMenu->SetValue(1);
		mTypeArray.FetchItemAt(1, &new_type);

		// Do not start with 'APPL'
		if (new_type == 'APPL')
		{
			mTypeMenu->SetValue(2);
			mTypeArray.FetchItemAt(2, &new_type);
		}
	}
	else
		mTypeMenu->SetValue(pos);
}

bool CPrefsEditMappingDialog::PoseDialog(CMIMEMap& mapping)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_PrefsEditMappingDialog, CMulberryApp::sApp);
	CPrefsEditMappingDialog* dlog = (CPrefsEditMappingDialog*) theHandler.GetDialog();
	dlog->SetMapping(mapping);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			dlog->GetMapping(mapping);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
