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


// Source for CCalendarWindow class

#include "CCalendarWindow.h"

#include "CCalendarView.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"

#include <algorithm>

CCalendarWindow* CCalendarWindow::FindWindow(const calstore::CCalendarStoreNode* node)
{
	cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
	for(CCalendarWindowList::const_iterator iter = sCalendarWindows->begin(); iter != sCalendarWindows->end(); iter++)
	{
		if (node == (*iter)->mNode)
			return *iter;
	}

	return NULL;
}

// Check for window
bool CCalendarWindow::WindowExists(const CCalendarWindow* wnd)
{
	cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
	CCalendarWindowList::iterator found = std::find(sCalendarWindows->begin(), sCalendarWindows->end(), wnd);
	return found != sCalendarWindows->end();
}

void CCalendarWindow::ListenTo_Message(long msg, void* param)
{
	switch (msg)
	{
	case calstore::CCalendarStoreManager::eBroadcast_DeleteNode:
		DeleteNode(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	case calstore::CCalendarStoreManager::eBroadcast_InsertNode:
		RefreshNode(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	}
}

// Reset state from prefs
void CCalendarWindow::ResetState(bool force)
{
	// Pass down to the view object which handles this
	GetCalendarView()->ResetState(force);
}

// Save current state in prefs
void CCalendarWindow::SaveState()
{
	// Pass down to the view object which handles this
	GetCalendarView()->SaveState();
}

// Save current state in prefs
void CCalendarWindow::SaveDefaultState()
{
	// Pass down to the view object which handles this
	GetCalendarView()->SaveDefaultState();
}

void CCalendarWindow::SetNode(calstore::CCalendarStoreNode* node)
{
	// Cache node and listen to it
	mNode = node;
	calstore::CCalendarStoreManager::sCalendarStoreManager->Add_Listener(this);

	// Set title etc
	RefreshNode(mNode);

	// Mark as single calendar if node is present
	if (node != NULL)
		GetCalendarView()->SetSingleCalendar();

	// Give calendar to view
	GetCalendarView()->SetCalendar((mNode != NULL) ? mNode->GetCalendar() : NULL);
}

void CCalendarWindow::DeleteNode(calstore::CCalendarStoreNode* node)
{
	// Only if its our node
	if (mNode != node)
		return;

	mNode = NULL;

	// Give calendar to view
	GetCalendarView()->SetCalendar(NULL);
	
	FRAMEWORK_DELETE_WINDOW(this)
}

void CCalendarWindow::RefreshNode(calstore::CCalendarStoreNode* node)
{
	// Only if its our node
	if (mNode != node)
		return;

	// Set title
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	cdstring title((mNode != NULL) ? cdstring(mNode->GetDisplayShortName()) : rsrc::GetString("UI::Calendar::SubscribedTitle"));
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

#elif __dest_os == __win32_os

	cdstring title((mNode != NULL) ? cdstring(mNode->GetDisplayShortName()) : rsrc::GetString("UI::Calendar::SubscribedTitle"));
	GetDocument()->SetTitle(title.win_str());

#elif __dest_os == __linux_os

#endif
}

void CCalendarWindow::SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date)
{
	// Give calendar to view
	GetCalendarView()->SetFreeBusy(calref, id, organizer, attendees, date);
}

