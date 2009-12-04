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

#include "CCalendarEventTableBase.h"

#include "CCalendarViewBase.h"
#include "CCommands.h"
#include "CErrorDialog.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarVEvent.h"
#include "CITIPProcessor.h"

#include <JXWindowPainter.h>
#include <jASCIIConstants.h>
#include <jXUtil.h>

#include <algorithm>

// ---------------------------------------------------------------------------
//	CCalendarEventTableBase														  [public]
/**
	Default constructor */

CCalendarEventTableBase::CCalendarEventTableBase(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CCalendarTableBase(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}


// ---------------------------------------------------------------------------
//	~CCalendarEventTableBase														  [public]
/**
	Destructor */

CCalendarEventTableBase::~CCalendarEventTableBase()
{
}

#pragma mark -

//	Respond to commands
bool CCalendarEventTableBase::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	// Pass these to selected item if present
	case CCommand::eFileNewMessage:
	case CCommand::eCalendarNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eToolbarNewLetterOption:
		OnNewDraft(cmd == CCommand::eToolbarNewLetterOption);
		return true;
		
	case CCommand::eCalendarEditItem:
		OnEditEvent();
		return true;

	case CCommand::eCalendarDuplicateItem:
		OnDuplicateEvent();
		return true;

	case CCommand::eCalendarDeleteItem:
		OnDeleteEvent();
		return true;

	case CCommand::eCalendarInvite:
		OnInviteAttendees();
		return true;

	default:;
	};

	return CCalendarTableBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CCalendarEventTableBase::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eCalendarNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eToolbarNewLetterOption:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eCalendarEditItem:
	case CCommand::eCalendarDuplicateItem:
		OnUpdateSelectedEvent(cmdui);
		return;

	case CCommand::eCalendarDeleteItem:
		OnUpdateDeleteEvent(cmdui);
		return;

	case CCommand::eCalendarInvite:
		OnUpdateInviteAttendees(cmdui);
		return;

	default:;
	}
	
	CCalendarTableBase::UpdateCommand(cmd, cmdui);
}

void CCalendarEventTableBase::OnUpdateSelectedEvent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mSelectedEvents.size() != 0);
}

void CCalendarEventTableBase::OnUpdateDeleteEvent(CCmdUI* pCmdUI)
{
	// Look at all events - any read-onloy prevent delete
	bool enabled = (mSelectedEvents.size() != 0);

	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);

	// Edit each one
	for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		iCal::CICalendarRef calref = static_cast<iCal::CICalendarVEvent*>(*iter)->GetCalendar();
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
		if ((cal == NULL) || cal->IsReadOnly())
		{
			enabled = false;
			break;
		}
	}

	pCmdUI->Enable(enabled);
}

void CCalendarEventTableBase::OnUpdateInviteAttendees(CCmdUI* pCmdUI)
{
	// Can only send one at a time
	if (mSelectedEvents.size() == 1)
	{
		// Look at all events
		bool enabled = false;

		// Get the unqiue original components
		iCal::CICalendarComponentRecurs vevents;
		GetSelectedMasterEvents(vevents);

		// Edit each one
		for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
		{
			// See whether iTIP processing is needed for any ATTENDEEs in the component
			if (iCal::CITIPProcessor::NeedsITIPRequest(**iter))
				enabled = true;
		}

		pCmdUI->Enable(enabled);
	}
	else
		pCmdUI->Enable(false);
}

bool CCalendarEventTableBase::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Return => edit
	switch(key)
	{
	case kJTabKey:
		SelectEvent(!modifiers.shift());
		break;

	case kJReturnKey:
		// Edit event if selected
		if (mSelectedEvents.size() != 0)
		{
			OnEditEvent();
			return true;			
		}
		// Switch to next view
		else if (IsSelectionValid())
		{
			SelectDay();
		}
		break;
	case kJDeleteKey:
	case kJForwardDeleteKey:
		// Special case escape key
		if (mSelectedEvents.size() != 0)
		{
			OnDeleteEvent();
			return true;
		}
		break;
	default:;
	}
	
	return CCalendarTableBase::HandleChar(key, modifiers);
}

// Click
void CCalendarEventTableBase::HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Handle scroll wheel
	if (ScrollForWheel(button, modifiers))
		return;

	// Remove selection from any selected items
	UnselectAllEvents();

	CCalendarTableBase::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void CCalendarEventTableBase::ClickEvent(CCalendarEventBase* item, bool shift)
{
	// Make sure the table is the target
	SetTarget(this);

	// Always remove table cell selection
	UnselectAllCells();
	
	// If not shifting always remove other selections
	if (shift)
	{
		// Add or remove the chosen item without affecting others
		if (item->IsSelected())
			UnselectEvent(item);
		else
			SelectEvent(item);
	}
	else
	{
		// Remove existing selection
		UnselectAllEvents();

		// Add the new one
		SelectEvent(item);
	}
}

// This is overridden because the sub-panes (events etc) are not rectangular,
// so the table needs to be drawn first to get the correct background, then the enclosed widgets
// are drawn with transparency to ensure it all looks OK.
void CCalendarEventTableBase::DrawAll(JXWindowPainter& p, const JRect& origFrameG)
{
#if 1
	// Got portion of frame that is visible
	JRect clipRectG = GetFrameGlobal();
	if (!JIntersection(clipRectG, origFrameG, &clipRectG))
	{
		return;
	}

	// Get portion of aperture that is visible
	// NB Frame may exceed aperture, so frame may be visible, but aperture not
	JRect apClipRectG = GetApertureGlobal();
	const JBoolean apVisible = JIntersection(apClipRectG, origFrameG, &apClipRectG);

	Region visRegion = NULL;
	// prepare the painter to draw

	const JRect boundsG = GetBoundsGlobal();

	p.SetOrigin(boundsG.left, boundsG.top);
	p.JPainter::Reset(clipRectG);

	// draw border before using visRegion

	const JRect frameLocal = JXContainer::GlobalToLocal(GetFrameGlobal());
	DrawBorder(p, frameLocal);

	// draw background and contents, if visible

	if (visRegion != NULL && XEmptyRegion(visRegion))
	{
		// If nothing else is visible, we can quit now.

		XDestroyRegion(visRegion);
		return;
	}
	else if (visRegion != NULL)
	{
		// convert visRegion to local coordinates

		XOffsetRegion(visRegion, -boundsG.left, -boundsG.top);
	}

	if (apVisible)
	{
		if (visRegion != NULL)
		{
			p.Reset(apClipRectG, visRegion);
		}
		else
		{
			p.JPainter::Reset(apClipRectG);
		}

		const JRect apLocal = GetAperture();
		DrawBackground(p, apLocal);

		// draw contents clipped to aperture, if visible

		JRect boundsClipRectG;
		if (JIntersection(boundsG, apClipRectG, &boundsClipRectG))
		{
			p.ResetAllButClipping();
			Draw(p, JXContainer::GlobalToLocal(boundsClipRectG));
		}
	}

#if 0
	// draw enclosed objects last, if visible

	if (apVisible && itsEnclosedObjs != NULL)
	{
		XRectangle xClipRect = JXJToXRect(apClipRectG);
		visRegion = JXRectangleRegion(&xClipRect);

		// draw visible objects in reverse order so all the
		// other routines can check them in the normal order

		const JSize objCount = itsEnclosedObjs->GetElementCount();
		for (JIndex i=objCount; i>=1; i--)
		{
			JXContainer* obj = itsEnclosedObjs->NthElement(i);
			if (obj->IsVisible())
			{
				obj->DrawAll(p, apClipRectG);

				// update region that we need to draw to

				XRectangle xFrame = JXJToXRect(obj->GetFrameGlobal());
				JXSubtractRectFromRegion(visRegion, &xFrame, visRegion);
			}
		}
	}
#else
	CCalendarTableBase::DrawAll(p, origFrameG);
#endif

	// clean up

	if (visRegion != NULL)
	{
		XDestroyRegion(visRegion);
	}
#else
	CCalendarTableBase::DrawAll(p, origFrameG);
#endif
}
