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

// Source for CAboutPluginsDialog class

#include "CAboutPluginsDialog.h"

#include "CPluginManager.h"
#include "CTextFieldX.h"
#include "CTextHierarchy.h"
#include "CUtils.h"
#include "CXStringResources.h"

#include <LPushButton.h>

//#include <PLStringFuncs.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAboutPluginsDialog::CAboutPluginsDialog()
{
}

// Constructor from stream
CAboutPluginsDialog::CAboutPluginsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CAboutPluginsDialog::~CAboutPluginsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAboutPluginsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mRegisterBtn = (LPushButton*) FindPaneByID(paneid_AboutPluginsRegister);
	mRegisterBtn->AddListener(this);
	mRegisterBtn->Disable();
	mName = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsName);
	mVersion = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsVersion);
	mType = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsType);
	mManufacturer = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsManufacturer);
	mDescription = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsDescription);
	mMode = (CTextFieldX*) FindPaneByID(paneid_AboutPluginsMode);
	mList = (CTextHierarchy*) FindPaneByID(paneid_AboutPluginsList);
	mList->AddListener(this);

	// Add types to list
	TableIndexT parent = 0;
	const CPluginList& plugins = CPluginManager::sPluginManager.GetPlugins();

	for(int i = CPlugin::ePluginUnknown; i < CPlugin::ePluginLast; i++)
	{
		LStr255 title(rsrc::GetIndexedString("Alerts::Plugins::Type", i).c_str());
		parent = mList->InsertSiblingRows(1, parent, title, *title + 1, true);
		mHierPlugins.push_back(nil);

		// Add suitable children
		for(CPluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		{
			if ((*iter)->GetType() == (CPlugin::EPluginType) i)
			{
				LStr255 name;
				name = (*iter)->GetName();
				mList->AddLastChildRow(parent, StringPtr(name), name.Length() + 1);
				mHierPlugins.push_back(*iter);
			}
		}

		// Always expanded
		mList->ExpandRow(parent);
	}
}

// Handle OK button
void CAboutPluginsDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_Selection:
	{
		STableCell aCell(0, 0);
		if (mList->GetNextSelectedCell(aCell))
			ShowPluginInfo(mHierPlugins.at(mList->GetWideOpenIndex(aCell.row) - 1));
		else
			ShowPluginInfo(nil);
		break;
	}

	case msg_AboutRegister:
	{
		STableCell aCell(0, 0);
		if (mList->GetNextSelectedCell(aCell))
		{
			CPlugin* plugin = mHierPlugins.at(mList->GetWideOpenIndex(aCell.row) - 1);
			// Attempt to load - will cache new refcon
			StLoadPlugin _load(plugin);

			plugin->DoRegistration(false, false);
			ShowPluginInfo(plugin);
		}
		break;
	}

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CAboutPluginsDialog::ShowPluginInfo(const CPlugin* plugin)
{
	if (plugin)
	{
		mName->SetText(plugin->GetName());

		mVersion->SetText(::GetVersionText(plugin->GetVersion()));

		cdstring temp = rsrc::GetIndexedString("Alerts::Plugins::Type", plugin->GetType());
		mType->SetText(temp);

		mManufacturer->SetText(plugin->GetManufacturer());

		mDescription->SetText(plugin->GetDescription());

		if (plugin->IsRegistered())
			temp = rsrc::GetString("Alerts::Plugins::ModeRegistered");
		else if (plugin->IsDemo())
			temp = rsrc::GetString("Alerts::Plugins::ModeDemo");
		else
			temp = rsrc::GetString("Alerts::Plugins::ModeUnregistered");
		mMode->SetText(temp);

		if (plugin->IsDemo())
			mRegisterBtn->Enable();
		else
			mRegisterBtn->Disable();
	}
	else
	{
		cdstring txt;
		mName->SetText(txt);
		mVersion->SetText(txt);
		mType->SetText(txt);
		mManufacturer->SetText(txt);
		mDescription->SetText(txt);
		mMode->SetText(txt);
		mRegisterBtn->Disable();
	}
}