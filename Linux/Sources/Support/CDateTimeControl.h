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


// CDateTimeControl.h - UI widget that implements a date control

#ifndef __CDateTimeControl__MULBERRY__
#define __CDateTimeControl__MULBERRY__

#include "CTextField.h"

// Classes
class JXMultiImageButton;

class CDateTimeControl : public CTextInputField
{
public:
	enum EDateTimeMode
	{
		eDate,
		eDateDDMMYYYY,
		eDateMMDDYYYY,
		eTimeSecs,
		eTimeNoSecs,
		e12TimeSecs,
		e12TimeNoSecs,
		e24TimeSecs,
		e24TimeNoSecs
	};
	
	enum EFieldType
	{
		eDay,
		eMonth,
		eYear,
		eHour,
		eMinute,
		eSecond,
		eAMPM
	};

				CDateTimeControl(const JCharacter* text, JXContainer* enclosure,
								JXTextMenu* menu,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);

				CDateTimeControl(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);

	virtual 		~CDateTimeControl() {}

	void OnCreate(EDateTimeMode dtmode);					// Do odds & ends

	void SetMode(EDateTimeMode dtmode);

	void SetDate(unsigned long year, unsigned long month, unsigned long mday);
	void GetDate(unsigned long& year, unsigned long& month, unsigned long& mday) const;

	void SetTime(unsigned long hours, unsigned long mins, unsigned long secs);
	void GetTime(unsigned long& hours, unsigned long& mins, unsigned long& secs) const;

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual void	Show();					// must call inherited
	virtual void	Hide();					// must call inherited
	virtual void	Activate();				// must call inherited
	virtual void	Deactivate();			// must call inherited

protected:
	JXMultiImageButton*			mSpinUp;
	JXMultiImageButton*			mSpinDown;
	EDateTimeMode				mMode;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
	void InitDate();
	
	void HandleNumber(const int key);
	void HandleAMPM(const int key);
	void ShiftSelection(bool forward);
	void Nudge(bool up);
	unsigned long NormaliseDay(unsigned long month, unsigned long mday) const;
	EFieldType GetCurrentField(JIndex& sel_start, JIndex& sel_end) const;
	void SelectField(EFieldType type);
};

#endif
