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


// CPostponeGURL.cp

// Class to handle a postponed GURL Apple Event at Idle time

// Created 24-9-96

#include "CPostponeGURL.h"

#include "CCalendarStoreManager.h"
#include "CErrorDialog.h"
#include "CMailAccountManager.h"
#include "CMulberryApp.h"

// Static
bool CPostponeGURL::sPause = false;

// Methods

CPostponeGURL::CPostponeGURL(char *text, EType type)
{
	mText = text;
	mType = type;
	mFirstTime = true;
}

CPostponeGURL::~CPostponeGURL()
{
	delete mText;
}

// Run GURL Event then remove
void CPostponeGURL::SpendTime(const EventRecord &inMacEvent)
{
	// Only do if not paused
	if (!sPause)
	{
		if (mType == eMailto)
		{
			// Do dialog if first time and not already started servers
			if (mFirstTime && !CMailAccountManager::sMailAccountManager)
			{
				// Do this first to stop re-entrancy since idler is called by alert
				mFirstTime = false;

				// Display error/notification
				CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Note,
											"ErrorDialog::Btn::OK",
											NULL,
											NULL,
											NULL,
											"Alerts::General::WaitGURL");
			}

			// Run the GURL only if SMTP server exists
			if (CMailAccountManager::sMailAccountManager)
			{
				// Stop idling to prevent re-entrancy
				StopIdling();

				// Handle GURL now
				CMulberryApp::ProcessMailto(mText);

				// Delete this since it is one-shot
				delete this;
			}
		}
		else if (mType == eWebcal)
		{
			// Run the GURL only if SMTP server exists
			if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
			{
				// Stop idling to prevent re-entrancy
				StopIdling();

				// Handle GURL now
				CMulberryApp::ProcessWebcal(mText);

				// Delete this since it is one-shot
				delete this;
			}
		}
	}
}
