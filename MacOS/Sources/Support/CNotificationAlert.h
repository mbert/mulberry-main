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


// Header for CNotificationAlert class

#ifndef __CNOTIFICATIONALERT__MULBERRY__
#define __CNOTIFICATIONALERT__MULBERRY__

#include "cdstring.h"

// Classes

class CNotificationAlert : public LPeriodical
{
public:
	struct SNotificationAlert
	{
		Str255 txt;
		bool doForegroundAlert;
		bool doBackgroundAlert;
		bool flashIcon;
		bool playSound;
		cdstring soundID;
		
		SNotificationAlert(const Str255 theTxt, bool fore, bool back, bool flash, bool play, const cdstring& sound);
		SNotificationAlert(const SNotificationAlert& copy);
	};

					CNotificationAlert(SNotificationAlert& notify);
	virtual 		~CNotificationAlert();
	
	virtual	void	SpendTime(const EventRecord &inMacEvent);

protected:
	NMRec				mNotification;
	SNotificationAlert	mNotify;
	bool				mHasBackground;
	Handle				mIconSuite;
	Handle				mSnd;

	virtual void	DoForegroundAlert() = 0;
	virtual void	DoBackgroundAlert();
};

class CNotificationStopAlert : public CNotificationAlert {

public:
					CNotificationStopAlert(SNotificationAlert& notify) :
						CNotificationAlert(notify) {}
	virtual 		~CNotificationStopAlert() {}

protected:
	virtual void	DoForegroundAlert();
};

class CNotificationNoteAlert : public CNotificationAlert {

public:
					CNotificationNoteAlert(SNotificationAlert& notify) :
						CNotificationAlert(notify) {}
	virtual 		~CNotificationNoteAlert() {}

protected:
	virtual void	DoForegroundAlert();
};


#endif
