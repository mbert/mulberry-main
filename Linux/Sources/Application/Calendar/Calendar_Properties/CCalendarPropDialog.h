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

#include "ptrvector.h"

// Classes
namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}
class JXTextButton;
class JXIconTextButton;


class CCalendarPropDialog : public CPropDialog
{
public:
	CCalendarPropDialog(JXDirector* supervisor);

	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list)
		{ return PoseDialog(cal_list, NULL); }
	static bool PoseDialog(calstore::CCalendarProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list, calstore::CCalendarProtocol* proto);

protected:
// begin JXLayout

    JXIconTextButton* mOptionsBtn;
    JXIconTextButton* mGeneralBtn;
    JXIconTextButton* mServerBtn;
    JXIconTextButton* mWebCalBtn;
    JXIconTextButton* mAccessBtn;
    JXCardFile*       mCards;
    JXTextButton*     mOKBtn;

// end JXLayout
	virtual void	OnCreate();								// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	SetUpPanels(void);									// About to start dialog
	virtual void	DoPanelInit(void);									// About to display a panel

private:
	calstore::CCalendarStoreNodeList*	mCalList;										// List of selected calendars
	calstore::CCalendarProtocol*		mCalProtocol;									// Selected protocol

	void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);						// Set cal list
	void	SetProtocol(calstore::CCalendarProtocol* proto);							// Set cal protocol
};

#endif
