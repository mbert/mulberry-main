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

#include "CWindow.h"
#include "CListener.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarProperty.h"

#include "cdmutexprotect.h"

// Classes
class CCalendarView;
class CEventPreview;
class CSplitterView;
class CToolbarView;

namespace calstore
{
	class CCalendarStoreNode;
};

class	CCalendarWindow : public CWindow,
							public CListener
{
public:
	typedef std::vector<CCalendarWindow*>	CCalendarWindowList;
	static cdmutexprotect<CCalendarWindowList> sCalendarWindows;	// List of windows (protected for multi-thread access)

	CCalendarWindow(JXDirector* owner);
	virtual 		~CCalendarWindow();

	static void MakeWindow(calstore::CCalendarStoreNode* node);
	static void CreateFreeBusyWindow(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);
	static void CreateSubscribedWindow();
	static void CloseAllWindows();

	static	CCalendarWindow* FindWindow(const calstore::CCalendarStoreNode* node);
	static bool	WindowExists(const CCalendarWindow* wnd);		// Check for window

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	CCalendarView* GetCalendarView() const
	{
		return mCalendarView;
	}

	CSplitterView* GetSplitter() const
	{
		return mSplitter;
	}

	CToolbarView* GetToolbarView() const
		{ return mToolbarView; }

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

protected:
// begin JXLayout


// end JXLayout

	CToolbarView*					mToolbarView;
	CSplitterView*					mSplitter;							// Splitter window
	CCalendarView*					mCalendarView;
	CEventPreview*					mPreview;
	bool							mPreviewVisible;
	calstore::CCalendarStoreNode*	mNode;

	virtual void OnCreate();
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

			void 	SetNode(calstore::CCalendarStoreNode* node);
			void 	DeleteNode(calstore::CCalendarStoreNode* node);
			void 	RefreshNode(calstore::CCalendarStoreNode* node);
			void 	SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);

			void	ShowPreview(bool preview);

	// Command updaters
	void	OnUpdateShowPreview(CCmdUI* pCmdUI);
	void	OnUpdateShowPreviewBtn(CCmdUI* pCmdUI);

	// message handlers
};

#endif
