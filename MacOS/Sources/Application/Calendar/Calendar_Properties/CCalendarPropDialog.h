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


// Header for CCalendarPropDialog class

#ifndef __CCalendarPropDialog__MULBERRY__
#define __CCalendarPropDialog__MULBERRY__

#include "CPropDialog.h"

// Constants

// Panes
const	PaneIDT		paneid_CalendarPropDialog = 1840;
const	PaneIDT		paneid_CalendarPropOptionsBtn = 'BTN1';
const	PaneIDT		paneid_CalendarPropGeneralBtn = 'BTN2';
const	PaneIDT		paneid_CalendarPropServerBtn = 'BTN3';
const	PaneIDT		paneid_CalendarPropWebcalBtn = 'BTN4';
const	PaneIDT		paneid_CalendarPropACLBtn = 'BTN5';

// Resources
const	ResIDT		RidL_CCalendarPropDialogBtns = 1840;

// Mesages
const	MessageT	msg_SetPropCalOptions = 'BTN1';
const	MessageT	msg_SetPropCalGeneral = 'BTN2';
const	MessageT	msg_SetPropCalServer = 'BTN3';
const	MessageT	msg_SetPropCalWebcal = 'BTN4';
const	MessageT	msg_SetPropCalACL = 'BTN5';

// Classes
namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CCalendarPropDialog : public CPropDialog
{
public:
	enum { class_ID = 'CPrt' };

					CCalendarPropDialog(LStream *inStream);
	virtual 		~CCalendarPropDialog();

	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list)
		{ return PoseDialog(cal_list, NULL); }
	static bool PoseDialog(calstore::CCalendarProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list, calstore::CCalendarProtocol* proto);

protected:
	virtual void	FinishCreateSelf(void);								// Do odds & ends

	virtual void	SetUpPanels(void);									// About to start dialog
	virtual void	DoPanelInit(void);									// About to display a panel

private:
	calstore::CCalendarStoreNodeList*	mCalList;										// List of selected calendars
	calstore::CCalendarProtocol*		mCalProtocol;									// Selected protocol

	void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);						// Set cal list
	void	SetProtocol(calstore::CCalendarProtocol* proto);							// Set cal protocol
};

#endif
