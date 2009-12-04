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


// Header for CWindowOptionsDialog class

#ifndef __CWINDOWOPTIONSDIALOG__MULBERRY__
#define __CWINDOWOPTIONSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CUserAction.h"
#include "C3PaneOptions.h"
#include "C3PaneWindowFwd.h"

// Constants

// Panes
const	PaneIDT		paneid_WindowOptionsDialog = 1790;
const	PaneIDT		paneid_WindowOptionsTabs = 'TABS';
const	PaneIDT		paneid_WindowOptionsView = 'VIEW';

// Mesages
const	MessageT	msg_WindowOptionsTab = 'TABS';

// Resources
const	ResIDT		RidL_CWindowOptionsDialogBtns = 1790;

// Type
class CTabController;
class CMessagePaneOptions;
class CAddressPaneOptions;
class CCalendarPaneOptions;

class CWindowOptionsDialog : public LDialogBox
{
private:

public:
	enum { class_ID = 'Wopt' };

					CWindowOptionsDialog();
					CWindowOptionsDialog(LStream *inStream);
	virtual 		~CWindowOptionsDialog();

	static bool 	PoseDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	CTabController*			mTabs;
	CMessagePaneOptions*	mMessage;
	CAddressPaneOptions*	mAddress;
	CCalendarPaneOptions*	mCalendar;

	N3Pane::EViewType		mCurrentView;
	CUserAction				mUserActions[N3Pane::eView_Total][C3PaneOptions::C3PaneViewOptions::eUserAction_Total];		// User actions to trigger viewing
	CMailViewOptions		mMailOptions;
	CAddressViewOptions		mAddressOptions;
	CCalendarViewOptions	mCalendarOptions;
	bool					mIs3Pane;

	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	void	SetViewType(N3Pane::EViewType view);
	
	void	SetDetails(C3PaneOptions* options, bool is3pane);
	void	GetDetails(C3PaneOptions* options);
};

#endif
