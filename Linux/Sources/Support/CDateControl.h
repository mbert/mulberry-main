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


// CDateControl.h - UI widget that implements a date control

#ifndef __CDATECONTROL__MULBERRY__
#define __CDATECONTROL__MULBERRY__

#include <JXWidgetSet.h>

// Classes
template <class T> class CInputField;
class JXInputField;
class JXStaticText;

class CDateControl : public JXWidgetSet
{
public:
					CDateControl(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CDateControl();

	void SetDate(time_t date);
	time_t GetDate() const;

protected:
// begin JXLayout1

    CInputField<JXInputField>* mText1;
    JXStaticText*              mSeparator1;
    CInputField<JXInputField>* mText2;
    JXStaticText*              mSeparator2;
    CInputField<JXInputField>* mText3;

// end JXLayout1
	bool						mDayFirst;
	CInputField<JXInputField>*	mDay;
	CInputField<JXInputField>*	mMonth;

	void OnCreate();					// Do odds & ends

	void InitDate();
};

#endif
