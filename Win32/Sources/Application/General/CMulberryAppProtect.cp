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


// CMulberryApp

#define REG_PROTECTION		0
#define DATE_PROTECTION		0
#define	COPYP_MAX_YEAR		2005
#define COPYP_MAX_MONTH		5



#include "CMulberryApp.h"

#include "CDynamicMenu.h"
#include "CErrorHandler.h"
#include "CPluginManager.h"
#include "CRegistration.h"
#include "CRegistrationDialog.h"
#include "CWinRegistry.h"
#include "CXStringResources.h"

#pragma mark ________________________Copy Protection

// Read in default resources
void CMulberryApp::ReadDefaults()
{
	// Get app registration info
	CRegistration::sRegistration.LoadAppInfo(AfxGetResourceHandle());
	mDemoTimeout = CRegistration::sRegistration.LoadAppTimeout(AfxGetResourceHandle());
}

// Get version
NumVersion CMulberryApp::GetVersionNumber()
{
	NumVersionVariant dummy;
	//dummy.whole = 0x04002001;	// v4.0d1
	dummy.whole = 0x04094001;	// v4.0.9a1
	//dummy.whole = 0x04006003;	// v4.0b3
	//dummy.whole = 0x04088000;	// v4.0.8
	return dummy.parts;
}

// Do copy protection checking
bool CMulberryApp::Protect()
{
	// Only if not done before
	if (mDoneProtect)
		return true;

#if DATE_PROTECTION
	time_t systime = ::time(NULL);
	struct tm* currtime = ::localtime(&systime);

	// Look for expirary
	if ((currtime->tm_year + 1900 > COPYP_MAX_YEAR) ||
		((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 > COPYP_MAX_MONTH)))
	{
		// Force quit if over time limit
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoMoreTime");
		return false;
	}
	
	// Look for expirary within a week
	else if ((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 == COPYP_MAX_MONTH))
	{
		// Check day
		bool alert_needed = false;
		unsigned long last_day = 0;
		switch(currtime->tm_mon + 1)
		{
		case 1:		// January
		case 3: 	// March
		case 5: 	// May
		case 7: 	// July
		case 8: 	// August
		case 10:	// October
		case 12:	// December
			last_day = 31;
			break;
		case 4:		// April
		case 6:		// June
		case 9:		// September
		case 11:	// December
			last_day = 30;
			break;
		case 2:		// February
			last_day = (currtime->tm_year % 4) ? 28 : 29;
			break;
		}
		
		if (currtime->tm_mday > last_day - 7)
		{
			// Put up alert warning of time left
			cdstring txt(rsrc::GetString("Alerts::General::Timeleft"));
			txt.Substitute(last_day - currtime->tm_mday);
			CErrorHandler::PutNoteAlert(txt);
		}
	}
#endif

#if REG_PROTECTION
	if (DoRegistration(true))
	{
#endif
		// Initialise plugins now
		PluginInit();
	
		mDoneProtect = true;
		return true;
#if REG_PROTECTION
	}
	else
		return false;
#endif
}

// Do registration dialog
bool CMulberryApp::DoRegistration(bool initial)
{
	// Check against actual CRC
	if (!initial ||
		!CRegistration::sRegistration.CheckAppRegistration(true) && !IsDemo() ||
		!CRegistration::sRegistration.ValidSerial(CRegistration::sRegistration.GetSerialNumber()))
	{
		// Try to re-register
		if (CRegistrationDialog::DoRegistration(initial))
		{
			if (IsDemo())
				return true;

			// Reload registration details
			CRegistration::sRegistration.LoadAppInfo(AfxGetResourceHandle());

			// Recalc. and match CRC
			if (CRegistration::sRegistration.CheckAppRegistration(true))
				return true;
		}
		mFailedProtect = true;
		return false;
	}
	
	return true;
}

// Do copy protection
void CMulberryApp::PluginInit()
{
	// Do plug-in search/registration here
	CPluginManager::sPluginManager.LookforPlugins();

	// Init main menus
	CMenu* menu = CMulberryApp::sApp->m_pMainWnd->GetMenu();

	CMenu* edit_menu = menu->GetSubMenu(1);
	CDynamicMenu::CreateEditMenu(edit_menu, false);
	
	CMenu* mbox_menu = menu->GetSubMenu(2);
	CDynamicMenu::CreateMailboxMenu(mbox_menu, false);

	CMenu* msgs_menu = menu->GetSubMenu(3);
	CDynamicMenu::CreateMessagesMenu(msgs_menu, false);

	//CMenu* draft_menu = menu->GetSubMenu(4);
	//CDynamicMenu::CreateDraftMenu(draft_menu, false);
	
	// Add import/export menu items
	CMenu* addr_menu = menu->GetSubMenu(4);
	for(int i = 0; i < addr_menu->GetMenuItemCount(); i++)
	{
		if (addr_menu->GetMenuItemID(i) == IDM_ADDR_RENAME)
		{
			CMenu* import_menu = addr_menu->GetSubMenu(i + 2);
			CMenu* export_menu = addr_menu->GetSubMenu(i + 3);
			CPluginManager::sPluginManager.SyncAdbkIOPluginMenu(import_menu, export_menu);
			break;
		}
	}

	CMenu* help_menu = menu->GetSubMenu(7);
	CDynamicMenu::CreateHelpMenu(help_menu);
}
