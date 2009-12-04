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
#define COPYP_MAX_MONTH		9

#include "CMulberryApp.h"

#include "CErrorHandler.h"
#include "CPluginManager.h"
#include "CRegistration.h"
#include "CRegistrationDialog.h"
#include "CXStringResources.h"

const char* cAppSignature = "cyrusoft-mulberry";

//#pragma mark ________________________Copy Protection

// Read in default resources
void CMulberryApp::ReadDefaults()
{
	// Get app registration info
	CRegistration::sRegistration.LoadAppInfo();
	mDemoTimeout = CRegistration::sRegistration.GetAppDemoTimeout();
}

// Get version
NumVersion CMulberryApp::GetVersionNumber()
{
	NumVersionVariant dummy;
	//NumVersion dummy = {0x02, 0x01, 0x20, 0x01};	// v2.0.1d1
	//NumVersion dummy = {0x04, 0x00, 0x40, 0x09};	// v4.0a9
	//NumVersion dummy = {0x04, 0x00, 0x60, 0x04};	// v4.0b4
	//NumVersion dummy = {0x04, 0x08, 0x80, 0x00};	// v4.0.8
	dummy.whole = 0x04104001;	// v4.1a1
	return dummy.parts;
}

// Get application signature
const JCharacter* CMulberryApp::GetAppSignature()
{
	return cAppSignature;
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
		long last_day = 0;
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
		!(CRegistration::sRegistration.CheckAppRegistration(true) && !IsDemo()) ||
		!CRegistration::sRegistration.ValidSerial(CRegistration::sRegistration.GetSerialNumber()))
	{
		// Try to re-register
		if (CRegistrationDialog::DoRegistration(initial))
		{
			if (IsDemo())
				return true;

			// Reload registration details
			CRegistration::sRegistration.LoadAppInfo();

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

	// Menu items are removed by CMainMenu when menus are constructed
	// Don't need to do it here
}
