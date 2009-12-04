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

#ifndef H_CSummaryView
#define H_CSummaryView

#include "CCalendarViewBase.h"

#include "CCalendarViewTypes.h"
#include "CSummaryTable.h"

class CSummaryTitleTable;
class CTimezonePopup;

#include "HPopupMenu.h"

class JXMultiImageButton;

// ===========================================================================
//	CSummaryView

class CSummaryView : public CCalendarViewBase
{
public:

			CSummaryView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CSummaryView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	NCalendarView::ESummaryType GetType() const
	{
		return mSummaryType;
	}
	void SetType(NCalendarView::ESummaryType type);

	NCalendarView::ESummaryRanges GetRange() const
	{
		return mSummaryRange;
	}
	void SetRange(NCalendarView::ESummaryRanges range);

	virtual CCalendarTableBase* GetTable() const;

	virtual void ResetFont(const SFontInfo& finfo);	// Reset text traits

protected:
	// UI Objects
// begin JXLayout1

    CTimezonePopup* mTimezonePopup;
    HPopupMenu*     mRangePopup;

// end JXLayout1

	CSummaryTitleTable*	mTitles;
	CSummaryTable*		mTable;
	
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	cdstring						mRangeText;
	
	virtual void		OnCreate();
	
	virtual	void		ResetDate();

			void		OnUpdateNewEvent(CCmdUI* pCmdUI);

			void		OnToday();
			void		OnNewEvent();
			void		OnTimezone();
			void		OnRange();
};

#endif
