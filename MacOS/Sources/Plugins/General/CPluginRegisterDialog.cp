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

// Source for CPluginRegisterDialog class

#include "CPluginRegisterDialog.h"

#include "CAboutPluginsDialog.h"
#include "CTextFieldX.h"
#include "CUtils.h"
#include "CXStringResources.h"

#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPluginRegisterDialog::CPluginRegisterDialog()
{
}

// Constructor from stream
CPluginRegisterDialog::CPluginRegisterDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CPluginRegisterDialog::~CPluginRegisterDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPluginRegisterDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterName);
	mVersion = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterVersion);
	mType = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterType);
	mManufacturer = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterManufacturer);
	mDescription = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterDescription);
	mRegKey = (CTextFieldX*) FindPaneByID(paneid_PluginRegisterRegKey);
	mRunDemoBtn = (LPushButton*) FindPaneByID(paneid_PluginRegisterRunDemoBtn);
	mRemovePluginBtn = (LPushButton*) FindPaneByID(paneid_PluginRegisterRemovePluginBtn);
	mCancelBtn = (LPushButton*) FindPaneByID(paneid_PluginRegisterCancelBtn);

}

void CPluginRegisterDialog::SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete)
{
	mName->SetText(plugin.GetName());

	mVersion->SetText(::GetVersionText(plugin.GetVersion()));

	mType->SetText(rsrc::GetIndexedString("Alerts::Plugins::Type", plugin.GetType()));

	mManufacturer->SetText(plugin.GetManufacturer());

	mDescription->SetText(plugin.GetDescription());

	if (!allow_demo)
		mRunDemoBtn->Hide();

	if (allow_delete)
		mCancelBtn->Hide();
	else
		mRemovePluginBtn->Hide();
}

cdstring CPluginRegisterDialog::GetRegKey(void) const
{
	return mRegKey->GetText();
}