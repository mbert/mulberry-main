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


// Header for CCalendarWindow class

#ifndef __CCalendarWindow__MULBERRY__
#define __CCalendarWindow__MULBERRY__

#include "LWindow.h"
#include "LDragAndDrop.h"
#include "CListener.h"
#include "CWindowStatus.h"
#include "CHelpTags.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarProperty.h"

#include "cdmutexprotect.h"

// Panes
const	PaneIDT		paneid_CalendarWindow = 1830;
const	PaneIDT		paneid_CalendarToolbarView = 'TBar';
const	PaneIDT		paneid_CalendarSplitterView = 'SPLT';
const	PaneIDT		paneid_CalendarView = 'VIEW';
const	PaneIDT		paneid_CalendarPreview = 'PREV';

// Constants

// Resources

// Classes
class CCalendarView;
class CEventPreview;
class CSplitterView;
class CToolbarView;

namespace calstore
{
	class CCalendarStoreNode;
};

class	CCalendarWindow : public LWindow,
							public LDragAndDrop,
							public CListener,
						 	public CWindowStatus,
							public CHelpTagWindow
{
public:
	typedef std::vector<CCalendarWindow*>	CCalendarWindowList;
	static cdmutexprotect<CCalendarWindowList> sCalendarWindows;	// List of windows (protected for multi-thread access)

	enum { class_ID = 'CaWi' };

					CCalendarWindow(LStream *inStream);
	virtual 		~CCalendarWindow();

	static void MakeWindow(calstore::CCalendarStoreNode* node);
	static void CreateFreeBusyWindow(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);
	static void CreateSubscribedWindow();
	static void CloseAllWindows();

	static	CCalendarWindow* FindWindow(const calstore::CCalendarStoreNode* node);
	static bool	WindowExists(const CCalendarWindow* wnd);		// Check for window

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	CCalendarView* GetCalendarView() const
	{
		return mCalendarView;
	}

	CSplitterView* GetSplitter() const
	{
		return mSplitter;
	}

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

	void	GetUserBounds(Rect& user_bounds) const
		{ user_bounds = mUserBounds; }

protected:
	CToolbarView*					mToolbarView;
	CSplitterView*					mSplitter;							// Splitter window
	CCalendarView*					mCalendarView;
	CEventPreview*					mPreview;
	bool							mPreviewVisible;
	calstore::CCalendarStoreNode*	mNode;

	virtual void	FinishCreateSelf(void);

			void 	SetNode(calstore::CCalendarStoreNode* node);
			void 	DeleteNode(calstore::CCalendarStoreNode* node);
			void 	RefreshNode(calstore::CCalendarStoreNode* node);
			void 	SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);

			void	ShowPreview(bool preview);
};

#endif
