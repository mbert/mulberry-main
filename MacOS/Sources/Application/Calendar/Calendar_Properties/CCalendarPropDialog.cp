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


// Source for CCalendarPropDialog class

#include "CCalendarPropDialog.h"

#include "CBalloonDialog.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPropCalendarACL.h"
#include "CPropCalendarGeneral.h"
#include "CPropCalendarOptions.h"
#include "CPropCalendarServer.h"
#include "CPropCalendarWebcal.h"

#include "MyCFString.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarPropDialog::CCalendarPropDialog(LStream *inStream)
		: CPropDialog(inStream)
{
}

// Default destructor
CCalendarPropDialog::~CCalendarPropDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCalendarPropDialog::FinishCreateSelf(void)
{
	// Do inherited
	CPropDialog::FinishCreateSelf();
}

// Set up panels
void CCalendarPropDialog::SetUpPanels(void)
{
	// Panel list
	mPanelList.push_back(paneid_PropCalendarOptions);
	mPanelList.push_back(paneid_PropCalendarGeneral);
	mPanelList.push_back(paneid_PropCalendarServer);
	mPanelList.push_back(paneid_PropCalendarWebcal);
	mPanelList.push_back(paneid_PropCalendarACL);

	// Message list
	mMsgPanelList.push_back(msg_SetPropCalOptions);
	mMsgPanelList.push_back(msg_SetPropCalGeneral);
	mMsgPanelList.push_back(msg_SetPropCalServer);
	mMsgPanelList.push_back(msg_SetPropCalWebcal);
	mMsgPanelList.push_back(msg_SetPropCalACL);

	// Ridl for buttons
	mRidl = RidL_CCalendarPropDialogBtns;
}

// About to show panel
void CCalendarPropDialog::DoPanelInit(void)
{
	if (mCalList)
		((CCalendarPropPanel*) mCurrentPanel)->SetCalList(mCalList);
	if (mCalProtocol)
		((CCalendarPropPanel*) mCurrentPanel)->SetProtocol(mCalProtocol);
}

// Set input panel
void CCalendarPropDialog::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	mCalList = cal_list;
	mCalProtocol = cal_list->front()->GetProtocol();

	// Load string title
	cdstring title;
	title.FromResource((mCalList->size() == 1 ? "UI::CalendarProp::Title" : "UI::CalendarProp::MultiTitle"));

	// Add name if single calendar
	if (mCalList->size() == 1)
	{
		title += mCalList->front()->GetName();
	}

	// Now set window title
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// Check webcal state
	int local = 0;
	int webcal = 0;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;

		if (!node->IsDirectory())
		{
			if (node->GetProtocol()->IsLocalCalendar())
				local++;

			if (node->GetProtocol()->IsWebCalendar())
				webcal++;
		}
	}
	
	// Must have server cals in list
	if ((webcal == mCalList->size()) || (local == mCalList->size()))
	{
		FindPaneByID(paneid_CalendarPropServerBtn)->Disable();
	}

	// Must have only webcals in the entire list
	if ((webcal == 0) || (webcal != mCalList->size()) || (local == mCalList->size()))
	{
		FindPaneByID(paneid_CalendarPropWebcalBtn)->Disable();
	}

	// Disable unwanted prefs panels
	bool logged_in = mCalProtocol->IsLoggedOn();
	if (!logged_in || !mCalProtocol->UsingACLs())
	{
		FindPaneByID(paneid_CalendarPropACLBtn)->Disable();
	}

	// Initial panel
	long set = 1;
	ListenToMessage(msg_SetPropCalOptions, &set);
}

// Set input panel
void CCalendarPropDialog::SetProtocol(calstore::CCalendarProtocol* proto)
{
	mCalList = NULL;
	mCalProtocol = proto;

	// Load string title
	cdstring title;
	title.FromResource("UI::CalendarProp::ServerTitle");

	// Add name if single calendar
	title += proto->GetAccountName();

	// Now set window title
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// Disable unwanted prefs panels
	FindPaneByID(paneid_CalendarPropOptionsBtn)->Disable();
	FindPaneByID(paneid_CalendarPropGeneralBtn)->Disable();
	FindPaneByID(paneid_CalendarPropWebcalBtn)->Disable();
	FindPaneByID(paneid_CalendarPropACLBtn)->Disable();

	// Initial panel
	FindPaneByID(paneid_CalendarPropServerBtn)->SetValue(1);
}

bool CCalendarPropDialog::PoseDialog(calstore::CCalendarStoreNodeList* cal_list, calstore::CCalendarProtocol* proto)
{
	CBalloonDialog	dlog(paneid_CalendarPropDialog, CMulberryApp::sApp);
	CCalendarPropDialog* prop_dlog = (CCalendarPropDialog*) dlog.GetDialog();

	if (cal_list)
		prop_dlog->SetCalList(cal_list);
	else if (proto)
		prop_dlog->SetProtocol(proto);

	// Let DialogHandler process events
	dlog.DoModal();

	return true;
}