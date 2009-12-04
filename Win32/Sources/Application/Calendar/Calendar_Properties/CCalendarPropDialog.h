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


// CCalendarPropDialog.h : header file
//

#ifndef __CCalendarPropDialog__MULBERRY__
#define __CCalendarPropDialog__MULBERRY__

#include "CHelpPropertySheet.h"
#include "CPropCalendarOptions.h"
#include "CPropCalendarGeneral.h"
#include "CPropCalendarServer.h"
#include "CPropCalendarWebcal.h"
#include "CPropCalendarACL.h"

/////////////////////////////////////////////////////////////////////////////
// CCalendarPropDialog dialog

namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CCalendarPropDialog : public CHelpPropertySheet
{
	DECLARE_DYNAMIC(CCalendarPropDialog)

// Construction
public:
	CCalendarPropDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list)
		{ return PoseDialog(cal_list, NULL); }
	static bool PoseDialog(calstore::CCalendarProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(calstore::CCalendarStoreNodeList* cal_list, calstore::CCalendarProtocol* proto);

	//virtual void BuildPropPageArray();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCalendarPropDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG

private:
	calstore::CCalendarStoreNodeList*	mCalList;										// List of selected calendars
	calstore::CCalendarProtocol*		mCalProtocol;									// Selected protocol

	CPropCalendarOptions	mPropCalendarOptions;
	CPropCalendarGeneral	mPropCalendarGeneral;
	CPropCalendarServer		mPropCalendarServer;
	CPropCalendarWebcal		mPropCalendarWebcal;
	CPropCalendarACL		mPropCalendarACL;

	void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);						// Set cal list
	void	SetProtocol(calstore::CCalendarProtocol* proto);							// Set cal protocol

	DECLARE_MESSAGE_MAP()
};

#endif
