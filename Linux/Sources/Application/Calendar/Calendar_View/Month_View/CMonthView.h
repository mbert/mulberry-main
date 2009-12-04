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

#ifndef H_CMONTHVIEW
#define H_CMONTHVIEW

#include "CCalendarViewBase.h"

class CCalendarEventBase;
class CMonthTable;
class CMonthTitleTable;
class CStaticText;
class CTimezonePopup;

class JXMultiImageButton;

namespace calstore
{
	class CCalendarStoreNode;
}


// ===========================================================================
//	CMonthView

class	CMonthView : public CCalendarViewBase
{
public:
			CMonthView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CMonthView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void ResetFont(const SFontInfo& finfo);			// Reset text traits

	virtual void DoPrint();

	virtual CCalendarTableBase* GetTable() const;

protected:

	// UI Objects
// begin JXLayout1

    JXMultiImageButton* mMonthMinus;
    CStaticText*        mMonth;
    JXMultiImageButton* mMonthPlus;
    JXMultiImageButton* mYearMinus;
    CStaticText*        mYear;
    JXMultiImageButton* mYearPlus;
    CTimezonePopup*     mTimezonePopup;

// end JXLayout1
	CMonthTitleTable*	mTitles;
	CMonthTable*		mTable;

	virtual void		OnCreate();
	
	virtual	void		ResetDate();
			void		SetCaptions();

			void		OnUpdateNewEvent(CCmdUI* pCmdUI);

			void		OnPreviousMonth();
			void		OnNextMonth();
			void		OnPreviousYear();
			void		OnNextYear();
			void		OnToday();
			void		OnNewEvent();
			void		OnTimezone();
};

#endif
