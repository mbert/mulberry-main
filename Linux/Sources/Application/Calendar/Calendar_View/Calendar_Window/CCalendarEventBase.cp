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

#include "CCalendarEventBase.h"

#include "CActionManager.h"
#include "CCalendarTableBase.h"
#include "CCalendarUtils.h"
#include "CCalendarViewBase.h"
#include "CCommands.h"
#include "CErrorDialog.h"
#include "CIconLoader.h"
#include "CListener.h"
#include "CMulberryCommon.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CTextBox.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarUtils.h"
#include "CICalendarVFreeBusy.h"
#include "CITIPProcessor.h"

#include "StPenState.h"

#include <JXColormap.h>
#include <JXImage.h>
#include <JXWindowPainter.h>

#include <strstream>

// ---------------------------------------------------------------------------
//	CCalendarEventBase														  [public]
/**
	Default constructor */

CCalendarEventBase::CCalendarEventBase(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CCommander(enclosure), JXWidget(enclosure, hSizing, vSizing, x, y, w, h), CContextMenu(this)
{
	SetBorderWidth(0);

	mVFreeBusy = NULL;
	mTable = NULL;
	mAllDay = true;
	mStartsInCol = true;
	mEndsInCol = true;
	mIsSelected = false;
	mIsCancelled = false;
	mIsNow = false;
	mHoriz = true;
	mColumnSpan = 1;
	mPreviousLink = NULL;
	mNextLink = NULL;
	mColour = 0;
	mIsInbox = false;
}

// ---------------------------------------------------------------------------
//	~CCalendarEventBase														  [public]
/**
	Destructor */

CCalendarEventBase::~CCalendarEventBase()
{
}

#pragma mark -

void CCalendarEventBase::OnCreate()
{
	CreateContextMenu(CMainMenu::eContextCalendarEventItem);
}

void CCalendarEventBase::Select(bool select)
{
	if (mIsSelected != select)
	{
		mIsSelected = select;

		// Force immediate redraw
		Redraw();
	}
}

const char cTickMarkU2713_UTF8[] = 
{
	0xE2, 0x9C, 0x93, 0x00
};

const char cCrossMarkU2713_UTF8[] = 
{
	0xE2, 0x9C, 0x93, 0x00
};

void CCalendarEventBase::SetDetails(iCal::CICalendarComponentExpandedShared& event, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz)
{
	mVEvent = event;
	mPeriod = iCal::CICalendarPeriod(mVEvent->GetInstanceStart(), mVEvent->GetInstanceEnd());
	mTable = table;
	if (event->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Confirmed)
	{
		mTitle = cTickMarkU2713_UTF8;
	}
	else if (event->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Tentative)
	{
		mTitle = "?? ";
	}
	mTitle += title;
	mAllDay = all_day;
	mStartsInCol = start_col;
	mEndsInCol = end_col;
	mHoriz = horiz;
	mIsCancelled = (event->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Cancelled);
	mHasAlarm = event->GetMaster<iCal::CICalendarVEvent>()->HasEmbeddedComponent(iCal::CICalendarComponent::eVALARM);
	mAttendeeState = iCal::CITIPProcessor::GetAttendeeState(*event->GetMaster<iCal::CICalendarVEvent>());

	// Setup a help tag
	SetupTagText();
	
	// Determine colour
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(event->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
	if (cal)
	{
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}

	// Check for inbox
	const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(cal);
	mIsInbox = node->IsInbox();
}

void CCalendarEventBase::SetDetails(iCal::CICalendarVFreeBusy* freebusy, const iCal::CICalendarPeriod& period, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz)
{
	mVFreeBusy = freebusy;
	mPeriod = period;
	mTable = table;

	mTitle = title;
	mAllDay = all_day;
	mStartsInCol = start_col;
	mEndsInCol = end_col;
	mHoriz = horiz;
	mIsCancelled = false;
	mHasAlarm = false;
	mAttendeeState = iCal::CITIPProcessor::GetAttendeeState(*mVFreeBusy);

	// Setup a help tag
	SetupTagText();

	// Determine colour
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVFreeBusy->GetCalendar());
	if (cal)
	{
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}
}

void CCalendarEventBase::Receive (JBroadcaster* sender, const Message& message) 
{
	// Check for context menu
	if (!ReceiveMenu(sender, message))
		// Do default action
		JXWidget::Receive(sender, message);
}

// Click
void CCalendarEventBase::HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Make sure it is selected
	if (modifiers.shift() | modifiers.control())
	{
		SClickEvent_Message _msg(this, true);
		Broadcast_Message(eBroadcast_ClickEvent, &_msg);
	}
	else if (!mIsSelected)
	{
		SClickEvent_Message _msg(this, false);
		Broadcast_Message(eBroadcast_ClickEvent, &_msg);
	}
	else
		mTable->Focus();

	// Broadcast double-click
	if ((clickCount > 1) && (button == kJXLeftButton))
	{
		Broadcast_Message(eBroadcast_EditEvent, this);
	}
	
	if ((clickCount == 1) && (button == kJXRightButton))
		ContextEvent(pt, buttonStates, modifiers);
}

void CCalendarEventBase::Draw(JXWindowPainter& p, const JRect& rect)
{
	StPenState		dc(&p);
	JRect			image_rect(GetBounds());
	JRect			actual_rect(image_rect);

	if (mHoriz)
		DrawHorizFrame(&p, actual_rect);
	else
		DrawVertFrame(&p, actual_rect);
	
	// Draw title
	actual_rect.left += 3;
	actual_rect.right -= (mHoriz && mAllDay || IsFreeBusy()) ? 3 : 0;
	JRect cliprect(actual_rect);
	if (cliprect.height() < 16)
	{
		JCoordinate height_adjust = (16 - cliprect.height()) / 2;
		cliprect.top -= height_adjust;
		cliprect.bottom = cliprect.top + 16;
	}

	JFontStyle style;
	float red = CCalendarUtils::GetRed(mColour);
	float green = CCalendarUtils::GetGreen(mColour);
	float blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
	{
		style.color = (red + green + blue > 2.5) ? p.GetColormap()->GetBlackColor() : p.GetColormap()->GetWhiteColor();
	}
	else
	{
		CCalendarUtils::DarkenColours(red, green, blue);
		JColorIndex cindex;
		JRGB fill = CCalendarUtils::GetRGBColor(red, green, blue);
		GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
		style.color = cindex;
	}
	p.SetPenColor(style.color);
	p.SetFont(CPreferences::sPrefs->mListTextFontInfo.GetValue().fontname, CPreferences::sPrefs->mListTextFontInfo.GetValue().size, style);

	if (mHoriz)
		::DrawClippedStringUTF8(&p, mTitle, JPoint(cliprect.left, cliprect.top), cliprect, (mAllDay || IsFreeBusy()) ? eDrawString_Center : eDrawString_Left);
	else
	{
		CTextBox::DrawText(&p, mTitle, cliprect, IsFreeBusy() ? eDrawString_Center : eDrawString_Left);
	}
	
	// Strike out text if status is cancelled
	if (mIsCancelled)
	{
		actual_rect = image_rect;
		actual_rect.Shrink(4, 4);
		if (mHoriz)
		{
			p.Line(actual_rect.left, (actual_rect.top + actual_rect.bottom) / 2, actual_rect.right, (actual_rect.top + actual_rect.bottom) / 2);
		}
		else
		{
			p.Line(actual_rect.left, actual_rect.top, actual_rect.right, actual_rect.bottom);
			p.Line(actual_rect.left, actual_rect.bottom, actual_rect.right, actual_rect.top);
		}
	}
}

void CCalendarEventBase::DrawHorizFrame(JXWindowPainter* pDC, JRect& rect)
{
	// Adjust for ends
	if (mStartsInCol)
	{
		rect.left += 3;
	}
	else
	{
		rect.left += 1;
	}
	if (mEndsInCol)
		rect.right -= 2;
	else
		rect.right -= 1;
	rect.bottom -= 1;
	
	// Use unsaturated colour for selected item
	float red = CCalendarUtils::GetRed(mColour);
	float green = CCalendarUtils::GetGreen(mColour);
	float blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
	{
		if (IsFreeBusy())
			red = green = blue = 1.0;
	}
	else
	{
		CCalendarUtils::LightenColours(red, green, blue);
		if (IsFreeBusy())
			red = green = blue = 1.0;
	}

	// Fill with appropriate colour first
	JColorIndex cindex;
	JRGB fill = CCalendarUtils::GetRGBColor(red, green, blue);
	uint32_t bkgnd = CCalendarUtils::GetCalColor(fill);
	GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
	pDC->SetPenColor(cindex);
	pDC->SetFilling(kTrue);
	DrawHorizFrameFill(pDC, rect);
	pDC->SetFilling(kFalse);
	
	// Draw outline with appropriate colour next
	fill = (!mAllDay && mIsNow) ? CCalendarUtils::GetRGBColor(0.95, 0.0, 0.0) : CCalendarUtils::GetRGBColor(red * 0.6, green * 0.6, blue * 0.6);
	GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
	pDC->SetPenColor(cindex);
	DrawHorizFrameOutline(pDC, rect);

	// Adjust for round edges clip
	if (rect.height() < 20)
	{
		rect.left += 3;
		rect.right -= 3;
	}

	// Display alarm indicator
	if (mHasAlarm)
	{
		JXImage* icon = CIconLoader::GetIcon(IDI_HASALARM, this, 16, bkgnd);
		pDC->JPainter::Image(*icon, icon->GetBounds(), rect.right - 17, rect.top);
		
		// Offset right edge of text
		rect.right -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		resType theResID;
		switch(mAttendeeState)
		{
		case iCal::CITIPProcessor::eNone:
			theResID = IDI_ATTENDEE;
			break;
		case iCal::CITIPProcessor::eAllAccepted:
			theResID = IDI_ATTENDEE;
			break;
		case iCal::CITIPProcessor::eSomeAccepted:
			theResID = IDI_NEEDSACTION;
			break;
		case iCal::CITIPProcessor::eSomeDeclined:
			theResID = IDI_DECLINED;
			break;
		case iCal::CITIPProcessor::eIHaveAccepted:
			theResID = IDI_ATTENDEEISME;
			break;
		case iCal::CITIPProcessor::eIHaveNotAccepted:
			theResID = IDI_NEEDSACTION;
			break;
		}

		JXImage* icon = CIconLoader::GetIcon(theResID, this, 16, bkgnd);
		pDC->JPainter::Image(*icon, icon->GetBounds(), rect.right - 17, rect.top);
		
		// Offset right edge of text
		rect.right -= 16;
	}
}

void CCalendarEventBase::DrawHorizFrameOutline(JXWindowPainter* pDC, JRect& rect)
{
	// Draw left end (rounded if starts, flat otherwise)
	if (mStartsInCol)
	{
		if (mAllDay)
		{
			pDC->Arc(rect.left, rect.top, rect.height(), rect.height() + 1, -90.0, -180.0);
			pDC->SetPenLocation(rect.left + rect.height() / 2, rect.top);
		}
		else
		{
			pDC->SetPenLocation(rect.left, rect.bottom);
			pDC->LineTo(rect.left, rect.top);
		}
	}
	else
	{
		pDC->SetPenLocation(rect.left + rect.height() / 2, rect.bottom);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->LineTo(rect.left + rect.height() / 6, rect.top +  (rect.height() * 5) / 6);
		pDC->LineTo(rect.left, rect.top + (rect.height() * 4) / 6);
		pDC->LineTo(rect.left + rect.height() / 6, rect.top + (rect.height() * 3) / 6);
		pDC->LineTo(rect.left, rect.top + (rect.height() * 2) / 6);
		pDC->LineTo(rect.left + rect.height() / 6, rect.top + rect.height() / 6);
		pDC->LineTo(rect.left, rect.top);
	}

	// Draw top line
	if (mAllDay)
		pDC->LineTo(rect.right - rect.height() / 2, rect.top);
	else
		pDC->LineTo(rect.right, rect.top);

	// Draw right end (rounded if starts, flat otherwise)
	if (mEndsInCol)
	{
		if (mAllDay)
		{
			pDC->Arc(rect.right - rect.height(), rect.top, rect.height(), rect.height() + 1, 90.0, -180.0);
			pDC->SetPenLocation(rect.right - rect.height() / 2, rect.bottom);
		}
		else
		{
			pDC->LineTo(rect.right, rect.bottom);
		}
	}
	else
	{
		pDC->LineTo(rect.right - rect.height() / 6, rect.top);
		pDC->LineTo(rect.right, rect.top + rect.height() / 6);
		pDC->LineTo(rect.right - rect.height() / 6, rect.top + (rect.height() * 2) / 6);
		pDC->LineTo(rect.right, rect.top + (rect.height() * 3) / 6);
		pDC->LineTo(rect.right - rect.height() / 6, rect.top + (rect.height() * 4) / 6);
		pDC->LineTo(rect.right, rect.top + (rect.height() * 5) / 6);
		pDC->LineTo(rect.right - rect.height() / 6, rect.top + rect.height());
	}

	// Draw bottom line
	if (mAllDay)
		pDC->LineTo(rect.left + rect.height() / 2, rect.bottom);
	else
		pDC->LineTo(rect.left, rect.bottom);
}

void CCalendarEventBase::DrawHorizFrameFill(JXWindowPainter* pDC, JRect& rect)
{
	// Draw left end (rounded if starts, flat otherwise)
	if (mStartsInCol)
	{
		if (mAllDay)
		{
			pDC->Arc(rect.left, rect.top, rect.height(), rect.height(), -90.0, -180.0);
		}
		else
		{
			pDC->Rect(rect.left, rect.top, rect.height() / 2, rect.height());
		}
	}
	else
	{
		JPolygon poly;
		
		poly.AppendElement(JPoint(rect.height() / 2, rect.height()));
		poly.AppendElement(JPoint(0, rect.width()));
		poly.AppendElement(JPoint(rect.height() / 6, (rect.height() * 5) / 6));
		poly.AppendElement(JPoint(0, (rect.height() * 4) / 6));
		poly.AppendElement(JPoint(rect.height() / 6, (rect.height() * 3) / 6));
		poly.AppendElement(JPoint(0, (rect.height() * 2) / 6));
		poly.AppendElement(JPoint(rect.height() / 6, rect.height() / 6));
		poly.AppendElement(JPoint(0, 0));
		poly.AppendElement(JPoint(rect.height() / 2, 0));
		poly.AppendElement(JPoint(rect.height() / 2, rect.height()));
		
		pDC->Polygon(rect.left, rect.top, poly);
	}

	// Draw middle section
	pDC->Rect(rect.left +  rect.height() / 2, rect.top, rect.width() - rect.height() + 1, rect.height());

	// Draw right end (rounded if starts, flat otherwise)
	if (mEndsInCol)
	{
		if (mAllDay)
		{
			pDC->Arc(rect.right - rect.height(), rect.top, rect.height(), rect.height(), 90.0, -180.0);
		}
		else
		{
			pDC->Rect(rect.right - rect.height() / 2, rect.top, rect.height() / 2, rect.height());
		}
	}
	else
	{
		JPolygon poly;
		
		poly.AppendElement(JPoint( - rect.height() / 2, 0));
		poly.AppendElement(JPoint( - rect.height() / 6, 0));
		poly.AppendElement(JPoint(0, rect.height() / 6));
		poly.AppendElement(JPoint( - rect.height() / 6, (rect.height() * 2) / 6));
		poly.AppendElement(JPoint(0, (rect.height() * 3) / 6));
		poly.AppendElement(JPoint( - rect.height() / 6, (rect.height() * 4) / 6));
		poly.AppendElement(JPoint(0, (rect.height() * 5) / 6));
		poly.AppendElement(JPoint( - rect.height() / 6, rect.height()));
		poly.AppendElement(JPoint( - rect.height() / 2, rect.height()));
		poly.AppendElement(JPoint( - rect.height() / 2, 0));
		
		pDC->Polygon(rect.right, rect.top, poly);
	}
}

const JCoordinate cRoundRadius = 8;
const JCoordinate cJaggedEdgeHeight = 6;

void CCalendarEventBase::DrawVertFrame(JXWindowPainter* pDC, JRect& rect)
{
	rect.Shrink(1, 0);
	rect.bottom -= 1;

	if (IsFreeBusy())
		rect.Shrink(3, 3);

	// Use unsaturated colour for selected item
	float red = CCalendarUtils::GetRed(mColour);
	float green = CCalendarUtils::GetGreen(mColour);
	float blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
	{
		if (IsFreeBusy())
			red = green = blue = 1.0;
	}
	else
	{
		CCalendarUtils::LightenColours(red, green, blue);
		if (IsFreeBusy())
			red = green = blue = 1.0;
	}
	
	// Fill with appropriate colour first
	JColorIndex cindex;
	JRGB fill = CCalendarUtils::GetRGBColor(red, green, blue);
	uint32_t bkgnd = CCalendarUtils::GetCalColor(fill);
	GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
	pDC->SetPenColor(cindex);
	pDC->SetFilling(kTrue);
	DrawVertFrameFill(pDC, rect);
	pDC->SetFilling(kFalse);
	
	// Draw outline with appropriate colour next
	fill = (!mAllDay && mIsNow) ? CCalendarUtils::GetRGBColor(0.95, 0.0, 0.0) : CCalendarUtils::GetRGBColor(red * 0.6, green * 0.6, blue * 0.6);
	GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
	pDC->SetPenColor(cindex);
	DrawVertFrameOutline(pDC, rect);

	
	// Adjust rect for jaggies/round corners
	if (mStartsInCol)
	{
		rect.top += 2;
	}
	else
	{
		rect.top += cJaggedEdgeHeight;
	}
	if (mEndsInCol)
	{
		rect.bottom -= 2;
	}
	else
	{
		rect.bottom -= cJaggedEdgeHeight;
	}

	// Display alarm indicator
	if (mHasAlarm)
	{
		JCoordinate vOffset = rect.top;
		if (rect.height() < 16)
			vOffset = (rect.top + rect.bottom) / 2 - 8;
		JXImage* icon = CIconLoader::GetIcon(IDI_HASALARM, this, 16, bkgnd);
		pDC->JPainter::Image(*icon, icon->GetBounds(), rect.right - 17, vOffset);
		
		// Offset right edge of text
		rect.right -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		// Determine whether to draw alongside or below any alarm
		JCoordinate horiz_offset = (mHasAlarm && (rect.height() >= 32)) ? 16 : 0;
		JCoordinate vert_offset = (mHasAlarm && (rect.height() >= 32)) ? 16 : 0;
		JCoordinate vOffset = rect.top;
		if (rect.height() < 16)
			vOffset = (rect.top + rect.bottom) / 2 - 8;

		resType theResID;
		switch(mAttendeeState)
		{
		case iCal::CITIPProcessor::eNone:
			theResID = IDI_ATTENDEE;
			break;
		case iCal::CITIPProcessor::eAllAccepted:
			theResID = IDI_ATTENDEE;
			break;
		case iCal::CITIPProcessor::eSomeAccepted:
			theResID = IDI_NEEDSACTION;
			break;
		case iCal::CITIPProcessor::eSomeDeclined:
			theResID = IDI_DECLINED;
			break;
		case iCal::CITIPProcessor::eIHaveAccepted:
			theResID = IDI_ATTENDEEISME;
			break;
		case iCal::CITIPProcessor::eIHaveNotAccepted:
			theResID = IDI_NEEDSACTION;
			break;
		}

		JXImage* icon = CIconLoader::GetIcon(theResID, this, 16, bkgnd);
		pDC->JPainter::Image(*icon, icon->GetBounds(), rect.right - 17 + horiz_offset, rect.top);
		
		// Offset right edge of text
		rect.right -= 16;
	}
}

void CCalendarEventBase::DrawVertFrameOutline(JXWindowPainter* pDC, JRect& rect)
{
	JCoordinate h_radius = rect.height() >= 16 ? cRoundRadius : rect.height() / 2;
	JCoordinate w_radius = rect.width() >= 16 ? cRoundRadius : rect.width() / 2;
	JCoordinate radius = std::min(h_radius, w_radius);
	
	if (mStartsInCol)
	{
		// Top-left corner
		pDC->Arc(rect.left, rect.top, 2 * radius, 2 * radius, 180.0, -90.0);
		
		// Top line
		pDC->Line(rect.left + radius, rect.top, rect.right - radius, rect.top);

		// Top-right corner
		pDC->Arc(rect.right - 2 * radius, rect.top, 2 * radius, 2 * radius, 90.0, -90.0);
		pDC->SetPenLocation(rect.right, rect.top + radius);
	}
	else
	{
		// Draw jagged top edge
		pDC->SetPenLocation(rect.left, rect.top + radius);
		uint32_t ctr = 0;
		for(JCoordinate x_pos = rect.left; x_pos < rect.right; x_pos += cJaggedEdgeHeight, ctr++)
			pDC->LineTo(x_pos, rect.top + ((ctr % 2 == 0) ? 0 : cJaggedEdgeHeight));
		
		// Draw last segment
		pDC->LineTo(rect.right, rect.top + ((ctr % 2 == 1) ? 0 : cJaggedEdgeHeight));
	}

	// Right line
	pDC->LineTo(rect.right, rect.bottom - radius);

	if (mEndsInCol)
	{
		// Bottom-right corner
		pDC->Arc(rect.right - 2 * radius, rect.bottom - 2 * radius, 2 * radius, 2 * radius + 1, 0.0, -90.0);
		
		// Bottom line
		pDC->Line(rect.right - radius, rect.bottom, rect.left + radius, rect.bottom);

		// Bottom-left corner
		pDC->Arc(rect.left, rect.bottom - 2 * radius, 2 * radius, 2 * radius + 1, -90.0, -90.0);
		pDC->SetPenLocation(rect.left, rect.bottom - radius);
	}
	else
	{
		// Draw jagged bottom edge
		uint32_t ctr = 0;
		for(JCoordinate x_pos = rect.right; x_pos > rect.left; x_pos -= cJaggedEdgeHeight, ctr++)
			pDC->LineTo(x_pos, rect.bottom - ((ctr % 2 == 1) ? 0 : cJaggedEdgeHeight));
		
		// Draw last segment
		pDC->LineTo(rect.left, rect.bottom - ((ctr % 2 == 0) ? 0 : cJaggedEdgeHeight));
	}

	// Left line
	pDC->LineTo(rect.left, rect.top + radius);
}

void CCalendarEventBase::DrawVertFrameFill(JXWindowPainter* pDC, JRect& rect)
{
	JCoordinate h_radius = rect.height() >= 16 ? cRoundRadius : rect.height() / 2;
	JCoordinate w_radius = rect.width() >= 16 ? cRoundRadius : rect.width() / 2;
	JCoordinate radius = std::min(h_radius, w_radius);
	
	if (mStartsInCol)
	{
		// Top-left corner
		pDC->Arc(rect.left, rect.top, 2 * radius, 2 * radius, 180.0, -90.0);
		
		// Top middle area
		pDC->Rect(rect.left + radius, rect.top, rect.width() - 2 * radius + 1, radius);

		// Top-right corner
		pDC->Arc(rect.right - 2 * radius, rect.top, 2 * radius, 2 * radius, 90.0, -90.0);
	}
	else
	{
		JPolygon poly;
		
		// Draw jagged top edge
		poly.AppendElement(JPoint(0, radius));
		uint32_t ctr = 0;
		for(JCoordinate x_pos = 0; x_pos < rect.width(); x_pos += cJaggedEdgeHeight, ctr++)
			poly.AppendElement(JPoint(x_pos, (ctr % 2 == 0) ? 0 : cJaggedEdgeHeight));

		// Draw last segments
		poly.AppendElement(JPoint(rect.width(), (ctr % 2 == 1) ? 0 : cJaggedEdgeHeight));
		poly.AppendElement(JPoint(rect.width(), radius));
		poly.AppendElement(JPoint(0, radius));
		
		pDC->Polygon(rect.left, rect.top, poly);
	}

	// Middle area
	pDC->Rect(rect.left, rect.top + radius, rect.width(), rect.height() - 2 * radius);

	if (mEndsInCol)
	{
		// Bottom-right corner
		pDC->Arc(rect.right - 2 * radius, rect.bottom - 2 * radius, 2 * radius, 2 * radius, 0.0, -90.0);
		
		// Bottom middle area
		pDC->Rect(rect.left + radius, rect.bottom - radius, rect.width() - 2 * radius + 1, radius);

		// Bottom-left corner
		pDC->Arc(rect.left, rect.bottom - 2 * radius, 2 * radius, 2 * radius, -90.0, -90.0);
	}
	else
	{
		JPolygon poly;
		
		// Draw jagged top edge
		poly.AppendElement(JPoint(0, -radius));
		uint32_t ctr = 0;
		for(JCoordinate x_pos = 0; x_pos > -rect.width(); x_pos -= cJaggedEdgeHeight, ctr++)
			poly.AppendElement(JPoint(x_pos, (ctr % 2 == 1) ? 0 : -cJaggedEdgeHeight));

		// Draw last segment
		poly.AppendElement(JPoint(-rect.width(), (ctr % 2 == 0) ? 0 : -cJaggedEdgeHeight));
		poly.AppendElement(JPoint(-rect.width(), -radius));
		poly.AppendElement(JPoint(0, -radius));
		
		pDC->Polygon(rect.right, rect.bottom, poly);
	}
}
