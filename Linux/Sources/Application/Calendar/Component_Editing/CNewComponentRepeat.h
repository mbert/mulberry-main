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

#ifndef H_CNewComponentRepeat
#define H_CNewComponentRepeat

#include "CNewComponentPanel.h"
#include "CTabPanel.h"

#include "CICalendarRecurrence.h"

#include "HPopupMenu.h"

class CNewComponentRepeatSimple;
class CNewComponentRepeatAdvanced;
class CNewComponentRepeatComplex;

class CDateTimeZoneSelect;
class CNewTimingPanel;
class CNumberEdit;

class CTabController;
class JXRadioGroup;
class JXStaticText;
class JXTextCheckbox;
class JXTextRadioButton;
class JXTextButton;

// ===========================================================================
//	CNewComponentRepeat

class CNewComponentRepeat : public CNewComponentPanel
{
public:

	CNewComponentRepeat(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
		: CNewComponentPanel(enclosure, hSizing, vSizing, x, y, w, h)
		{ }
	virtual				~CNewComponentRepeat() {}

	virtual void	OnCreate();

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual	void	SetReadOnly(bool read_only);
			
protected:
	enum
	{
		eOccurs_Simple = 1,
		eOccurs_Advanced,
		eOccurs_Complex
	};

	enum
	{
		eOccurs_Yearly = 1,
		eOccurs_Monthly,
		eOccurs_Weekly,
		eOccurs_Daily,
		eOccurs_Hourly,
		eOccurs_Minutely,
		eOccurs_Secondly
	};

	enum
	{
		eOccurs_ForEver = 1,
		eOccurs_Count,
		eOccurs_Until
	};

	// UI Objects
// begin JXLayout1

    JXTextCheckbox* mRepeats;
    CTabController* mRepeatsTabs;

// end JXLayout1
	CNewComponentRepeatSimple*		mRepeatSimpleItems;
	CNewComponentRepeatAdvanced*	mRepeatAdavancedItems;
	CNewComponentRepeatComplex*		mRepeatComplexItems;

	iCal::CICalendarRecurrence	mAdvancedRecur;
	cdstring				mComplexDescription;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
	const CNewTimingPanel* GetTimingPanel() const;

			void	DoRepeat(bool repeat);
			void	DoRepeatTab(JIndex value);
			void	DoOccursGroup(JIndex value);
			void	DoOccursEdit();
			
			void	SetRecurrence(const iCal::CICalendarRecurrenceSet* recurs);
			
			bool	GetRecurrence(iCal::CICalendarRecurrenceSet& recurs);
};

class CNewComponentRepeatSimple : public CTabPanel
{
public:
	CNewComponentRepeatSimple(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CNewComponentRepeatSimple() {}

	virtual void	OnCreate();
	virtual void	SetData(void* data) {}			// Set data
	virtual bool	UpdateData(void* data) { return true; }		// Force update of data

	// UI Objects
// begin JXLayout2

    CNumberEdit*         mOccursInterval;
    HPopupMenu*          mOccursFreq;
    JXRadioGroup*        mOccursGroup;
    JXTextRadioButton*   mOccursForEver;
    JXTextRadioButton*   mOccursCount;
    JXTextRadioButton*   mOccursUntil;
    CNumberEdit*         mOccursCounter;
    CDateTimeZoneSelect* mOccursDateTimeZone;

// end JXLayout2
};

class CNewComponentRepeatAdvanced : public CTabPanel
{
public:
	CNewComponentRepeatAdvanced(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CNewComponentRepeatAdvanced() {}

	virtual void	OnCreate();
	virtual void	SetData(void* data) {}			// Set data
	virtual bool	UpdateData(void* data) { return true; }		// Force update of data

	// UI Objects
// begin JXLayout3

    JXStaticText* mOccursDescription;
    JXTextButton* mOccursEdit;

// end JXLayout3

protected:
    CNewComponentRepeat*	mTimingPanel;
};

class CNewComponentRepeatComplex : public CTabPanel
{
public:
	CNewComponentRepeatComplex(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CNewComponentRepeatComplex() {}

	virtual void	OnCreate();
	virtual void	SetData(void* data) {}			// Set data
	virtual bool	UpdateData(void* data) { return true; }		// Force update of data

	// UI Objects
// begin JXLayout4

    JXStaticText* mOccursDescription;

// end JXLayout4
};

#endif
