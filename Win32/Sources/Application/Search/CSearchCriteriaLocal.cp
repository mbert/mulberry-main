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


// Source for CSearchCriteriaLocal class

#include "CSearchCriteriaLocal.h"

#include "CFilterItem.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSearchBase.h"
#include "CSearchCriteriaContainer.h"
#include "CSearchStyle.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	eCriteria_From = IDM_SEARCH_CRITERIA_FROM,
	eCriteria_To,
	eCriteria_CC,
	eCriteria_Bcc,
	eCriteria_Recipient,
	eCriteria_Correspondent,
	eCriteria_Sender,
	eCriteria_DateSent,
	eCriteria_DateReceived,
	eCriteria_Subject,
	eCriteria_Body,
	eCriteria_Header,
	eCriteria_Text,
	eCriteria_Size,
	eCriteria_Separator1,
	eCriteria_Recent,
	eCriteria_Seen,
	eCriteria_Answered,
	eCriteria_Flagged,
	eCriteria_Deleted,
	eCriteria_Draft,
	eCriteria_Separator2,
	eCriteria_Label1 = IDM_SEARCH_CRITERIA_LABEL1,
	eCriteria_Label2,
	eCriteria_Label3,
	eCriteria_Label4,
	eCriteria_Label5,
	eCriteria_Label6,
	eCriteria_Label7,
	eCriteria_Label8,
	eCriteria_Separator3 = IDM_SEARCH_CRITERIA_SEPARATOR3,
	eCriteria_Group,
	eCriteria_Separator4,
	eCriteria_SearchSet,
	eCriteria_Separator5,
	eCriteria_All,
	eCriteria_Selected
};

enum
{
	eAddressMethod_Contains = IDM_SEARCH_ADDRESSMETHOD_CONTAINS,
	eAddressMethod_NotContains,
	eAddressMethod_IsMe,
	eAddressMethod_IsNotMe
};

enum
{
	eDateMethod_Before = IDM_SEARCH_DATEMETHOD_BEFORE,
	eDateMethod_On,
	eDateMethod_After,
	eDateMethod_Separator1,
	eDateMethod_Is,
	eDateMethod_IsNot,
	eDateMethod_IsWithin,
	eDateMethod_IsNotWithin
};

enum
{
	eDateRelMethod_SentToday = IDM_SEARCH_DATERELMETHOD_TODAY,
	eDateRelMethod_SentYesterday,
	eDateRelMethod_SentWeek,
	eDateRelMethod_Sent7Days,
	eDateRelMethod_SentMonth,
	eDateRelMethod_SentYear
};

enum
{
	eDateWithin_Days = IDM_SEARCH_DATEWITHIN_DAYS,
	eDateWithin_Weeks,
	eDateWithin_Months,
	eDateWithin_Years
};

enum
{
	eTextMethod_Contains = IDM_SEARCH_TEXTMETHOD_CONTAINS,
	eTextMethod_NotContains
};

enum
{
	eSizeMethod_Larger = IDM_SEARCH_SIZEMETHOD_LARGER,
	eSizeMethod_Smaller
};

enum
{
	eFlagMethod_Set = IDM_SEARCH_FLAGMETHOD_SET,
	eFlagMethod_NotSet
};

enum
{
	eSize_Bytes = IDM_SEARCH_SIZE_BYTES,
	eSize_KBytes,
	eSize_MBytes
};

enum
{
	eSearchSetMethod_Is = IDM_SEARCH_SETMETHOD_IS,
	eSearchSetMethod_IsNot
};

enum
{
	eMode_Or = IDM_SEARCH_MODE_OR,
	eMode_And
};

BEGIN_MESSAGE_MAP(CSearchCriteriaLocal, CSearchCriteria)
	ON_WM_CREATE()
	
	ON_COMMAND_RANGE(IDM_SEARCH_CRITERIA_FROM, IDM_SEARCH_CRITERIA_SELECTED, OnSetCriteria)
	ON_COMMAND_RANGE(IDM_SEARCH_CRITERIA_LABEL1, IDM_SEARCH_CRITERIA_LABEL8, OnSetCriteria)
	ON_COMMAND_RANGE(IDM_SEARCH_ADDRESSMETHOD_CONTAINS, IDM_SEARCH_ADDRESSMETHOD_ISNOTME, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_DATEMETHOD_BEFORE, IDM_SEARCH_DATEMETHOD_ISNOTWITHIN, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_TEXTMETHOD_CONTAINS, IDM_SEARCH_TEXTMETHOD_NOTCONTAINS, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_SIZEMETHOD_LARGER, IDM_SEARCH_SIZEMETHOD_SMALLER, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_FLAGMETHOD_SET, IDM_SEARCH_FLAGMETHOD_UNSET, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_SIZE_BYTES, IDM_SEARCH_SIZE_MBYTES, OnSetSize)
	ON_COMMAND_RANGE(IDM_SEARCH_DATERELMETHOD_TODAY, IDM_SEARCH_DATERELMETHOD_YEAR, OnSetDateRel)
	ON_COMMAND_RANGE(IDM_SEARCH_DATEWITHIN_DAYS, IDM_SEARCH_DATEWITHIN_YEARS, OnSetDateWithin)
	ON_COMMAND_RANGE(IDM_SEARCH_SEARCHMETHOD_Start, IDM_SEARCH_SEARCHMETHOD_End, OnSetSearchSet)
	ON_COMMAND_RANGE(IDM_SEARCH_SETMETHOD_IS, IDM_SEARCH_SETMETHOD_ISNOT, OnSetSearchSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_MODE_OR, IDM_SEARCH_MODE_AND, OnSetMode)

	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT1, OnTextReturn)
	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT2, OnTextReturn)
	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT3, OnTextReturn)
END_MESSAGE_MAP()

// Default constructor
CSearchCriteriaLocal::CSearchCriteriaLocal()
{
}

// Default destructor
CSearchCriteriaLocal::~CSearchCriteriaLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CSearchCriteriaLocal::Create(const CRect& rect, CWnd* pParentWnd)
{
	if (CSearchCriteria::Create(rect, pParentWnd))
	{
		// Get parent view
		CWnd* wnd = pParentWnd;
		while((wnd != NULL) && (dynamic_cast<CView*>(wnd) == NULL))
			wnd = wnd->GetParent();
		CView* view = dynamic_cast<CView*>(wnd);

		// Make sure context menus are active
		mText1.SetContextView(view);
		mText2.SetContextView(view);
		mText3.SetContextView(view);
		return true;
	}
	else
		return false;
}

const int cCaptionHeight = 16;
const int cEditHeight = 22;
const int cPopupHeight = 22;

const int cMoveBtnWidth = 16;
const int cMoveBtnHeight = 12;
const int cMoveBtnHOffset = 0;
const int cMoveUpBtnVOffset = 1;
const int cMoveDownBtnVOffset = cMoveUpBtnVOffset + cMoveBtnHeight;

const int cPopup4Width = 56;
const int cPopup4Height = cPopupHeight;
const int cPopup4HOffset = cMoveBtnHOffset + cMoveBtnWidth + 4;
const int cPopup4VOffset = 2;

const int cPopup1Width = 122;
const int cPopup1Height = cPopupHeight;
const int cPopup1HOffset = cPopup4HOffset + cPopup4Width + 4;
const int cPopup1VOffset = cPopup4VOffset;

const int cPopup2Width = 128;
const int cPopup2Height = cPopupHeight;
const int cPopup2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup2VOffset = cPopup1VOffset;

const int cText1Width = 120;
const int cText1Height = cEditHeight;
const int cText1HOffset = cPopup2HOffset + cPopup2Width + 4;
const int cText1VOffset = cPopup1VOffset;

const int cText2Width = 128;
const int cText2Height = cEditHeight;
const int cText2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cText2VOffset = cText1VOffset;

const int cText3Width = 52;
const int cText3Height = cEditHeight;
const int cText3HOffset = cPopup2HOffset + cPopup2Width + 4;
const int cText3VOffset = cText1VOffset;

const int cDateWidth = 120;
const int cDateHeight = cEditHeight;
const int cDateHOffset = cPopup2HOffset + cPopup2Width + 4;
const int cDateVOffset = cPopup1VOffset;

const int cPopup3Width = 64;
const int cPopup3Height = cPopupHeight;
const int cPopup3HOffset = cText3HOffset + cText3Width + 4;
const int cPopup3VOffset = cPopup1VOffset;

const int cPopup5Width = cDateWidth;
const int cPopup5Height = cPopupHeight;
const int cPopup5HOffset = cPopup2HOffset + cPopup2Width + 4;
const int cPopup5VOffset = cPopup1VOffset;

const int cPopup6Width = 64;
const int cPopup6Height = cPopupHeight;
const int cPopup6HOffset = cText3HOffset + cText3Width + 4;
const int cPopup6VOffset = cPopup1VOffset;

const int cPopup8Width = 96;
const int cPopup8Height = cPopupHeight;
const int cPopup8HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup8VOffset = cPopup1VOffset;

const int cPopup7Width = 160;
const int cPopup7Height = cPopupHeight;
const int cPopup7HOffset = cPopup8HOffset + cPopup8Width + 4;
const int cPopup7VOffset = cPopup1VOffset;

int CSearchCriteriaLocal::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSearchCriteria::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Popups
	CString s;
	CRect r = CRect(cPopup1HOffset, cPopup1VOffset, cPopup1HOffset + cPopup1Width, cPopup1VOffset + cPopup1Height + small_offset);
	mPopup1.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup1.SetMenu(IDR_POPUP_SEARCH_CRITERIA);
	mPopup1.SetValue(IDM_SEARCH_CRITERIA_FROM);
	mPopup1.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup1, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup2HOffset, cPopup2VOffset, cPopup2HOffset + cPopup2Width, cPopup2VOffset + cPopup2Height + small_offset);
	mPopup2.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP2, IDC_STATIC, IDI_POPUPBTN);
	mPopup2.SetMenu(IDR_POPUP_SEARCH_ADDRESSMETHOD);
	mPopup2.SetValue(IDM_SEARCH_ADDRESSMETHOD_CONTAINS);
	mPopup2.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup2, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup3HOffset, cPopup3VOffset, cPopup3HOffset + cPopup3Width, cPopup3VOffset + cPopup3Height + small_offset);
	mPopup3.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP3, IDC_STATIC, IDI_POPUPBTN);
	mPopup3.SetMenu(IDR_POPUP_SEARCH_SIZE);
	mPopup3.SetValue(IDM_SEARCH_SIZE_KBYTES);
	mPopup3.SetFont(CMulberryApp::sAppFont);
	mPopup3.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mPopup3, CWndAlignment::eAlign_TopRight));

	r = CRect(cPopup4HOffset, cPopup4VOffset, cPopup4HOffset + cPopup4Width, cPopup4VOffset + cPopup4Height);
	mPopup4.Create(_T(""), r, this, IDC_SEARCHCRITERIA_POPUP4, IDC_STATIC, IDI_POPUPBTN);
	mPopup4.SetMenu(IDR_POPUP_SEARCH_MODE);
	mPopup4.SetValue(IDM_SEARCH_MODE_OR);
	mPopup4.SetFont(CMulberryApp::sAppFont);
	mPopup4.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mPopup4, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup5HOffset, cPopup5VOffset, cPopup5HOffset + cPopup5Width, cPopup5VOffset + cPopup5Height);
	mPopup5.Create(_T(""), r, this, IDC_SEARCHCRITERIA_POPUP5, IDC_STATIC, IDI_POPUPBTN);
	mPopup5.SetMenu(IDR_POPUP_SEARCH_DATERELMETHOD);
	mPopup5.SetValue(IDM_SEARCH_DATERELMETHOD_TODAY);
	mPopup5.SetFont(CMulberryApp::sAppFont);
	mPopup5.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mPopup5, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup6HOffset, cPopup6VOffset, cPopup6HOffset + cPopup6Width, cPopup6VOffset + cPopup6Height);
	mPopup6.Create(_T(""), r, this, IDC_SEARCHCRITERIA_POPUP6, IDC_STATIC, IDI_POPUPBTN);
	mPopup6.SetMenu(IDR_POPUP_SEARCH_DATEWITHIN);
	mPopup6.SetValue(IDM_SEARCH_DATEWITHIN_DAYS);
	mPopup6.SetFont(CMulberryApp::sAppFont);
	mPopup6.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mPopup6, CWndAlignment::eAlign_TopRight));

	r = CRect(cPopup7HOffset, cPopup7VOffset, cPopup7HOffset + cPopup7Width, cPopup7VOffset + cPopup7Height + small_offset);
	mPopup7.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP7, IDC_STATIC, IDI_POPUPBTN);
	mPopup7.SetMenu(IDR_POPUP_SEARCH_SEARCHSET);
	mPopup7.SetFont(CMulberryApp::sAppFont);
	mPopup7.ShowWindow(SW_HIDE);

	r = CRect(cPopup8HOffset, cPopup8VOffset, cPopup8HOffset + cPopup8Width, cPopup8VOffset + cPopup8Height + small_offset);
	mPopup8.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP8, IDC_STATIC, IDI_POPUPBTN);
	mPopup8.SetMenu(IDR_POPUP_SEARCH_SEARCHSETMETHOD);
	mPopup8.SetValue(IDM_SEARCH_SETMETHOD_IS);
	mPopup8.SetFont(CMulberryApp::sAppFont);
	mPopup8.ShowWindow(SW_HIDE);

	// Edit fields
	r = CRect(cText1HOffset, cText1VOffset, cText1HOffset + cText1Width, cText1VOffset + cText1Height + small_offset);
	mText1.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_SEARCHCRITERIA_TEXT1);
	mText1.SetFont(CMulberryApp::sAppFont);
	mText1.SetReturnCmd(true);
	mHeader.AddAlignment(new CWndAlignment(&mText1, CWndAlignment::eAlign_TopWidth));

	r = CRect(cText2HOffset, cText2VOffset, cText2HOffset + cText2Width, cText2VOffset + cText2Height + small_offset);
	mText2.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_SEARCHCRITERIA_TEXT2);
	mText2.SetFont(CMulberryApp::sAppFont);
	mText2.SetReturnCmd(true);
	mHeader.AddAlignment(new CWndAlignment(&mText2, CWndAlignment::eAlign_TopLeft));

	r = CRect(cText3HOffset, cText3VOffset, cText3HOffset + cText3Width, cText3VOffset + cText3Height + small_offset);
	mText3.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_SEARCHCRITERIA_TEXT3);
	mText3.SetFont(CMulberryApp::sAppFont);
	mText3.SetReturnCmd(true);
	mHeader.AddAlignment(new CWndAlignment(&mText3, CWndAlignment::eAlign_TopWidth));

	// Date field
	r = CRect(cDateHOffset, cDateVOffset, cDateHOffset + cDateWidth, cDateVOffset + cDateHeight + small_offset);
	if (CMulberryApp::sCommonControlsEx)
		mDate.CreateEx(0, DATETIMEPICK_CLASS, _T("DateTime"), WS_CHILD | WS_BORDER, r, &mHeader, IDC_SEARCHCRITERIA_DATE);
	else
		mDate.CreateEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, _T(""), WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_SEARCHCRITERIA_DATE);
	mDate.SetCommonCtrlEx(CMulberryApp::sCommonControlsEx);
	mDate.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mDate, CWndAlignment::eAlign_TopWidth));

	InitLabelNames();

	return 0;
}

void CSearchCriteriaLocal::SetRules(bool rules)
{
	mRules = rules;
	
	// Remove select item from popup if not rules
	if (!mRules)
	{
		mPopup1.GetPopupMenu()->RemoveMenu(eCriteria_Selected, MF_BYCOMMAND);
	}
}

// Tell window to focus on this one
bool CSearchCriteriaLocal::SetInitialFocus()
{
	// Force focus to first visible text item
	if (mText2.IsWindowVisible())
	{
		mText2.SetFocus();
		mText2.SetSel(0, -1);
		return true;
	}
	else if (mText1.IsWindowVisible())
	{
		mText1.SetFocus();
		mText1.SetSel(0, -1);
		return true;
	}
	else if (mText3.IsWindowVisible())
	{
		mText3.SetFocus();
		mText3.SetSel(0, -1);
		return true;
	}
	else if (mGroupItems)
		return mGroupItems->SetInitialFocus();
	
	return false;
}

long CSearchCriteriaLocal::ShowOrAnd(bool show)
{
	if (show)
		mPopup4.ShowWindow(SW_SHOW);
	else
		mPopup4.ShowWindow(SW_HIDE);
	
	return 0;
}

bool CSearchCriteriaLocal::IsOr() const
{
	return (mPopup4.GetValue() == eMode_Or);
}

void CSearchCriteriaLocal::SetOr(bool use_or)
{
	mPopup4.SetValue(use_or ? eMode_Or : eMode_And);
}

void CSearchCriteriaLocal::OnSetCriteria(UINT nID)
{
	// Set popup menu for method and show/hide text field as approriate
	bool method_refresh = false;
	switch(nID)
	{
	case eCriteria_From:
	case eCriteria_To:
	case eCriteria_CC:
	case eCriteria_Bcc:
	case eCriteria_Recipient:
	case eCriteria_Correspondent:
	case eCriteria_Sender:
		if (mPopup2.GetMenuID() != IDR_POPUP_SEARCH_ADDRESSMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SEARCH_ADDRESSMETHOD);
			mPopup2.SetValue(IDM_SEARCH_ADDRESSMETHOD_CONTAINS);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_DateSent:
	case eCriteria_DateReceived:
		if (mPopup2.GetMenuID() != IDR_POPUP_SEARCH_DATEMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SEARCH_DATEMETHOD);
			mPopup2.SetValue(IDM_SEARCH_DATEMETHOD_BEFORE);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_SHOW);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Subject:
	case eCriteria_Body:
	case eCriteria_Text:
		if (mPopup2.GetMenuID() != IDR_POPUP_SEARCH_TEXTMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SEARCH_TEXTMETHOD);
			mPopup2.SetValue(IDM_SEARCH_TEXTMETHOD_CONTAINS);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Header:
		CUnicodeUtils::SetWindowTextUTF8(&mText2, cdstring::null_str);
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_SHOW);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Size:
		if (mPopup2.GetMenuID() != IDR_POPUP_SEARCH_SIZEMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SEARCH_SIZEMETHOD);
			mPopup2.SetValue(IDM_SEARCH_SIZEMETHOD_LARGER);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_SHOW);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_SHOW);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Recent:
	case eCriteria_Seen:
	case eCriteria_Answered:
	case eCriteria_Flagged:
	case eCriteria_Deleted:
	case eCriteria_Draft:
	case eCriteria_Label1:
	case eCriteria_Label2:
	case eCriteria_Label3:
	case eCriteria_Label4:
	case eCriteria_Label5:
	case eCriteria_Label6:
	case eCriteria_Label7:
	case eCriteria_Label8:
		if (mPopup2.GetMenuID() != IDR_POPUP_SEARCH_FLAGMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SEARCH_FLAGMETHOD);
			mPopup2.SetValue(IDM_SEARCH_FLAGMETHOD_SET);
		}
		CUnicodeUtils::SetWindowTextUTF8(&mText1, cdstring::null_str);
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		mPopup7.ShowWindow(SW_HIDE);
		mPopup8.ShowWindow(SW_HIDE);
		break;
	case eCriteria_SearchSet:
		mPopup2.ShowWindow(SW_HIDE);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		InitSearchSets();
		mPopup7.ShowWindow(SW_SHOW);
		mPopup8.ShowWindow(SW_SHOW);
		break;
	case eCriteria_Group:
	case eCriteria_All:
	case eCriteria_Selected:
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		mDate.ShowWindow(SW_HIDE);
		mPopup5.ShowWindow(SW_HIDE);
		mPopup6.ShowWindow(SW_HIDE);
		break;
	}
	
	// Set menu item
	mPopup1.SetValue(nID);

	// Special for group display
	if (nID == eCriteria_Group)
		MakeGroup(CFilterItem::eLocal);
	else
		RemoveGroup();

	// Refresh method for new criteria
	if (method_refresh)
		OnSetMethod(mPopup2.GetValue());
}

void CSearchCriteriaLocal::OnSetMethod(UINT nID)
{
	// Show/hide text field as appropriate
	switch(mPopup1.GetValue())
	{
	case eCriteria_From:
	case eCriteria_To:
	case eCriteria_CC:
	case eCriteria_Bcc:
	case eCriteria_Recipient:
	case eCriteria_Correspondent:
	case eCriteria_Sender:
		switch(nID)
		{
		case eAddressMethod_Contains:
		case eAddressMethod_NotContains:
			mText1.ShowWindow(SW_SHOW);
			break;
		case eAddressMethod_IsMe:
		case eAddressMethod_IsNotMe:
			mText1.ShowWindow(SW_HIDE);
			CUnicodeUtils::SetWindowTextUTF8(&mText1, cdstring::null_str);
			break;
		}
		break;
	case eCriteria_DateSent:
	case eCriteria_DateReceived:
		switch(nID)
		{
		case eDateMethod_Before:
		case eDateMethod_On:
		case eDateMethod_After:
			mText3.ShowWindow(SW_HIDE);
			mDate.ShowWindow(SW_SHOW);
			mPopup5.ShowWindow(SW_HIDE);
			mPopup6.ShowWindow(SW_HIDE);
			break;
		case eDateMethod_Is:
		case eDateMethod_IsNot:
			mText3.ShowWindow(SW_HIDE);
			mDate.ShowWindow(SW_HIDE);
			mPopup5.ShowWindow(SW_SHOW);
			mPopup6.ShowWindow(SW_HIDE);
			break;
		case eDateMethod_IsWithin:
		case eDateMethod_IsNotWithin:
			mText3.ShowWindow(SW_SHOW);
			mDate.ShowWindow(SW_HIDE);
			mPopup5.ShowWindow(SW_HIDE);
			mPopup6.ShowWindow(SW_SHOW);
			break;
		}
		break;
	default:
		break;
	}
	
	// Set menu item
	mPopup2.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetSize(UINT nID)
{
	// Set menu item
	mPopup3.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetDateRel(UINT nID)
{
	// Set menu item
	mPopup5.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetDateWithin(UINT nID)
{
	// Set menu item
	mPopup6.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetMode(UINT nID)
{
	// Set menu item
	mPopup4.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetSearchSet(UINT nID)
{
	// Set menu item
	mPopup7.SetValue(nID);
}

void CSearchCriteriaLocal::OnSetSearchSetMethod(UINT nID)
{
	// Set menu item
	mPopup8.SetValue(nID);
}

// Return typed in field
void CSearchCriteriaLocal::OnTextReturn()
{
	GetParentOwner()->PostMessage(WM_COMMAND, IDC_SEARCH_SEARCH);
}

void CSearchCriteriaLocal::InitLabelNames()
{
	// Change name of labels
	for(short i = eCriteria_Label1; i < eCriteria_Label1 + NMessage::eMaxLabels; i++)
		CUnicodeUtils::ModifyMenuUTF8(mPopup1.GetPopupMenu(), i, MF_BYCOMMAND | MF_STRING, i, CPreferences::sPrefs->mLabels.GetValue()[i - eCriteria_Label1]->name);
}

void CSearchCriteriaLocal::InitSearchSets()
{
	// Remove any existing items from main menu
	short num_menu = mPopup7.GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		mPopup7.GetPopupMenu()->RemoveMenu(0, MF_BYPOSITION);
	
	short menu_id = IDM_SEARCH_SEARCHMETHOD_Start;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(mPopup7.GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());

	// Force title update
	mPopup7.SetValue(0);
	mPopup7.SetValue(IDM_SEARCH_SEARCHMETHOD_Start);
}

CSearchItem* CSearchCriteriaLocal::GetSearchItem() const
{
	switch(mPopup1.GetValue())
	{
	case eCriteria_From:
		return ParseAddress(CSearchItem::eFrom);
	case eCriteria_To:
		return ParseAddress(CSearchItem::eTo);
	case eCriteria_CC:
		return ParseAddress(CSearchItem::eCC);
	case eCriteria_Bcc:
		return ParseAddress(CSearchItem::eBcc);
	case eCriteria_Recipient:
		return ParseAddress(CSearchItem::eRecipient);
	case eCriteria_Correspondent:
		return ParseAddress(CSearchItem::eCorrespondent);
	case eCriteria_Sender:
		return ParseAddress(CSearchItem::eSender);
	case eCriteria_DateSent:
		return ParseDate(true);
	case eCriteria_DateReceived:
		return ParseDate(false);
	case eCriteria_Subject:
		return ParseText(CSearchItem::eSubject);
	case eCriteria_Body:
		return ParseText(CSearchItem::eBody);
	case eCriteria_Text:
		return ParseText(CSearchItem::eText);
	case eCriteria_Header:
		{
			cdstring text1;
			mText1.GetText(text1);

			cdstring text2;
			mText2.GetText(text2);
			text2.trimspace();

			// Strip trailing colon from header field
			if (text2.compare_end(":"))
				text2[text2.length() - 1] = 0;

			// Look for '!' at start of header field as negate item
			if (text2[(cdstring::size_type)0] == '!')
				return new CSearchItem(CSearchItem::eNot, new CSearchItem(CSearchItem::eHeader, text2.c_str() + 1, text1));
			else
				return new CSearchItem(CSearchItem::eHeader, text2, text1);
		}
	case eCriteria_Size:
		return ParseSize();
	case eCriteria_Recent:
		return ParseFlag(CSearchItem::eRecent, CSearchItem::eOld);
	case eCriteria_Seen:
		return ParseFlag(CSearchItem::eSeen, CSearchItem::eUnseen);
	case eCriteria_Answered:
		return ParseFlag(CSearchItem::eAnswered, CSearchItem::eUnanswered);
	case eCriteria_Flagged:
		return ParseFlag(CSearchItem::eFlagged, CSearchItem::eUnflagged);
	case eCriteria_Deleted:
		return ParseFlag(CSearchItem::eDeleted, CSearchItem::eUndeleted);
	case eCriteria_Draft:
		return ParseFlag(CSearchItem::eDraft, CSearchItem::eUndraft);
	case eCriteria_Label1:
	case eCriteria_Label2:
	case eCriteria_Label3:
	case eCriteria_Label4:
	case eCriteria_Label5:
	case eCriteria_Label6:
	case eCriteria_Label7:
	case eCriteria_Label8:
		return ParseLabel(CSearchItem::eLabel, mPopup1.GetValue() - eCriteria_Label1);
	case eCriteria_SearchSet:
		{
			cdstring title = CUnicodeUtils::GetWindowTextUTF8(mPopup7);
			
			const CSearchItem* found = CPreferences::sPrefs->mSearchStyles.GetValue().FindStyle(title)->GetSearchItem();
			
			// May need to negate
			if (mPopup8.GetValue() == eSearchSetMethod_Is)
				return (found ? new CSearchItem(CSearchItem::eNamedStyle, cdstring(title)) : NULL);
			else
				return (found ? new CSearchItem(CSearchItem::eNot, new CSearchItem(CSearchItem::eNamedStyle, cdstring(title))) : NULL);
		}
	case eCriteria_Group:
		return mGroupItems->ConstructSearch();
	case eCriteria_All:
		return new CSearchItem(CSearchItem::eAll);
	case eCriteria_Selected:
		return new CSearchItem(CSearchItem::eSelected);
	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseAddress(CSearchItem::ESearchType type) const
{
	cdstring text;
	mText1.GetText(text);

	switch(mPopup2.GetValue())
	{
	case eAddressMethod_Contains:
		return new CSearchItem(type, text);
	case eAddressMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, cdstring(text)));
	case eAddressMethod_IsMe:
		return new CSearchItem(type);
	case eAddressMethod_IsNotMe:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type));
	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseDate(bool sent) const
{
	switch(mPopup2.GetValue())
	{
	case eDateMethod_Before:
		return new CSearchItem(sent ? CSearchItem::eSentBefore : CSearchItem::eBefore, mDate.GetDate());
	case eDateMethod_On:
		return new CSearchItem(sent ? CSearchItem::eSentOn : CSearchItem::eOn, mDate.GetDate());
	case eDateMethod_After:
		return new CSearchItem(sent ? CSearchItem::eSentSince : CSearchItem::eSince, mDate.GetDate());

	// Look at relative date popup
	case eDateMethod_Is:
	case eDateMethod_IsNot:
	{
		bool is = (mPopup2.GetValue() == eDateMethod_Is);

		// Set up types for different categories
		CSearchItem::ESearchType typeToday = (sent ? CSearchItem::eSentOn : CSearchItem::eOn);
		CSearchItem::ESearchType typeOther = (sent ? CSearchItem::eSentSince : CSearchItem::eSince);
		CSearchItem::ESearchType typeNot = (sent ? CSearchItem::eSentBefore : CSearchItem::eBefore);

		// Look at menu item chosen
		switch(mPopup5.GetValue())
		{
		case eDateRelMethod_SentToday:
			return new CSearchItem(is ? typeToday : typeNot, static_cast<unsigned long>(CSearchItem::eToday));
		case eDateRelMethod_SentYesterday:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eSinceYesterday));
		case eDateRelMethod_SentWeek:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisWeek));
		case eDateRelMethod_Sent7Days:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eWithin7Days));
		case eDateRelMethod_SentMonth:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisMonth));
		case eDateRelMethod_SentYear:
			return new CSearchItem(is ? typeOther : typeNot, static_cast<unsigned long>(CSearchItem::eThisYear));
		default:
			return NULL;
		}
		break;
	}

	// Look at relative date popup
	case eDateMethod_IsWithin:
	case eDateMethod_IsNotWithin:
	{
		bool is = (mPopup2.GetValue() == eDateMethod_IsWithin);

		// Set up types for different categories
		CSearchItem::ESearchType typeIs = (sent ? CSearchItem::eSentSince : CSearchItem::eSince);
		CSearchItem::ESearchType typeIsNot = (sent ? CSearchItem::eSentBefore : CSearchItem::eBefore);

		// Get numeric value
		cdstring text;
		mText3.GetText(text);
		long size = ::atol(text);
		unsigned long within = (size > 0) ? size : 1;
		if (within > 0x0000FFFF)
			within = 0x0000FFFF;

		// Look at menu item chosen
		switch(mPopup6.GetValue())
		{
		case eDateWithin_Days:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinDays) | within);
		case eDateWithin_Weeks:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinWeeks) | within);
		case eDateWithin_Months:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinMonths) | within);
		case eDateWithin_Years:
			return new CSearchItem(is ? typeIs : typeIsNot, static_cast<unsigned long>(CSearchItem::eWithinYears) | within);
		default:
			return NULL;
		}
		break;
	}

	default:
		return NULL;
	}
}

CSearchItem* CSearchCriteriaLocal::ParseText(CSearchItem::ESearchType type) const
{
	cdstring text;
	mText1.GetText(text);

	if (mPopup2.GetValue() == eTextMethod_Contains)
		return new CSearchItem(type, text);
	else
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text));
}

CSearchItem* CSearchCriteriaLocal::ParseSize() const
{
	cdstring text;
	mText3.GetText(text);
	long size = ::atol(text);
	switch(mPopup3.GetValue())
	{
	case eSize_Bytes:
		break;
	case eSize_KBytes:
		size *= 1024L;
		break;
	case eSize_MBytes:
		size *= 1024L * 1024L;
		break;
	}
	return new CSearchItem((mPopup2.GetValue() == eSizeMethod_Larger) ? CSearchItem::eLarger : CSearchItem::eSmaller, size);
}

CSearchItem* CSearchCriteriaLocal::ParseFlag(CSearchItem::ESearchType type1, CSearchItem::ESearchType type2) const
{
	return new CSearchItem((mPopup2.GetValue() == eFlagMethod_Set) ? type1 : type2);
}

CSearchItem* CSearchCriteriaLocal::ParseLabel(CSearchItem::ESearchType type, unsigned long index) const
{
	if (mPopup2.GetValue() == eFlagMethod_Set)
		return new CSearchItem(type, index);
	else
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, index));
}

void CSearchCriteriaLocal::SetSearchItem(const CSearchItem* spec, bool negate)
{
	long popup1 = eCriteria_From;
	long popup2 = eAddressMethod_Contains;
	long popup3 = eSize_KBytes;
	long popup5 = eDateRelMethod_SentToday;
	long popup6 = eDateWithin_Days;
	long popup7 = IDM_SEARCH_SEARCHMETHOD_Start;
	long popup8 = eSearchSetMethod_Is;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	time_t date = ::time(NULL);

	if (spec)
	{
		switch(spec->GetType())
		{
		case CSearchItem::eAll:			// -
			popup1 = eCriteria_All;
			break;

		case CSearchItem::eAnd:			// CSearchItemList*
			break;

		case CSearchItem::eAnswered:	// -
			popup1 = eCriteria_Answered;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eBcc:			// cdstring*
			popup1 = eCriteria_Bcc;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eBefore:		// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_Before, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eBody:			// cdstring*
			popup1 = eCriteria_Body;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eCC:			// cdstring*
			popup1 = eCriteria_CC;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eDeleted:		// -
			popup1 = eCriteria_Deleted;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eDraft:		// -
			popup1 = eCriteria_Draft;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eFlagged:		// -
			popup1 = eCriteria_Flagged;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eFrom:		// cdstring*
			popup1 = eCriteria_From;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eGroup:		// CSearchItemList*
			popup1 = eCriteria_Group;
			break;

		case CSearchItem::eHeader:		// cdstrpair*
			popup1 = eCriteria_Header;
			if (negate)
				text2 = "!";
			text2 += static_cast<const cdstrpair*>(spec->GetData())->first;
			text1 = static_cast<const cdstrpair*>(spec->GetData())->second;
			break;

		case CSearchItem::eKeyword:		// cdstring*
			break;

		case CSearchItem::eLabel:		// unsigned long
		{
			unsigned long index = reinterpret_cast<unsigned long>(spec->GetData());
			if (index >= NMessage::eMaxLabels)
				index = 0;
			popup1 = eCriteria_Label1 + index;
			popup2 = negate ? eFlagMethod_NotSet : eFlagMethod_Set;
			break;
		}

		case CSearchItem::eLarger:		// long
		{
			popup1 = eCriteria_Size;
			popup2 = eSizeMethod_Larger;
			long size = reinterpret_cast<long>(spec->GetData());
			if (size >= 1024L * 1024L)
			{
				size /= 1024L * 1024L;
				popup3 = eSize_MBytes;
			}
			else if (size >= 1024L)
			{
				size /= 1024L;
				popup3 = eSize_KBytes;
			}
			text3 = size;
			break;
		}

		case CSearchItem::eNew:			// -
			break;

		case CSearchItem::eNot:			// CSearchItem*
			// Do negated - can only be text based items
			SetSearchItem(static_cast<const CSearchItem*>(spec->GetData()), true);
			
			// Must exit now without changing any UI item since they have already been done
			return;

		case CSearchItem::eNumber:		// ulvector* only - no key
			break;

		case CSearchItem::eOld:			// -
			popup1 = eCriteria_Recent;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eOn:			// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_On, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eOr:			// CSearchItemList*
			break;

		case CSearchItem::eRecent:		// -
			popup1 = eCriteria_Recent;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eSeen:		// -
			popup1 = eCriteria_Seen;
			popup2 = eFlagMethod_Set;
			break;

		case CSearchItem::eSender:		// cdstring*
			popup1 = eCriteria_Sender;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eSelected:	// -
			popup1 = eCriteria_Selected;
			break;

		case CSearchItem::eSentBefore:	// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_Before, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSentOn:		// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_On, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSentSince:	// date
			popup1 = eCriteria_DateSent;
			popup2 = GetDatePopup(spec, eDateMethod_After, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSince:		// date
			popup1 = eCriteria_DateReceived;
			popup2 = GetDatePopup(spec, eDateMethod_After, text1, text3, popup5, popup6);
			date = reinterpret_cast<time_t>(spec->GetData());
			break;

		case CSearchItem::eSmaller:		// long
		{
			popup1 = eCriteria_Size;
			popup2 = eSizeMethod_Smaller;
			long size = reinterpret_cast<long>(spec->GetData());
			if (size >= 1024L)
			{
				size /= 1024L;
				popup3 = eSize_KBytes;
			}
			else if (size >= 1024L * 1024L)
			{
				size /= 1024L * 1024L;
				popup3 = eSize_MBytes;
			}
			text3 = size;
			break;
		}

		case CSearchItem::eSubject:		// cdstring*
			popup1 = eCriteria_Subject;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eText:			// cdstring*
			popup1 = eCriteria_Text;
			text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = negate ? eTextMethod_NotContains : eTextMethod_Contains;
			break;

		case CSearchItem::eTo:			// cdstring*
			popup1 = eCriteria_To;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eUID:			// ulvector*
			break;

		case CSearchItem::eUnanswered:	// -
			popup1 = eCriteria_Answered;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUndeleted:	// -
			popup1 = eCriteria_Deleted;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUndraft:		// -
			popup1 = eCriteria_Draft;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUnflagged:	// -
			popup1 = eCriteria_Flagged;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eUnkeyword:	// cdstring*
			break;

		case CSearchItem::eUnseen:		// -
			popup1 = eCriteria_Seen;
			popup2 = eFlagMethod_NotSet;
			break;

		case CSearchItem::eRecipient:		// cdstring*
			popup1 = eCriteria_Recipient;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCorrespondent:	// cdstring*
			popup1 = eCriteria_Correspondent;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? (negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eNamedStyle:	// -
			popup1 = eCriteria_SearchSet;
			popup7 = CPreferences::sPrefs->mSearchStyles.GetValue().FindIndexOf(*static_cast<const cdstring*>(spec->GetData())) + IDM_SEARCH_SEARCHMETHOD_Start;
			popup8 = negate ? eSearchSetMethod_IsNot : eSearchSetMethod_Is;
			break;
		}
	}

	OnSetCriteria(popup1);
	OnSetMethod(popup2);
	OnSetSize(popup3);
	CUnicodeUtils::SetWindowTextUTF8(&mText1, text1);
	CUnicodeUtils::SetWindowTextUTF8(&mText2, text2);
	CUnicodeUtils::SetWindowTextUTF8(&mText3, text3);
	mDate.SetDate(date);
	mPopup5.SetValue(popup5);
	OnSetSearchSet(popup7);
	OnSetSearchSetMethod(popup8);
	
	// Set group contents
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
		mGroupItems->InitGroup(CFilterItem::eLocal, spec);
}

long CSearchCriteriaLocal::GetDatePopup(const CSearchItem* spec, long original, cdstring& text1, cdstring& text3, long& popup5, long& popup6) const
{
	switch(reinterpret_cast<unsigned long>(spec->GetData()))
	{
	// Relative date
	case CSearchItem::eToday:
	case CSearchItem::eSinceYesterday:
	case CSearchItem::eThisWeek:
	case CSearchItem::eWithin7Days:
	case CSearchItem::eThisMonth:
	case CSearchItem::eThisYear:
	{
		// Set relative popup
		switch(reinterpret_cast<unsigned long>(spec->GetData()))
		{
		case CSearchItem::eToday:
		default:
			popup5 = eDateRelMethod_SentToday;
			return (original == eDateMethod_On) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eSinceYesterday:
			popup5 = eDateRelMethod_SentYesterday;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisWeek:
			popup5 = eDateRelMethod_SentWeek;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eWithin7Days:
			popup5 = eDateRelMethod_Sent7Days;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisMonth:
			popup5 = eDateRelMethod_SentMonth;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		case CSearchItem::eThisYear:
			popup5 = eDateRelMethod_SentYear;
			return (original == eDateMethod_After) ? eDateMethod_Is : eDateMethod_IsNot;
		}
	}
	
	default:;
	}

	switch(reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinMask)
	{
	// Relative date
	case CSearchItem::eWithinDays:
	case CSearchItem::eWithinWeeks:
	case CSearchItem::eWithinMonths:
	case CSearchItem::eWithinYears:
	{
		unsigned long within = reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinValueMask;

		// Set relative popup
		switch(reinterpret_cast<unsigned long>(spec->GetData()) & CSearchItem::eWithinMask)
		{
		case CSearchItem::eWithinDays:
		default:
			popup6 = eDateWithin_Days;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinWeeks:
			popup6 = eDateWithin_Weeks;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinMonths:
			popup6 = eDateWithin_Months;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		case CSearchItem::eWithinYears:
			popup6 = eDateWithin_Years;
			text3 = within;
			return (original == eDateMethod_After) ? eDateMethod_IsWithin : eDateMethod_IsNotWithin;
		}
	}
	
	default:;
	}
	
	// Standard date
	text1 = spec->GenerateDate(false);
	return original;
}