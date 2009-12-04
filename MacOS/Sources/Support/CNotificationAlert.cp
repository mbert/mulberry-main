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


// Source for CNotificationAlert class

#include "CNotificationAlert.h"

#include "CErrorDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

// __________________________________________________________________________________________________
// C L A S S __ C N O T I F I C A T I O N A L E R T
// __________________________________________________________________________________________________

CNotificationAlert::SNotificationAlert::SNotificationAlert(const Str255 theTxt, bool fore, bool back, bool flash, bool play, const cdstring& sound)
{
	::PLstrcpy(txt, theTxt);
	doForegroundAlert = fore;
	doBackgroundAlert = back;
	flashIcon = flash;
	playSound = play;
	soundID = sound;
}

CNotificationAlert::SNotificationAlert::SNotificationAlert(const SNotificationAlert& copy)
{
	::PLstrcpy(txt, copy.txt);
	doForegroundAlert = copy.doForegroundAlert;
	doBackgroundAlert = copy.doBackgroundAlert;
	flashIcon = copy.flashIcon;
	playSound = copy.playSound;
	soundID = copy.soundID;
}

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor
CNotificationAlert::CNotificationAlert(SNotificationAlert& notify) :
	mNotify(notify)
{
	// Copy text
	mHasBackground = false;
	mIconSuite = nil;
	mSnd = nil;

	// Disable apps periodics
	CMulberryApp::sApp->ErrorPause(true);

	// Start this idling
	StartRepeating();

	// If suspended do background alert now
	if (CMulberryApp::sSuspended)
	{
		DoBackgroundAlert();

		// Must make sure that any foreground alert is not displayed if background was displayed
		if (mNotify.doBackgroundAlert)
			mNotify.doForegroundAlert = false;

		// Must make sure that any sound is not played if background sound was played
		if (mNotify.playSound)
			mNotify.playSound = false;
	}
}

// Default destructor
CNotificationAlert::~CNotificationAlert()
{
	// Remove any notification
	if (mHasBackground)
		::NMRemove(&mNotification);

	if (mIconSuite)
	{
		::DisposeIconSuite(mIconSuite, true);
		mIconSuite = nil;
	}

	if (mSnd)
	{
		::ReleaseResource(mSnd);
		mSnd = nil;
	}

	// Enable apps periodics
	CMulberryApp::sApp->ErrorPause(false);

}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Do periodic action
void CNotificationAlert::SpendTime(const EventRecord &inMacEvent)
{
	// Wait till its not suspended
	if (!CMulberryApp::sSuspended)
	{
		// Stop repeating first as alert may call back to periodical
		StopRepeating();

		// Do sound
		if (mNotify.playSound)
			::PlayNamedSound(mNotify.soundID);

		// Do foreground alert
		if (mNotify.doForegroundAlert)
			DoForegroundAlert();

		// Now delete to stop periodic
		delete this;
	}
}

// Do background notification
void CNotificationAlert::DoBackgroundAlert()
{
	// Get icon to flash
	if (mNotify.flashIcon)
		if (GetIconSuite(&mIconSuite, 128, svAllSmallData) != noErr)
			mIconSuite = nil;

	// Get sound to play
	if (mNotify.playSound)
	{
		// OS X Notifications cannot use 'snd ' resources so instead we
		// now play the sound directly when the notification is posted
		// rather than letting the Notification manager do it
#if 0
		if (mNotify.soundID.length())
		{
			LStr255 title;;
			title = mNotify.soundID;
			mSnd = ::GetNamedResource('snd ', title);
		}
		else
			mSnd = (Handle) -1L;
#else
		::PlayNamedSound(mNotify.soundID);
#endif
	}

	mNotification.qType = nmType;					// set queue type
	mNotification.nmMark = mNotify.flashIcon;		// put mark in Application menu
	mNotification.nmIcon = mIconSuite;				// alternating icon
	mNotification.nmSound = mSnd;					// play system alert sound if requested
	mNotification.nmStr = (mNotify.doBackgroundAlert && *mNotify.txt) ? mNotify.txt : nil;	// do not display alert box
	mNotification.nmResp = nil;					// no response procedure
	mNotification.nmRefCon = 0;					// not needed

	OSErr err = ::NMInstall(&mNotification);		// Install it
	mHasBackground = true;
}

// __________________________________________________________________________________________________
// C L A S S __ C N O T I F I C A T I O N S T O P A L E R T
// __________________________________________________________________________________________________

// Do foreground notification
void CNotificationStopAlert::DoForegroundAlert()
{
	cdstring temp(mNotify.txt);
	short answer = CErrorDialog::StopAlert(temp, mNotify.playSound);
}

// __________________________________________________________________________________________________
// C L A S S __ C N O T I F I C A T I O N S T O P A L E R T
// __________________________________________________________________________________________________

// Do foreground notification
void CNotificationNoteAlert::DoForegroundAlert()
{
	cdstring temp(mNotify.txt);
	short answer = CErrorDialog::NoteAlert(temp, NULL, NULL, mNotify.playSound);
}

