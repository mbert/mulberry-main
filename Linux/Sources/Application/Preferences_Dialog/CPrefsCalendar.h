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


// CPrefsCalendar.h : header file
//

#ifndef __CPrefsCalendar__MULBERRY__
#define __CPrefsCalendar__MULBERRY__

#include "CPrefsPanel.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar dialog

class CDateTimeControl;
class JXTextCheckbox;
class JXTextPushButton;

class CPrefsCalendar : public CPrefsPanel
{
// Construction
public:
	CPrefsCalendar(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    HPopupMenu*       mWeekStartBtn;
    JXTextPushButton* mWorkDay0;
    JXTextPushButton* mWorkDay1;
    JXTextPushButton* mWorkDay2;
    JXTextPushButton* mWorkDay3;
    JXTextPushButton* mWorkDay4;
    JXTextPushButton* mWorkDay5;
    JXTextPushButton* mWorkDay6;
    JXTextCheckbox*   mHandleICS;
    JXTextCheckbox*   mAutomaticIMIP;
    JXTextCheckbox*   mDisplayTime;
    CDateTimeControl* mDT1;
    CDateTimeControl* mDT2;
    CDateTimeControl* mDT3;
    CDateTimeControl* mDT4;
    CDateTimeControl* mDT5;
    CDateTimeControl* mDT6;
    CDateTimeControl* mDT7;
    CDateTimeControl* mDT8;
    CDateTimeControl* mDT9;
    CDateTimeControl* mDT10;
    JXTextCheckbox*   mAutomaticEDST;

// end JXLayout1
	JXTextPushButton*	mWorkDay[7];
	CDateTimeControl*	mRangeStarts[5];
	CDateTimeControl*	mRangeEnds[5];

private:

	void SetHours(CDateTimeControl* ctrl, unsigned long hours);
	unsigned long GetHours(CDateTimeControl* ctrl);

};

#endif
