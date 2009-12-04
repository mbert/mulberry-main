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
#include "CCalendarUtils.h"
#include "CDrawUtils.h"
#include "CErrorDialog.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CListener.h"
#include "CMulberryCommon.h"
#include "CCalendarTableBase.h"
#include "CCalendarViewBase.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarUtils.h"
#include "CITIPProcessor.h"

#include "cdustring.h"

#include <strstream.h>

BEGIN_MESSAGE_MAP(CCalendarEventBase, CWnd)

	ON_WM_CREATE()

	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()

	ON_WM_PAINT()

	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CCalendarEventBase														  [public]
/**
	Default constructor */

CCalendarEventBase::CCalendarEventBase()
{
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
}

// ---------------------------------------------------------------------------
//	~CCalendarEventBase														  [public]
/**
	Destructor */

CCalendarEventBase::~CCalendarEventBase()
{
}

#pragma mark -

int CCalendarEventBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableToolTips();

	return 0;
}

void CCalendarEventBase::Select(bool select)
{
	if (mIsSelected != select)
	{
		mIsSelected = select;

		// Force immediate redraw
		RedrawWindow();
	}
}

const unsigned char cTickMarkU2713_UTF8[] = 
{
	0xE2, 0x9C, 0x93, 0x00
};

const unsigned char cCrossMarkU2713_UTF8[] = 
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
	mTitle = name;
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
}

// Click
void CCalendarEventBase::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Make sure it is selected
	if (nFlags & (MK_SHIFT | MK_CONTROL))
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
		mTable->SetFocus();
}

// Double-click
void CCalendarEventBase::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Broadcast double-click
	Broadcast_Message(eBroadcast_EditEvent, this);
}

// Display and track context menu
void CCalendarEventBase::OnContextMenu(CWnd* wnd, CPoint point)
{
	if (!mIsSelected)
	{
		SClickEvent_Message _msg(this, false);
		Broadcast_Message(eBroadcast_ClickEvent, &_msg);
	}

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_CONTEXT_EVENTITEM));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, pWndPopupOwner);
	GetParent()->SendMessage(WM_COMMAND, popup_result);
}

void CCalendarEventBase::OnPaint()
{
	CPaintDC		dc(this);
	CRect			rect;
	GetClientRect(rect);

	if (mHoriz)
		DrawHorizFrame(&dc, rect);
	else
		DrawVertFrame(&dc, rect);
	
	// Draw title
	rect.DeflateRect(3, 1);
	CRect cliprect(rect);
	if (rect.Height() < 16)
	{
		short height_adjust = (16 - rect.Height()) / 2;
		rect.top -= height_adjust;
		rect.bottom = rect.top + 16;
		GetClientRect(cliprect);
		cliprect.DeflateRect(0, 1);
		cliprect.left = rect.left;
		cliprect.right = rect.right;
	}
	dc.SetTextColor(mIsSelected ? CDrawUtils::sWhiteColor : CDrawUtils::sBlackColor);
	dc.SelectObject(CFontCache::GetListFont());
	if (mHoriz)
		::DrawClippedStringUTF8(&dc, mTitle, CPoint(rect.left, rect.top), cliprect, mAllDay ? eDrawString_Center : eDrawString_Left);
	else
	{
		cdustring utf16(mTitle);
		size_t len = utf16.length();
		::DrawTextExW(dc, utf16, len, cliprect, DT_END_ELLIPSIS | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK, NULL);
	}
	
	// Strike out text if status is cancelled
	if (mIsCancelled)
	{
		GetClientRect(rect);
		rect.DeflateRect(4, 4);
		CPen temp(PS_SOLID, 1, dc.GetTextColor());
		CPen* old_pen = (CPen*) dc.SelectObject(&temp);
		if (mHoriz)
		{
			dc.MoveTo(rect.left, (rect.top + rect.bottom) / 2);
			dc.LineTo(rect.right, (rect.top + rect.bottom) / 2);
		}
		else
		{
			dc.MoveTo(rect.left, rect.top);
			dc.LineTo(rect.right, rect.bottom);
			dc.MoveTo(rect.right, rect.top);
			dc.LineTo(rect.left, rect.bottom);
		}
		dc.SelectObject(old_pen);
	}
}

void CCalendarEventBase::DrawHorizFrame(CDC* pDC, CRect& rect)
{
	pDC->SetArcDirection(AD_CLOCKWISE);

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
	
	// Start the object outline
	pDC->BeginPath();

	// Draw left end (rounded if starts, flat otherwise)
	if (mStartsInCol)
	{
		if (mAllDay)
		{
			pDC->MoveTo(rect.left + rect.Height() / 2, rect.bottom);
			pDC->ArcTo(rect.left, rect.top, rect.left + rect.Height(), rect.bottom + 1,
						rect.left + rect.Height() / 2, rect.bottom + 1, rect.left + rect.Height() / 2, rect.top);
			pDC->LineTo(rect.left + rect.Height() / 2, rect.top);
		}
		else
		{
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.left, rect.top);
		}
	}
	else
	{
		pDC->MoveTo(rect.left + rect.Height() / 2, rect.bottom);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->LineTo(rect.left + rect.Height() / 6, rect.top +  (rect.Height() * 5) / 6);
		pDC->LineTo(rect.left, rect.top + (rect.Height() * 4) / 6);
		pDC->LineTo(rect.left + rect.Height() / 6, rect.top + (rect.Height() * 3) / 6);
		pDC->LineTo(rect.left, rect.top + (rect.Height() * 2) / 6);
		pDC->LineTo(rect.left + rect.Height() / 6, rect.top + rect.Height() / 6);
		pDC->LineTo(rect.left, rect.top);
	}

	// Draw top line
	if (mAllDay)
		pDC->LineTo(rect.right - rect.Height() / 2, rect.top);
	else
		pDC->LineTo(rect.right, rect.top);

	// Draw right end (rounded if starts, flat otherwise)
	if (mEndsInCol)
	{
		if (mAllDay)
		{
			pDC->ArcTo(rect.right - rect.Height(), rect.top, rect.right, rect.bottom + 1,
						rect.right - rect.Height() / 2, rect.top, rect.right - rect.Height() / 2, rect.bottom + 1);
			pDC->LineTo(rect.right - rect.Height() / 2, rect.bottom);
		}
		else
		{
			pDC->LineTo(rect.right, rect.bottom);
		}
	}
	else
	{
		pDC->LineTo(rect.right - rect.Height() / 6, rect.top);
		pDC->LineTo(rect.right, rect.top + rect.Height() / 6);
		pDC->LineTo(rect.right - rect.Height() / 6, rect.top + (rect.Height() * 2) / 6);
		pDC->LineTo(rect.right, rect.top + (rect.Height() * 3) / 6);
		pDC->LineTo(rect.right - rect.Height() / 6, rect.top + (rect.Height() * 4) / 6);
		pDC->LineTo(rect.right, rect.top + (rect.Height() * 5) / 6);
		pDC->LineTo(rect.right - rect.Height() / 6, rect.top + rect.Height());
	}

	// Draw bottom line
	if (mAllDay)
		pDC->LineTo(rect.left + rect.Height() / 2, rect.bottom);
	else
		pDC->LineTo(rect.left, rect.bottom);

	// Outline complete
	pDC->EndPath();

	// Use unsaturated colour for selected item
	double red = CCalendarUtils::GetRed(mColour);
	double green = CCalendarUtils::GetGreen(mColour);
	double blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
		CCalendarUtils::UnflattenColours(red, green, blue);

	CBrush temp_brush(CCalendarUtils::GetWinColor(red, green, blue));
	CBrush* old_brush = pDC->SelectObject(&temp_brush);
	CPen temp_pen(PS_SOLID, 1, (!mAllDay && mIsNow) ? CCalendarUtils::GetWinColor(0.95, 0.0, 0.0) : CCalendarUtils::GetWinColor(red * 0.6, green * 0.6, blue * 0.6));
	CPen* old_pen = pDC->SelectObject(&temp_pen);
	pDC->SetBkColor(CCalendarUtils::GetWinColor(red, green, blue));
	pDC->StrokeAndFillPath();

	pDC->SelectObject(old_pen);
	pDC->SelectObject(old_brush);

	// Adjust for round edges clip
	if (rect.Height() < 20)
	{
		rect.left += 3;
		rect.right -= 3;
	}

	// Display alarm indicator
	if (mHasAlarm)
	{
		CIconLoader::DrawIcon(pDC, rect.right - 17, rect.top, IDI_HASALARM, 16);
		
		// Offset right edge of text
		rect.right -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		UINT theResID;
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

		CIconLoader::DrawIcon(pDC, rect.right - 17, rect.top, theResID, 16);
		
		// Offset right edge of text
		rect.right -= 16;
	}
	
	if (!mHasAlarm || (mAttendeeState == iCal::CITIPProcessor::eNone))
	{
		rect.right -= 4;
	}
}

const int32_t cRoundRadius = 8;
const int32_t cJaggedEdgeHeight = 6;

void CCalendarEventBase::DrawVertFrame(CDC* pDC, CRect& rect)
{
	pDC->SetArcDirection(AD_CLOCKWISE);

	rect.DeflateRect(1, 0);
	rect.bottom -= 1;

	int32_t h_radius = rect.Height() >= 16 ? cRoundRadius : rect.Height() / 2;
	int32_t w_radius = rect.Width() >= 16 ? cRoundRadius : rect.Width() / 2;
	int32_t radius = min(h_radius, w_radius);

	pDC->BeginPath();
	
	if (mStartsInCol)
	{
		// Top-left corner
		pDC->MoveTo(rect.left, rect.top + radius);
		pDC->ArcTo(rect.left, rect.top, rect.left + 2 * radius, rect.top + 2 * radius, rect.left, rect.top + radius, rect.left + radius, rect.top);
		pDC->LineTo(rect.left + radius, rect.top);
		
		// Top line
		pDC->LineTo(rect.right - radius, rect.top);

		// Top-right corner
		pDC->ArcTo(rect.right - 2 * radius, rect.top, rect.right, rect.top + 2 * radius, rect.right - radius, rect.top, rect.right, rect.top + radius);
		pDC->LineTo(rect.right, rect.top + radius);
	}
	else
	{
		// Draw jagged top edge
		pDC->MoveTo(rect.left, rect.top + radius);
		uint32_t ctr = 0;
		for(int32_t x_pos = rect.left; x_pos < rect.right; x_pos += cJaggedEdgeHeight, ctr++)
			pDC->LineTo(x_pos, rect.top + ((ctr % 2 == 0) ? 0 : cJaggedEdgeHeight));
		
		// Draw last segment
		pDC->LineTo(rect.right, rect.top + ((ctr % 2 == 1) ? 0 : cJaggedEdgeHeight));
	}

	// Right line
	pDC->LineTo(rect.right, rect.bottom - radius);

	if (mEndsInCol)
	{
		// Bottom-right corner
		pDC->ArcTo(rect.right - 2 * radius, rect.bottom - 2 * radius, rect.right, rect.bottom, rect.right, rect.bottom - radius, rect.right - radius, rect.bottom);
		pDC->LineTo(rect.right - radius, rect.bottom);
		
		// Bottom line
		pDC->LineTo(rect.left + radius, rect.bottom);

		// Bottom-left corner
		pDC->ArcTo(rect.left, rect.bottom - 2 * radius, rect.left + 2 * radius, rect.bottom, rect.left - radius, rect.bottom, rect.left, rect.bottom - radius);
		pDC->LineTo(rect.left, rect.bottom - radius);
	}
	else
	{
		// Draw jagged bottom edge
		uint32_t ctr = 0;
		for(int32_t x_pos = rect.right; x_pos > rect.left; x_pos -= cJaggedEdgeHeight, ctr++)
			pDC->LineTo(x_pos, rect.bottom - ((ctr % 2 == 1) ? 0 : cJaggedEdgeHeight));
		
		// Draw last segment
		pDC->LineTo(rect.left, rect.bottom - ((ctr % 2 == 0) ? 0 : cJaggedEdgeHeight));
	}

	// Left line
	pDC->LineTo(rect.left, rect.top + radius);

	// Outline complete
	pDC->EndPath();

	// Use unsaturated colour for selected item
	double red = CCalendarUtils::GetRed(mColour);
	double green = CCalendarUtils::GetGreen(mColour);
	double blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
		CCalendarUtils::UnflattenColours(red, green, blue);
	
	CBrush temp_brush(CCalendarUtils::GetWinColor(red, green, blue));
	CBrush* old_brush = pDC->SelectObject(&temp_brush);
	CPen temp_pen(PS_SOLID, 1, mIsNow ? CCalendarUtils::GetWinColor(0.95, 0.0, 0.0) : CCalendarUtils::GetWinColor(red * 0.6, green * 0.6, blue * 0.6));
	CPen* old_pen = pDC->SelectObject(&temp_pen);
	pDC->SetBkColor(CCalendarUtils::GetWinColor(red, green, blue));
	pDC->StrokeAndFillPath();

	pDC->SelectObject(old_pen);
	pDC->SelectObject(old_brush);
	
	// Adjust rect for jaggies/round corners
	if (mStartsInCol)
	{
		rect.top += 3;
	}
	else
	{
		rect.top += cJaggedEdgeHeight;
	}
	if (mEndsInCol)
	{
		rect.bottom -= 3;
	}
	else
	{
		rect.bottom -= cJaggedEdgeHeight;
	}

	// Adjust for round edges clip
	if (rect.Height() < 20)
	{
		rect.left += 3;
		rect.right -= 3;
	}

	// Display alarm indicator
	if (mHasAlarm)
	{
		int vOffset = rect.top;
		if (rect.Height() < 16)
			vOffset = (rect.top + rect.bottom) / 2 - 8;
		CIconLoader::DrawIcon(pDC, rect.right - 17, vOffset, IDI_HASALARM, 16);
		
		// Offset right edge of text
		rect.right -= 16;
	}

	// Display attendee indicator
	if (mAttendeeState != iCal::CITIPProcessor::eNone)
	{
		// Determine whether to draw alongside or below any alarm
		int horiz_offset = (mHasAlarm && (rect.Height() >= 32)) ? 16 : 0;
		int vert_offset = (mHasAlarm && (rect.Height() >= 32)) ? 16 : 0;
		int vOffset = rect.top;
		if (rect.Height() < 16)
			vOffset = (rect.top + rect.bottom) / 2 - 8;

		UINT theResID;
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

		CIconLoader::DrawIcon(pDC, rect.right - 17 + horiz_offset, vOffset + vert_offset, theResID, 16);
		
		// Offset right edge of text
		rect.right -= 16;
	}
}

#pragma mark ____________________________________Tooltips

int CCalendarEventBase::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rect;
	GetClientRect(rect);
	if (rect.PtInRect(point))
	{
		// Hits against child windows always center the tip
		if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
		{
			// Setup the TOOLINFO structure
			pTI->hwnd = m_hWnd;
			pTI->uId = 1;
			pTI->lpszText = LPSTR_TEXTCALLBACK;
			pTI->rect = rect;
			return pTI->uId;
		}
	}
	
	// Not found
	return -1;
}

BOOL CCalendarEventBase::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// Need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	UINT nID = pNMHDR->idFrom;

	if (nID == 0)
		return false;
	
	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

#ifdef _UNICODE
	mTooltipTextUTF16 = cdustring(mTooltipText);
	pTTTW->lpszText = mTooltipTextUTF16.c_str_mod();
#else
	pTTTA->lpszText = mTooltipText.c_str_mod();
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}
