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


// Header for CNetworkResetError class

#ifndef __CNETWORKRESETERROR__MULBERRY__
#define __CNETWORKRESETERROR__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_NetworkResetError = 10006;
const	PaneIDT		paneid_NetworkResetDisconnectBtn = 'DISC';
const	PaneIDT		paneid_NetworkResetDisconnectText = 'DIST';

// Resources

class CNetworkResetError : public LDialogBox
{
public:
	enum { class_ID = 'NetR' };

					CNetworkResetError();
					CNetworkResetError(LStream *inStream);
	virtual 		~CNetworkResetError();

	static bool	PoseDialog(bool allow_disconnect);

protected:
	EventLoopTimerRef mTimer;
	bool			  mIdleExit;

	virtual void		FinishCreateSelf();

	void	SetAllowDisconnect(bool allow_disconnect);

	static pascal	void IdleTimer(EventLoopTimerRef inTimer, void * inUserData);
	void	Idle();
};

#endif
