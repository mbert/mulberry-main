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
#include "CCalendarViewBase.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CErrorDialog.h"
#include "CListener.h"
#include "CMulberryCommon.h"
#include "CNewEventDialog.h"
#include "CResources.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarUtils.h"
#include "CICalendarVFreeBusy.h"
#include "CITIPProcessor.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CCalendarEventBase														  [public]
/**
	Default constructor */

const float body_transparency = 0.75;
const float line_transparency = 1.0;
const float text_transparency = 0.9;

CCalendarEventBase::CCalendarEventBase(const SPaneInfo	&inPaneInfo) :
	LPane(inPaneInfo)
{
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

void CCalendarEventBase::FinishCreateSelf()
{
	// Do inherited
	LPane::FinishCreateSelf();

	CContextMenuAttachment::AddUniqueContext(this, 1830, dynamic_cast<LCommander*>(GetSuperView()));
}

void CCalendarEventBase::Select(bool select)
{
	if (mIsSelected != select)
	{
		mIsSelected = select;

		// Force immediate redraw
		Rect	refreshRect;

		if (CalcPortExposedRect(refreshRect)) {
			StRegion update = refreshRect;
			mTable->Draw(update);
		}
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
	cdstring name;
	if (event->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Confirmed)
	{
		name = cTickMarkU2713_UTF8;
	}
	else if (event->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Tentative)
	{
		name = "?? ";
	}
	name += title;
	mTitle = MyCFString(name.c_str(), kCFStringEncodingUTF8);
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
	if (node != NULL)
		mIsInbox = node->IsInbox();

}

void CCalendarEventBase::SetDetails(iCal::CICalendarVFreeBusy* freebusy, const iCal::CICalendarPeriod& period, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz)
{
	mVFreeBusy = freebusy;
	mPeriod = period;
	mTable = table;

	mTitle = MyCFString(title, kCFStringEncodingUTF8);
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

// Click
void CCalendarEventBase::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Make sure it is selected
	if ((inMouseDown.macEvent.modifiers & (shiftKey | cmdKey)) != 0)
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
		mTable->SwitchTarget(mTable);

	// Broadcast double-click
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		Broadcast_Message(eBroadcast_EditEvent, this);
	}
}

void CCalendarEventBase::DrawSelf()
{
	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	Rect qdrect;
	CalcLocalFrameRect(qdrect);
	HIRect rect;
	CGUtils::QDToHIRect(qdrect, rect);
	
	_cg.Restore();
	if (mHoriz)
		DrawHorizFrame(inContext, rect);
	else
		DrawVertFrame(inContext, rect);
	_cg.Save(inContext);

	// Draw title
	rect.origin.x += 3.0;
	rect.size.width -= (mHoriz && mAllDay || IsFreeBusy()) ? 6.0 : 3.0;
	Rect box;
	CGUtils::HIToQDRect(rect, box);
	if (box.bottom - box.top < 16)
	{
		short height_adjust = (16 - (box.bottom - box.top)) / 2;
		box.top -= height_adjust;
		box.bottom = box.top + 16;
	}

	// Use white colour for selected item
	float red = CGUtils::GetCGRed(mColour);
	float green = CGUtils::GetCGGreen(mColour);
	float blue = CGUtils::GetCGBlue(mColour);
	if (mIsSelected)
	{
		::CGContextSetGrayFillColor(inContext, (red + green + blue > 2.5) ? 0.0 : 1.0, 1.0);
	}
	else
	{
		CGUtils::DarkenColours(red, green, blue);
		::CGContextSetRGBFillColor(inContext, red, green, blue, text_transparency);
	}

	MyCFString trunc(mTitle, kCFStringEncodingUTF8);
	if (mHoriz)
		::TruncateThemeText(trunc, kThemeSmallSystemFont, kThemeStateActive, rect.size.width, truncEnd, NULL);
	::DrawThemeTextBox(trunc, kThemeSmallSystemFont, kThemeStateActive, !mHoriz, &box, (mHoriz && mAllDay || IsFreeBusy()) ? teJustCenter : teJustLeft, inContext);
	
	// Strike out text if status is cancelled
	if (mIsCancelled)
	{
		CGUtils::QDToHIRect(qdrect, rect);
		rect = ::CGRectInset(rect, 4.0, 4.0);
		::CGContextBeginPath(inContext);
		if (mHoriz)
		{
			::CGContextMoveToPoint(inContext, rect.origin.x, rect.origin.y + rect.size.height / 2);
			::CGContextAddLineToPoint(inContext, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height / 2);
		}
		else
		{
			::CGContextMoveToPoint(inContext, rect.origin.x, rect.origin.y);
			::CGContextAddLineToPoint(inContext, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
			::CGContextMoveToPoint(inContext, rect.origin.x + rect.size.width, rect.origin.y);
			::CGContextAddLineToPoint(inContext, rect.origin.x, rect.origin.y + rect.size.height);
		}

		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}
}

void CCalendarEventBase::DrawHorizFrame(CGUtils::CGContextFromQD& inContext, HIRect& rect)
{
	CGUtils::CGContextSaver _cg(inContext);

	// Adjust for ends
	if (mStartsInCol)
	{
		rect.origin.x += 3;
		rect.size.width -= 3;
	}
	else
	{
		rect.origin.x += 1;
		rect.size.width -= 1;
	}
	if (mEndsInCol)
		rect.size.width -= 2;
	else
		rect.size.width -= 1;
	rect.size.height -= 1;
	
	CGMutablePathRef path = ::CGPathCreateMutable();
		
	// Draw left end (rounded if starts, flat otherwise)
	::CGPathMoveToPoint(path, NULL, rect.origin.x + rect.size.width / 2.0, rect.origin.y + rect.size.height);
	if (mStartsInCol)
	{
		if (mAllDay)
		{
			::CGPathAddArcToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height, rect.origin.x, rect.origin.y + rect.size.height / 2.0, rect.size.height/ 2.0);
			::CGPathAddArcToPoint(path, NULL, rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width / 2.0, rect.origin.y, rect.size.height/ 2.0);
		}
		else
		{
			::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height);
			::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y);
			::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width / 2.0, rect.origin.y);
		}
	}
	else
	{
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.height / 2.0, rect.origin.y + rect.size.height);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.height / 6.0, rect.origin.y +  rect.size.height * 5.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height * 4.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.height / 6.0, rect.origin.y + rect.size.height * 3.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height * 2.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.height / 6.0, rect.origin.y + rect.size.height / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width / 2.0, rect.origin.y);
	}

	// Draw right end (rounded if starts, flat otherwise)
	if (mEndsInCol)
	{
		if (mAllDay)
		{
			::CGPathAddArcToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height / 2.0, rect.size.height/ 2.0);
			::CGPathAddArcToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height, rect.origin.x + rect.size.width / 2.0, rect.origin.y + rect.size.height, rect.size.height/ 2.0);
		}
		else
		{
			::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y);
			::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
			::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width / 2.0, rect.origin.y + rect.size.height);
		}
	}
	else
	{
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width - rect.size.height / 6.0, rect.origin.y);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width - rect.size.height / 6.0, rect.origin.y + rect.size.height * 2.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height * 3.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width - rect.size.height / 6.0, rect.origin.y + rect.size.height * 4.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height * 5.0 / 6.0);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width - rect.size.height / 6.0, rect.origin.y + rect.size.height);
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width / 2.0, rect.origin.y + rect.size.height);
	}
	
	// Close path
	::CGPathCloseSubpath(path);

	// Use unsaturated colour for selected item
	float red = CGUtils::GetCGRed(mColour);
	float green = CGUtils::GetCGGreen(mColour);
	float blue = CGUtils::GetCGBlue(mColour);
	float line_factor = IsFreeBusy() ? 1.0 : 1.0;
	if (mIsSelected)
	{
		::CGContextSetRGBStrokeColor(inContext, red * line_factor, green * line_factor, blue * line_factor, 1.0);
		if (IsFreeBusy())
			::CGContextSetGrayFillColor(inContext, 1.0, 1.0);
		else
			::CGContextSetRGBFillColor(inContext, red, green, blue, 1.0);
	}
	else
	{
		::CGContextSetRGBStrokeColor(inContext, red * line_factor, green * line_factor, blue * line_factor, line_transparency);
		CGUtils::LightenColours(red, green, blue);
		if (IsFreeBusy())
			::CGContextSetGrayFillColor(inContext, 1.0, body_transparency);
		else
			::CGContextSetRGBFillColor(inContext, red, green, blue, body_transparency);
	}
	
	::CGContextSetLineWidth(inContext, 0.75);
	if (mIsInbox)
	{
		// Dashed line
		float dashes[2] = { 6.0, 6.0 };
		::CGContextSetLineDash(inContext, 0.0, dashes, 2);
	}

	if (!mAllDay)
		::CGContextSetShouldAntialias(inContext, false);

	::CGContextAddPath(inContext, path);
	::CGContextDrawPath(inContext, kCGPathFillStroke);
	::CGContextSetShouldAntialias(inContext, true);
	
	::CGContextSetLineWidth(inContext, 1.0);
	if (mIsInbox)
	{
		// Continuous line
		::CGContextSetLineDash(inContext, 0.0, NULL, 0);
	}

	// Check for now marker
	if (!mAllDay && mIsNow)
	{
		::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, mIsSelected ? 1.0 : line_transparency);
		::CGContextAddPath(inContext, path);
		::CGContextStrokePath(inContext);
	}

	::CGPathRelease(path);

	// Display alarm indicator
	if (mHasAlarm)
	{
		// Temporarily turn of Quartz whilst we draw a QD icon
		{
			CGUtils::CGContextFromQD::StAllowQuickdraw _qd(inContext, _cg);

			Rect plotFrame;
			CGUtils::HIToQDRect(rect, plotFrame);
			plotFrame.right -= 2;
			plotFrame.left = plotFrame.right - 16;
			plotFrame.bottom = plotFrame.top + 16;
			::Ploticns(&plotFrame, atNone, ttNone, ICNx_AlarmedEvent);
		}
		
		// Offset right edge of text
		rect.size.width -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		// Temporarily turn of Quartz whilst we draw a QD icon
		{
			CGUtils::CGContextFromQD::StAllowQuickdraw _qd(inContext, _cg);

			Rect plotFrame;
			CGUtils::HIToQDRect(rect, plotFrame);
			plotFrame.right -= 2;
			plotFrame.left = plotFrame.right - 16;
			plotFrame.bottom = plotFrame.top + 16;

			SInt16 theResID;
			switch(mAttendeeState)
			{
			case iCal::CITIPProcessor::eNone:
				theResID = ICNx_AttendedEvent;
				break;
			case iCal::CITIPProcessor::eAllAccepted:
				theResID = ICNx_AttendedEvent;
				break;
			case iCal::CITIPProcessor::eSomeAccepted:
				theResID = ICNx_AttendeeNeedsAction;
				break;
			case iCal::CITIPProcessor::eSomeDeclined:
				theResID = ICNx_DeclinedEvent;
				break;
			case iCal::CITIPProcessor::eIHaveAccepted:
				theResID = ICNx_AttendeeIsMeEvent;
				break;
			case iCal::CITIPProcessor::eIHaveNotAccepted:
				theResID = ICNx_AttendeeNeedsAction;
				break;
			}
			
			::Ploticns(&plotFrame, atNone, ttNone, theResID);
		}
		
		// Offset right edge of text
		rect.size.width -= 16;
	}
}

const float cRoundRadius = 8.0;
const float cJaggedEdgeHeight = 6.0;

void CCalendarEventBase::DrawVertFrame(CGUtils::CGContextFromQD& inContext, HIRect& rect)
{
	CGUtils::CGContextSaver _cg(inContext);

	rect = ::CGRectInset(rect, 1.0, 1.0);

	if (IsFreeBusy())
		rect = ::CGRectInset(rect, 3.0, 3.0);
	
	float h_radius = rect.size.height >= 16 ? cRoundRadius : rect.size.height / 2.0;
	float w_radius = rect.size.width >= 16 ? cRoundRadius : rect.size.width / 2.0;
	float radius = std::min(h_radius, w_radius);

	CGMutablePathRef path = ::CGPathCreateMutable();
	
	::CGPathMoveToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height / 2.0);
	if (mStartsInCol)
	{
		// Top-left, top-right corners
		::CGPathAddArcToPoint(path, NULL, rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width / 2.0, rect.origin.y, radius);
		::CGPathAddArcToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height / 2.0, radius);
	}
	else
	{
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + radius);

		// Draw jagged top edge
		uint32_t ctr = 0;
		for(float x_pos = rect.origin.x; x_pos < rect.origin.x + rect.size.width; x_pos += cJaggedEdgeHeight, ctr++)
			::CGPathAddLineToPoint(path, NULL, x_pos, rect.origin.y + ((ctr % 2 == 0) ? 0.0 : cJaggedEdgeHeight));
		
		// Draw last segment
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + ((ctr % 2 == 1) ? 0.0 : cJaggedEdgeHeight));
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height / 2.0);
	}

	if (mEndsInCol)
	{
		// Bottom-right, bottom-left corners
		::CGPathAddArcToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height, rect.origin.x + rect.size.width / 2.0, rect.origin.y + rect.size.height, radius);
		::CGPathAddArcToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height, rect.origin.x, rect.origin.y + rect.size.height / 2.0, radius);
	}
	else
	{
		::CGPathAddLineToPoint(path, NULL, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height - radius);

		// Draw jagged bottom edge
		uint32_t ctr = 0;
		for(float x_pos = rect.origin.x + rect.size.width; x_pos > rect.origin.x; x_pos -= cJaggedEdgeHeight, ctr++)
			::CGPathAddLineToPoint(path, NULL, x_pos, rect.origin.y + rect.size.height - ((ctr % 2 == 1) ? 0.0 : cJaggedEdgeHeight));
		
		// Draw last segments
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height - ((ctr % 2 == 0) ? 0.0 : cJaggedEdgeHeight));
		::CGPathAddLineToPoint(path, NULL, rect.origin.x, rect.origin.y + rect.size.height / 2.0);
	}

	// Close the path
	::CGPathCloseSubpath(path);

	// Use unsaturated colour for selected item
	float red = CGUtils::GetCGRed(mColour);
	float green = CGUtils::GetCGGreen(mColour);
	float blue = CGUtils::GetCGBlue(mColour);
	float line_factor = IsFreeBusy() ? 1.0 : 1.0;
	if (mIsSelected)
	{
		::CGContextSetRGBStrokeColor(inContext, red * line_factor, green * line_factor, blue * line_factor, 1.0);
		if (IsFreeBusy())
			::CGContextSetGrayFillColor(inContext, 1.0, 1.0);
		else
			::CGContextSetRGBFillColor(inContext, red, green, blue, 1.0);
	}
	else
	{
		::CGContextSetRGBStrokeColor(inContext, red * line_factor, green * line_factor, blue * line_factor, line_transparency);
		CGUtils::LightenColours(red, green, blue);
		if (IsFreeBusy())
			::CGContextSetGrayFillColor(inContext, 1.0, body_transparency);
		else
			::CGContextSetRGBFillColor(inContext, red, green, blue, body_transparency);
	}
	
	if (IsFreeBusy())
		::CGContextSetLineWidth(inContext, 5.0);
	else
		::CGContextSetLineWidth(inContext, 0.5);
	if (mIsInbox)
	{
		// Dashed line
		float dashes[2] = { 6.0, 6.0 };
		::CGContextSetLineDash(inContext, 0.0, dashes, 2);
	}
	
	::CGContextAddPath(inContext, path);
	::CGContextDrawPath(inContext, kCGPathFillStroke);

	::CGContextSetLineWidth(inContext, 1.0);
	if (mIsInbox)
	{
		// Continuous line
		::CGContextSetLineDash(inContext, 0.0, NULL, 0);
	}

	// Check for now marker
	if (mIsNow)
	{
		::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, mIsSelected ? 1.0 : line_transparency);
		::CGContextAddPath(inContext, path);
		::CGContextStrokePath(inContext);
	}

	::CGPathRelease(path);
	
	// Adjust rect for jaggies/round corners
	if (mStartsInCol)
	{
		rect.origin.y += 2.0;
		rect.size.height -= 2.0;
	}
	else
	{
		rect.origin.y += cJaggedEdgeHeight;
		rect.size.height -= cJaggedEdgeHeight;
	}
	if (mEndsInCol)
	{
		rect.size.height -= 2.0;
	}
	else
	{
		rect.size.height -= cJaggedEdgeHeight;
	}

	// Display alarm indicator
	if (mHasAlarm)
	{
		// Temporarily turn of Quartz whilst we draw a QD icon
		{
			CGUtils::CGContextFromQD::StAllowQuickdraw _qd(inContext, _cg);

			Rect plotFrame;
			CGUtils::HIToQDRect(rect, plotFrame);
			plotFrame.right -= 2;
			plotFrame.left = plotFrame.right - 16;
			if (plotFrame.bottom - plotFrame.top < 16)
			{
				plotFrame.top = (plotFrame.top + plotFrame.bottom) / 2 - 8;
			}
			plotFrame.bottom = plotFrame.top + 16;
			::Ploticns(&plotFrame, atNone, ttNone, ICNx_AlarmedEvent);
		}
		
		// Offset right edge of text
		rect.size.width -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		// Determine whether to draw alongside or below any alarm
		int32_t horiz_offset = (mHasAlarm && (rect.size.height >= 32)) ? -16 : 0;
		int32_t vert_offset = (mHasAlarm && (rect.size.height >= 32)) ? 16 : 0;

		// Temporarily turn of Quartz whilst we draw a QD icon
		{
			CGUtils::CGContextFromQD::StAllowQuickdraw _qd(inContext, _cg);

			Rect plotFrame;
			CGUtils::HIToQDRect(rect, plotFrame);
			plotFrame.right -= 2 + horiz_offset;
			plotFrame.left = plotFrame.right - 16;
			if (plotFrame.bottom - plotFrame.top < 16)
			{
				plotFrame.top = (plotFrame.top + plotFrame.bottom) / 2 - 8;
			}
			plotFrame.top += vert_offset;
			plotFrame.bottom = plotFrame.top + 16;
			
			SInt16 theResID;
			switch(mAttendeeState)
			{
			case iCal::CITIPProcessor::eNone:
				theResID = ICNx_AttendedEvent;
				break;
			case iCal::CITIPProcessor::eAllAccepted:
				theResID = ICNx_AttendedEvent;
				break;
			case iCal::CITIPProcessor::eSomeAccepted:
				theResID = ICNx_AttendeeNeedsAction;
				break;
			case iCal::CITIPProcessor::eSomeDeclined:
				theResID = ICNx_DeclinedEvent;
				break;
			case iCal::CITIPProcessor::eIHaveAccepted:
				theResID = ICNx_AttendeeIsMeEvent;
				break;
			case iCal::CITIPProcessor::eIHaveNotAccepted:
				theResID = ICNx_AttendeeNeedsAction;
				break;
			}
			
			::Ploticns(&plotFrame, atNone, ttNone, theResID);
		}
		
		// Offset right edge of text
		if (vert_offset == 0)
			rect.size.width -= 16;
	}
	
}
