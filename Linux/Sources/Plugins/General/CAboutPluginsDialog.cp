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

// CAboutPluginsDialog.cpp : implementation file
//

#include "CAboutPluginsDialog.h"

#include "CPluginManager.h"
#include "CStaticText.h"
#include "CTableScrollbarSet.h"
#include "CUtils.h"
#include "CXStringResources.h"

#include <UNX_LTableMonoGeometry.h>
#include <UNX_LTableSingleSelector.h>

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog dialog


CAboutPluginsDialog::CAboutPluginsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}


/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog message handlers

// Get details of sub-panes
void CAboutPluginsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 480,370, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 480,370);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Plug-ins:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 60,20);
    assert( obj2 != NULL );
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    CScrollbarSet* sbs =
        new CScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 210,295);
    assert( sbs != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,30, 85,15);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    JXStaticText* obj4 =
        new JXStaticText("Version:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,55, 85,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    JXStaticText* obj5 =
        new JXStaticText("Type:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,80, 85,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    JXStaticText* obj6 =
        new JXStaticText("Manufacturer:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,105, 85,15);
    assert( obj6 != NULL );
    obj6->SetFontSize(10);
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    JXStaticText* obj7 =
        new JXStaticText("Description:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,130, 85,15);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    JXStaticText* obj8 =
        new JXStaticText("Mode:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,295, 85,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    mName =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,25, 160,20);
    assert( mName != NULL );
    mName->SetFontSize(10);
    mName->SetBorderWidth(kJXDefaultBorderWidth);

    mVersion =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,50, 160,20);
    assert( mVersion != NULL );
    mVersion->SetFontSize(10);
    mVersion->SetBorderWidth(kJXDefaultBorderWidth);

    mType =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,75, 160,20);
    assert( mType != NULL );
    mType->SetFontSize(10);
    mType->SetBorderWidth(kJXDefaultBorderWidth);

    mManufacturer =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,100, 160,20);
    assert( mManufacturer != NULL );
    mManufacturer->SetFontSize(10);
    mManufacturer->SetBorderWidth(kJXDefaultBorderWidth);

    mDescription =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,125, 160,165);
    assert( mDescription != NULL );
    mDescription->SetFontSize(10);
    mDescription->SetBorderWidth(kJXDefaultBorderWidth);

    mMode =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,295, 160,20);
    assert( mMode != NULL );
    mMode->SetFontSize(10);
    mMode->SetBorderWidth(kJXDefaultBorderWidth);

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 390,330, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mRegisterBtn =
        new JXTextButton("Register", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,330, 80,25);
    assert( mRegisterBtn != NULL );

// end JXLayout

	window->SetTitle("About Plug-ins");
	SetButtons(mOKBtn, NULL);

	mDescription->SetBreakCROnly(false);

	mList = new CAboutPluginsTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kVElastic,
										0,0, 10, 10);
	mList->OnCreate();

	// Hide registration button if already registered
	mRegisterBtn->Deactivate();
	ListenTo(mRegisterBtn);

	// Add items to table
	InitTable();
}

// Called during startup
void CAboutPluginsDialog::InitTable(void)
{
	// Subclass table
	mList->SetDlog(this);

	// Create columns and adjust flag rect
	mList->InsertCols(1, 1);
	mList->SetColWidth(mList->GetApertureWidth(), 1, 1);

	// Add types to list
	int parent = 0;
	const CPluginList& plugins = CPluginManager::sPluginManager.GetPlugins();

	for(int i = CPlugin::ePluginUnknown; i < CPlugin::ePluginLast; i++)
	{
		cdstring title = rsrc::GetIndexedString("Alerts::Plugins::Type", i);

		parent = mList->InsertSiblingRows(1, parent, NULL, 0, true);
		mList->SetCellString(parent, title);
		mHierPlugins.push_back(NULL);

		// Add suitable children
		for(CPluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		{
			if ((*iter)->GetType() == (CPlugin::EPluginType) i)
			{
				int child = mList->AddLastChildRow(parent, NULL, 0);
				mList->SetCellString(child, (*iter)->GetName());
				mHierPlugins.push_back(*iter);
			}
		}
		
		// Always expanded
		mList->ExpandRow(parent);
	}
}

void CAboutPluginsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == mRegisterBtn) && message.Is(JXButton::kPushed))
	{
		OnAboutPluginsRegister();
		return;
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CAboutPluginsDialog::OnAboutPluginsRegister() 
{
	STableCell aCell(0, 0);
	if (mList->GetNextSelectedCell(aCell))
	{
		CPlugin* plugin = mHierPlugins.at(mList->GetWideOpenIndex(aCell.row) - 1);
		StLoadPlugin load(plugin);
		plugin->DoRegistration(false, false);
		ShowPluginInfo(plugin);
	}
}

void CAboutPluginsDialog::OnClickAboutPluginsList() 
{
	STableCell aCell(0, 0);
	if (mList->GetNextSelectedCell(aCell))
		ShowPluginInfo(mHierPlugins.at(mList->GetWideOpenIndex(aCell.row) - 1));
	else
		ShowPluginInfo(NULL);
}

void CAboutPluginsDialog::ShowPluginInfo(const CPlugin* plugin)
{
	if (plugin)
	{
		mName->SetText(plugin->GetName());
		mVersion->SetText(::GetVersionText(plugin->GetVersion()));

		cdstring s = rsrc::GetIndexedString("Alerts::Plugins::Type", plugin->GetType());
		mType->SetText(s);
		mManufacturer->SetText(plugin->GetManufacturer());
		mDescription->SetText(plugin->GetDescription());
		
		if (plugin->IsRegistered())
			s = rsrc::GetString("Alerts::Plugins::ModeRegistered");
		else if (plugin->IsDemo())
			s = rsrc::GetString("Alerts::Plugins::ModeDemo");
		else
			s = rsrc::GetString("Alerts::Plugins::ModeUnregistered");
		mMode->SetText(s);
		
		if (plugin->IsDemo())
			mRegisterBtn->Activate();
		else
			mRegisterBtn->Deactivate();
	}
	else
	{
		mName->SetText(cdstring::null_str);
		mVersion->SetText(cdstring::null_str);
		mType->SetText(cdstring::null_str);
		mManufacturer->SetText(cdstring::null_str);
		mDescription->SetText(cdstring::null_str);
		mMode->SetText(cdstring::null_str);
		mRegisterBtn->Deactivate();
	}
}

bool CAboutPluginsDialog::PoseDialog()
{
	CAboutPluginsDialog* dlog = new CAboutPluginsDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
		dlog->Close();

	return true;
}

#pragma mark ____________________________CAboutPluginsTable

CAboutPluginsTable::CAboutPluginsTable(JXScrollbarSet* scrollbarSet,
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
	: LTextHierTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mDlog = NULL;
	SetTableGeometry(new LTableMonoGeometry(this, 175, 16));
	SetTableSelector(new LTableSingleSelector(this));
	
	mHierarchyCol = 1;
}

void CAboutPluginsTable::DoSelectionChanged()
{ 
	if (mDlog)
		mDlog->OnClickAboutPluginsList();
}
