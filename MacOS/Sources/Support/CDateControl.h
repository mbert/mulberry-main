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

#include "time.h"

// Constants

// Panes
const	PaneIDT		paneid_DateControl = 1012;
const	PaneIDT		paneid_DateControlText1 = 'TXT1';
const	PaneIDT		paneid_DateControlSeparator1 = 'SEP1';
const	PaneIDT		paneid_DateControlText2 = 'TXT2';
const	PaneIDT		paneid_DateControlSeparator2 = 'SEP2';
const	PaneIDT		paneid_DateControlText3 = 'TXT3';
const	PaneIDT		paneid_DateControlArrows = 'ARRW';

// Mesages
const	MessageT	msg_DateControlText1 = 'TXT1';
const	MessageT	msg_DateControlText2 = 'TXT2';
const	MessageT	msg_DateControlText3 = 'TXT3';
const	MessageT	msg_DateControlArrows = 'ARRW';

// Resources
const	ResIDT		RidL_CDateControlBtns = 1012;

// Classes
class CTextFieldX;
class CStaticText;
class LLittleArrows;

class	CDateControl : public LView,
							public LListener
{
private:
	CTextFieldX*		mText1;
	CStaticText*		mSeparator1;
	CTextFieldX*		mText2;
	CStaticText*		mSeparator2;
	CTextFieldX*		mText3;
	LLittleArrows*		mArrows;
	bool				mDayFirst;
	CTextFieldX*		mDay;
	CTextFieldX*		mMonth;

public:
	enum { class_ID = 'Date' };

					CDateControl();
					CDateControl(LStream *inStream);
	virtual 		~CDateControl();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void SetDate(time_t date);
	time_t GetDate() const;

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void InitDate();
};

#endif
