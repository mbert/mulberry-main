/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CToDoItem.h"

#include "CCalendarTableBase.h"
#include "CCalendarUtils.h"
#include "CCalendarViewBase.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNewToDoDialog.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentExpanded.h"

#include <strstream>

const int cItemHeight = 32;
const int cCheckboxLeftOffset = 2;
const int cCheckboxTopOffset = 2;
const int cCheckboxSize = 16;

BEGIN_MESSAGE_MAP(CToDoItem, CWnd)
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_EDITITEM, OnUpdateRealToDo)
	ON_COMMAND(IDM_CALENDAR_EDITITEM, OnEditToDo)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_DUPLICATEITEM, OnUpdateRealToDo)
	ON_COMMAND(IDM_CALENDAR_DUPLICATEITEM, OnDuplicateToDo)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_DELETEITEM, OnUpdateRealToDo)
	ON_COMMAND(IDM_CALENDAR_DELETEITEM, OnDeleteToDo)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_COMPLETEDTODO, OnUpdateCompletedToDo)
	ON_COMMAND(IDM_CALENDAR_COMPLETEDTODO, OnChangeCompleted)

	ON_COMMAND(IDC_TODO_CHECKBOX, OnCompleted)

	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	ON_WM_PAINT()

	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CToDoItem														  [public]
/**
	Default constructor */

CToDoItem::CToDoItem(CCommander* inSuper) :
	CCommander(inSuper)
{
	mType = eToDo;
	mTable = NULL;
	mIsSelected = false;
	mIsCompleted = false;
	mColour = 0;
}


// ---------------------------------------------------------------------------
//	~CToDoItem														  [public]
/**
	Destructor */

CToDoItem::~CToDoItem()
{
}

CToDoItem* CToDoItem::Create(CWnd* parent, const CRect& frame, CCommander* inSuper)
{

	CToDoItem* result = new CToDoItem(inSuper);
	result->CreateEx(WS_EX_CONTROLPARENT, NULL, NULL, WS_CHILD, frame, parent, 0);
	result->SetFont(CFontCache::GetListFont());

	return result;
}

#pragma mark -

int CToDoItem::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create the checkbox - bottom-left corner of control
	CRect r = CRect(cCheckboxLeftOffset, cCheckboxTopOffset, cCheckboxLeftOffset + cCheckboxSize, cCheckboxTopOffset + cCheckboxSize);
	CString s;
	mCompleted.Create(s, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, r, this, IDC_TODO_CHECKBOX);
	mCompleted.SetFont(CMulberryApp::sAppFont);

	EnableToolTips();
	
	return 0;
}

HBRUSH CToDoItem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

   	// Change background for CStatics only
	if ((nCtlColor == CTLCOLOR_BTN))
	{
		// Use unsaturated colour for selected item
		double red = CCalendarUtils::GetRed(mColour);
		double green = CCalendarUtils::GetGreen(mColour);
		double blue = CCalendarUtils::GetBlue(mColour);
		if (mIsSelected)
			CCalendarUtils::UnflattenColours(red, green, blue);
		pDC->SetBkColor(CCalendarUtils::GetWinColor(red, green, blue));
	}

	return hbr;
}

void CToDoItem::OnUpdateRealToDo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mType == eToDo);
}

void CToDoItem::OnUpdateCompletedToDo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mType == eToDo);
	pCmdUI->SetCheck(mIsCompleted);
}

void CToDoItem::SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table)
{
	mVToDo = todo;
	mType = eToDo;
	mTable = table;
	mSummary = todo->GetMaster<iCal::CICalendarVToDo>()->GetSummary();
	mStatus = todo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Setup a help tag
	SetupTagText();

	// Set the completed status (do without handling the change of value)
	{
		switch(todo->GetMaster<iCal::CICalendarVToDo>()->GetStatus())
		{
		default:
			mCompleted.SetCheck(0);
			break;
		case iCal::eStatus_VToDo_Completed:
			mIsCompleted = true;
			mCompleted.SetCheck(1);
			break;
		case iCal::eStatus_VToDo_Cancelled:
			mCompleted.ShowWindow(SW_HIDE);
			break;
		}
	}
	
	// Determine colour
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
	if (cal)
	{
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}
}

void CToDoItem::SetDetails(EType type)
{
	mVToDo.reset();
	mType = type;
	mTable = NULL;
	
	mSummary = rsrc::GetIndexedString("CToDoItem::When", type);
	
	// Hide checkbox
	mCompleted.ShowWindow(SW_HIDE);
}

// Handle character
void CToDoItem::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch(nChar)
	{
	case VK_RETURN:
		// Only if real to do
		if (mType == eToDo)
			OnEditToDo();
		break;
	default:
		CWnd::OnChar(nChar, nRepCnt, nFlags);
		break;
	}
}

// Click
void CToDoItem::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Only if real to do
	if (mType == eToDo)
	{
		bool was_selected = mIsSelected;
		
		// Always change focus to this one
		SetFocus();

		// Look for toggle of state
		if (nFlags & (MK_SHIFT | MK_CONTROL))
			SelectToDo(!was_selected);
	}
}

// Double-clicked item
void CToDoItem::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Only if real to do
	if (mType == eToDo)
		OnEditToDo();	
}

// Display and track context menu
void CToDoItem::OnContextMenu(CWnd* wnd, CPoint point)
{
	// Only if real to do
	if (mType != eToDo)
		return;

	// Always change focus to this one (will also select it)
	SetFocus();

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_CONTEXT_TODOITEM));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
	SendMessage(WM_COMMAND, popup_result);
}

void CToDoItem::OnSetFocus(CWnd* pOldWnd)
{
	// Make it the commander target
	SetTarget(this);

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(true);
}

void CToDoItem::OnKillFocus(CWnd* pNewWnd)
{
	// Remove the commander target
	//SetTarget(GetSuperCommander());

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(false);
}

void CToDoItem::OnPaint()
{
	CPaintDC dc(this);
	CRect			rect;
	GetClientRect(rect);

	if (mType == eToDo)
	{
		// Use unsaturated colour for selected item
		double red = CCalendarUtils::GetRed(mColour);
		double green = CCalendarUtils::GetGreen(mColour);
		double blue = CCalendarUtils::GetBlue(mColour);
		if (!mIsSelected)
			CCalendarUtils::LightenColours(red, green, blue);
		dc.SetBkColor(CCalendarUtils::GetWinColor(red, green, blue));
		dc.ExtTextOut(rect.left, rect.top, ETO_OPAQUE, rect, _T(""), 0, nil);
	}
	else
	{
		CDrawUtils::DrawSimpleFrame(&dc, rect);
		dc.SetBkColor(CDrawUtils::sGrayColor);
		rect.DeflateRect(0, 3);
	}
	
	// Draw summary text
	rect.DeflateRect(3, 0);
	CRect box(rect);
	if (mType == eToDo)
		box.left += cCheckboxLeftOffset + cCheckboxSize;
	box.bottom = box.top + cItemHeight / 2;

	double red = CCalendarUtils::GetRed(mColour);
	double green = CCalendarUtils::GetGreen(mColour);
	double blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
	{
		dc.SetTextColor((red + green + blue > 2.5) ? CDrawUtils::sBlackColor : CDrawUtils::sWhiteColor);
	}
	else
	{
		CCalendarUtils::DarkenColours(red, green, blue);
		dc.SetTextColor(CCalendarUtils::GetWinColor(red, green, blue));
	}
	dc.SelectObject(CFontCache::GetListFont());
	::DrawClippedStringUTF8(&dc, mSummary, CPoint(box.left, box.top), box, (mType == eToDo) ? eDrawString_Left : eDrawString_Center);

	// Draw status text
	box = rect;
	box.top += cItemHeight / 2;
	::DrawClippedStringUTF8(&dc, mStatus, CPoint(box.left, box.top), box, (mType == eToDo) ? eDrawString_Left : eDrawString_Center);
}


void CToDoItem::OnChangeCompleted()
{
	mCompleted.SetCheck(!mIsCompleted);
	OnCompleted();
}

void CToDoItem::OnCompleted()
{
	// Only if real to do
	if (mType != eToDo)
		return;

	// Change the completed status on the event but do not force an update to the view right now
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->EditStatus(mCompleted.GetCheck() == 1 ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->Changed();
	mIsCompleted = (mCompleted.GetCheck() == 1);
	mStatus = mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Redo tooltip for change
	SetupTagText();
	
	// Refresh as status may have changed
	FRAMEWORK_REFRESH_WINDOW(this);
}

#pragma mark ____________________________________Tooltips

int CToDoItem::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// Only if real to do
	if (mType != eToDo)
		return -1;

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

BOOL CToDoItem::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
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
