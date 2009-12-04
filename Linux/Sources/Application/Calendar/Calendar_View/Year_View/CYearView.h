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

#ifndef H_CYearView
#define H_CYearView

#include "CCalendarViewBase.h"

#include "CYearTable.h"

#include "HPopupMenu.h"

class JXMultiImageButton;
class CStaticText;

// ===========================================================================
//	CYearView

class	CYearView : public CCalendarViewBase
{
public:
			CYearView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CYearView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void ResetFont(const SFontInfo& finfo);			// Reset text traits

	virtual void DoPrint();

	NCalendarView::EYearLayout GetLayout() const
	{
		return mLayoutValue;
	}
	void SetLayout(NCalendarView::EYearLayout layout);

	virtual CCalendarTableBase* GetTable() const;

protected:
	NCalendarView::EYearLayout	mLayoutValue;

	// UI Objects
// begin JXLayout1

    JXMultiImageButton* mYearMinus;
    CStaticText*        mYear;
    JXMultiImageButton* mYearPlus;
    HPopupMenu*         mLayout;

// end JXLayout1
	CYearTable*			mTable;	

	virtual void		OnCreate();
	
	virtual void		ResetDate();
			void		SetCaptions();

			void		OnUpdateNewEvent(CCmdUI* pCmdUI);

			void		OnPreviousYear();
			void		OnNextYear();
			void		OnThisYear();
			void		OnNewEvent();
			void		OnLayout();
};

#endif
