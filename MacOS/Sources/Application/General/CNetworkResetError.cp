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


// Source for CNetworkResetError class

#include "CNetworkResetError.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTCPSocket.h"

EventLoopTimerUPP CNetworkResetError_IdleTimerUPP = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNetworkResetError::CNetworkResetError()
{
}

// Constructor from stream
CNetworkResetError::CNetworkResetError(LStream *inStream)
		: LDialogBox(inStream)
{
	mIdleExit = false;
	mTimer = NULL;
}

// Default destructor
CNetworkResetError::~CNetworkResetError()
{
	// remove our timer, if any
	if (mTimer != NULL)
	{
		::RemoveEventLoopTimer(mTimer);
		mTimer = NULL;
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CNetworkResetError::FinishCreateSelf()
{
	// Need idle check of doing network auto-reconnect
	if (CPreferences::sPrefs->mSleepReconnect.GetValue())
	{
		//	create timer UPP first time through
		if (CNetworkResetError_IdleTimerUPP == NULL)
		{
			CNetworkResetError_IdleTimerUPP = NewEventLoopTimerUPP(IdleTimer);
		}

		// install an event loop timer for blinking the caret
		::InstallEventLoopTimer(GetMainEventLoop(), 0.0, 1.0, CNetworkResetError_IdleTimerUPP, this, &mTimer);
	}
}

void CNetworkResetError::SetAllowDisconnect(bool allow_disconnect)
{
	// Hide disconnect text & button if not wanted
	if (!allow_disconnect)
	{
		FindPaneByID(paneid_NetworkResetDisconnectBtn)->Hide();
		FindPaneByID(paneid_NetworkResetDisconnectText)->Hide();
	}
}

pascal void CNetworkResetError::IdleTimer(EventLoopTimerRef inTimer, void * inUserData)
{
	static_cast<CNetworkResetError*>(inUserData)->Idle();

	SetEventLoopTimerNextFireTime(inTimer, 1.0);
}

void CNetworkResetError::Idle()
{
	mIdleExit = CTCPSocket::CheckConnectionState();
}

bool CNetworkResetError::PoseDialog(bool allow_disconnect)
{
	bool result = false;

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_NetworkResetError, CMulberryApp::sApp);
		CNetworkResetError* dlog = (CNetworkResetError*) theHandler.GetDialog();
		dlog->SetAllowDisconnect(allow_disconnect);
		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{
			hitMessage = theHandler.DoDialog();

			if (dlog->mIdleExit || (hitMessage == msg_OK))
			{
				result = false;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				result = true;
				break;
			}
		}
	}
	
	return result;
}
