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


// Header for CBalloonDialog class

#ifndef __CBALLOONDIALOG__MULBERRY__
#define __CBALLOONDIALOG__MULBERRY__

#include <UModalDialogs.h>

// Classes

class	LPane;

class	CBalloonDialog : public StDialogHandler
{
// Methods
public:
					CBalloonDialog(ResIDT inDialogResID, LCommander	*inSuper, bool blocking = false);
	virtual 		~CBalloonDialog();

	virtual void	SetCritical(bool critical)
						{ mCritical = critical; }

	virtual void	StartDialog(void);
	virtual void	EndDialog(void);

	virtual MessageT	DoModal();
	virtual MessageT	DoDialog();

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

protected:
	//virtual void	DispatchEvent(const EventRecord &inMacEvent);
	virtual void	AdjustCursor(const EventRecord &inMacEvent);	// OVERRIDE

protected:
	bool				mCritical;				// Flag to indicate limited event processing
	bool				mBlocking;				// Prevent periodics
	LEventDispatcher* 	mSaveNextDispatcher;	//
};

class	CCriticalBalloonDialog : public CBalloonDialog
{
// Methods
public:
					CCriticalBalloonDialog(ResIDT inDialogResID, LCommander	*inSuper);
	virtual 		~CCriticalBalloonDialog();

protected:
	virtual void	DispatchEvent(const EventRecord &inMacEvent);

};

#endif
