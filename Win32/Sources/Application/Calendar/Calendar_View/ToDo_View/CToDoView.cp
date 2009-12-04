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

#include "CToDoView.h"

#include "CCalendarView.h"
#include "CFontCache.h"
#include "CMulberryApp.h"
#include "CToDoItem.h"

#include "CCalendarStoreManager.h"

BEGIN_MESSAGE_MAP(CToDoView, CCalendarViewBase)
	ON_WM_CREATE()
	
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWTODO, OnUpdateNewToDo)
	ON_COMMAND(IDM_CALENDAR_NEWTODO, OnNewToDo)

	ON_COMMAND(IDC_TODOVIEW_SHOW, OnShowPopup)
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_NEWTODO, OnUpdateNewToDo)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWTODO, OnNewToDo)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CToDoView														  [public]
/**
	Default constructor */

CToDoView::CToDoView(CCommander* cmdr) :
	CCalendarViewBase(cmdr),
	mShowPopup(true)
{
}


// ---------------------------------------------------------------------------
//	~CToDoView														  [public]
/**
	Destructor */

CToDoView::~CToDoView()
{
}

#pragma mark -

const int cCaptionHeight = 16;
const int cPopupHeight = 22;

const int cCaptionWidth = 36;
const int cCaptionHOffset = 2;

const int cPopupWidth = 116;
const int cPopupHOffset = cCaptionHOffset + cCaptionWidth;

const int cPopupVOffset = 2;
const int cCaptionVOffset = cPopupVOffset + 5;

const int cTableVOffset = cPopupVOffset + cPopupHeight + cPopupVOffset;

int CToDoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Popup bits
	CString s;
	s.LoadString(IDS_POPUP_TODOSHOW);
	CRect r = CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight);
	mShowPopupTitle.Create(s, WS_CHILD | WS_VISIBLE, r, this, IDC_STATIC);
	mShowPopupTitle.SetFont(CMulberryApp::sAppFont);

	r = CRect(cPopupHOffset, cPopupVOffset, cPopupHOffset + cPopupWidth, cPopupVOffset + cPopupHeight);
	mShowPopup.Create(_T(""), r, this, IDC_TODOVIEW_SHOW, IDC_STATIC, IDI_POPUPBTN);
	mShowPopup.SetMenu(IDR_POPUP_TODOSHOW);
	mShowPopup.SetFont(CMulberryApp::sAppFont);
	mShowPopup.SetValue(eShowAllDue);
	mShowValue = eShowAllDue;

	// Table bits
	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
					CRect(0, cTableVOffset, width, height), this, IDC_TODO_TABLE);
	AddAlignment(new CWndAlignment(&mTable, CWndAlignment::eAlign_WidthHeight));
	mTable.SetContextMenuID(IDR_POPUP_CONTEXT_TODOTABLE);
	mTable.SetContextView(static_cast<CView*>(mCalendarView->GetOwningWindow()));
	mTable.SetSuperCommander(this);
	mTable.SetCalendarView(this);
	mTable.Add_Listener(this);
	
	return 0;
}

CCalendarTableBase* CToDoView::GetTable() const
{
	return const_cast<CToDoTable*>(&mTable);
}

void CToDoView::OnUpdateNewToDo(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CToDoView::OnShowPopup()
{
	mShowValue = mShowPopup.GetValue();
	ResetDate();
}
