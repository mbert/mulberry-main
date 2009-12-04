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


// Source for CSearchCriteriaSIEVE class

#include "CSearchCriteriaSIEVE.h"

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
	eCriteria_From = IDM_SIEVE_CRITERIA_FROM,
	eCriteria_To,
	eCriteria_CC,
	eCriteria_Bcc,
	eCriteria_Recipient,
	eCriteria_Correspondent,
	eCriteria_Sender,
	eCriteria_Subject,
	eCriteria_Header,
	eCriteria_Size,
	eCriteria_Group,
	eCriteria_All
};

enum
{
	eAddressMethod_Is = IDM_SIEVE_ADDRESSMETHOD_IS,
	eAddressMethod_IsNot,
	eAddressMethod_Contains,
	eAddressMethod_NotContains,
	eAddressMethod_StartsWith,
	eAddressMethod_NotStartsWith,
	eAddressMethod_EndsWith,
	eAddressMethod_NotEndsWith,
	eAddressMethod_IsMe,
	eAddressMethod_IsNotMe,
	eAddressMethod_Matches,
	eAddressMethod_NotMatches
};

enum
{
	eStringMethod_Is = IDM_SIEVE_TEXTMETHOD_IS,
	eStringMethod_IsNot,
	eStringMethod_Contains,
	eStringMethod_NotContains,
	eStringMethod_StartsWith,
	eStringMethod_NotStartsWith,
	eStringMethod_EndsWith,
	eStringMethod_NotEndsWith,
	eStringMethod_Matches,
	eStringMethod_NotMatches
};

enum
{
	eSizeMethod_Larger = IDM_SEARCH_SIZEMETHOD_LARGER,
	eSizeMethod_Smaller
};

enum
{
	eSize_Bytes = IDM_SEARCH_SIZE_BYTES,
	eSize_KBytes,
	eSize_MBytes
};

enum
{
	eMode_Or = IDM_SEARCH_MODE_OR,
	eMode_And
};

BEGIN_MESSAGE_MAP(CSearchCriteriaSIEVE, CSearchCriteria)
	ON_WM_CREATE()
	
	ON_COMMAND_RANGE(IDM_SIEVE_CRITERIA_FROM, IDM_SIEVE_CRITERIA_ALL, OnSetCriteria)
	ON_COMMAND_RANGE(IDM_SIEVE_ADDRESSMETHOD_IS, IDM_SIEVE_ADDRESSMETHOD_NOTMATCHES, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SIEVE_TEXTMETHOD_IS, IDM_SIEVE_TEXTMETHOD_NOTMATCHES, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_SIZEMETHOD_LARGER, IDM_SEARCH_SIZEMETHOD_SMALLER, OnSetMethod)
	ON_COMMAND_RANGE(IDM_SEARCH_SIZE_BYTES, IDM_SEARCH_SIZE_MBYTES, OnSetSize)
	ON_COMMAND_RANGE(IDM_SEARCH_MODE_OR, IDM_SEARCH_MODE_AND, OnSetMode)

	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT1, OnTextReturn)
	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT2, OnTextReturn)
	ON_COMMAND(IDC_SEARCHCRITERIA_TEXT3, OnTextReturn)
END_MESSAGE_MAP()

// Default constructor
CSearchCriteriaSIEVE::CSearchCriteriaSIEVE()
{
}

// Default destructor
CSearchCriteriaSIEVE::~CSearchCriteriaSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CSearchCriteriaSIEVE::Create(const CRect& rect, CWnd* pParentWnd)
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

const int cPopup2Width = 138;
const int cPopup2Height = cPopupHeight;
const int cPopup2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup2VOffset = cPopup1VOffset;

const int cText1Width = 112;
const int cText1Height = cEditHeight;
const int cText1HOffset = cPopup2HOffset + cPopup2Width + 4;
const int cText1VOffset = cPopup1VOffset;

const int cText2Width = 138;
const int cText2Height = cEditHeight;
const int cText2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cText2VOffset = cText1VOffset;

const int cText3Width = 48;
const int cText3Height = cEditHeight;
const int cText3HOffset = cPopup2HOffset + cPopup2Width + 4;
const int cText3VOffset = cText1VOffset;

const int cPopup3Width = 64;
const int cPopup3Height = cPopupHeight;
const int cPopup3HOffset = cText3HOffset + cText3Width + 4;
const int cPopup3VOffset = cPopup1VOffset;

int CSearchCriteriaSIEVE::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSearchCriteria::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Popups
	CString s;
	CRect r = CRect(cPopup1HOffset, cPopup1VOffset, cPopup1HOffset + cPopup1Width, cPopup1VOffset + cPopup1Height + small_offset);
	mPopup1.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup1.SetMenu(IDR_POPUP_SIEVE_CRITERIA);
	mPopup1.SetValue(IDM_SIEVE_CRITERIA_FROM);
	mPopup1.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup1, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup2HOffset, cPopup2VOffset, cPopup2HOffset + cPopup2Width, cPopup2VOffset + cPopup2Height + small_offset);
	mPopup2.Create(_T(""), r, &mHeader, IDC_SEARCHCRITERIA_POPUP2, IDC_STATIC, IDI_POPUPBTN);
	mPopup2.SetMenu(IDR_POPUP_SIEVE_ADDRESSMETHOD);
	mPopup2.SetValue(IDM_SIEVE_ADDRESSMETHOD_CONTAINS);
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

	return 0;
}

// Tell window to focus on this one
bool CSearchCriteriaSIEVE::SetInitialFocus()
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

long CSearchCriteriaSIEVE::ShowOrAnd(bool show)
{
	if (show)
		mPopup4.ShowWindow(SW_SHOW);
	else
		mPopup4.ShowWindow(SW_HIDE);
	
	return 0;
}

bool CSearchCriteriaSIEVE::IsOr() const
{
	return (mPopup4.GetValue() == eMode_Or);
}

void CSearchCriteriaSIEVE::SetOr(bool use_or)
{
	mPopup4.SetValue(use_or ? eMode_Or : eMode_And);
}

void CSearchCriteriaSIEVE::OnSetCriteria(UINT nID)
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
		if (mPopup2.GetMenuID() != IDR_POPUP_SIEVE_ADDRESSMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SIEVE_ADDRESSMETHOD);
			mPopup2.SetValue(IDM_SIEVE_ADDRESSMETHOD_IS);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Subject:
		if (mPopup2.GetMenuID() != IDR_POPUP_SIEVE_TEXTMETHOD)
		{
			mPopup2.SetMenu(IDR_POPUP_SIEVE_TEXTMETHOD);
			mPopup2.SetValue(IDM_SIEVE_TEXTMETHOD_IS);
		}
		mPopup2.ShowWindow(SW_SHOW);
		method_refresh = true;
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		break;
	case eCriteria_Header:
		CUnicodeUtils::SetWindowTextUTF8(&mText2, cdstring::null_str);
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_SHOW);
		mText2.ShowWindow(SW_SHOW);
		mText3.ShowWindow(SW_HIDE);
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
		break;
	case eCriteria_Group:
	case eCriteria_All:
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText1.ShowWindow(SW_HIDE);
		mText2.ShowWindow(SW_HIDE);
		mText3.ShowWindow(SW_HIDE);
		break;
	}
	
	// Set menu item
	mPopup1.SetValue(nID);

	// Special for group display
	if (nID == eCriteria_Group)
		MakeGroup(CFilterItem::eSIEVE);
	else
		RemoveGroup();

	// Refresh method for new criteria
	if (method_refresh)
		OnSetMethod(mPopup2.GetValue());
}

void CSearchCriteriaSIEVE::OnSetMethod(UINT nID)
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
		case eAddressMethod_Is:
		case eAddressMethod_IsNot:
		case eAddressMethod_Contains:
		case eAddressMethod_NotContains:
		case eAddressMethod_StartsWith:
		case eAddressMethod_NotStartsWith:
		case eAddressMethod_EndsWith:
		case eAddressMethod_NotEndsWith:
		case eAddressMethod_Matches:
			mText1.ShowWindow(SW_SHOW);
			break;
		case eAddressMethod_IsMe:
		case eAddressMethod_IsNotMe:
			mText1.ShowWindow(SW_HIDE);
			CUnicodeUtils::SetWindowTextUTF8(&mText1, cdstring::null_str);
			break;
		}
		break;
	default:
		break;
	}
	
	// Set menu item
	mPopup2.SetValue(nID);
}

void CSearchCriteriaSIEVE::OnSetSize(UINT nID)
{
	// Set menu item
	mPopup3.SetValue(nID);
}

void CSearchCriteriaSIEVE::OnSetMode(UINT nID)
{
	// Set menu item
	mPopup4.SetValue(nID);
}

// Return typed in field
void CSearchCriteriaSIEVE::OnTextReturn()
{
	GetParentOwner()->PostMessage(WM_COMMAND, IDC_SEARCH_SEARCH);
}

void CSearchCriteriaSIEVE::InitSearchSets()
{
	// Remove any existing items from main menu
	short num_menu = mPopup2.GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		mPopup2.GetPopupMenu()->RemoveMenu(0, MF_BYPOSITION);
	
	short menu_id = IDM_SEARCH_SEARCHMETHOD_Start;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(mPopup2.GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());

	// Force max/min update
	mPopup2.SetValue(IDM_SEARCH_SEARCHMETHOD_Start);
}

CSearchItem* CSearchCriteriaSIEVE::GetSearchItem() const
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
	case eCriteria_Subject:
		return ParseText(CSearchItem::eSubject);
	case eCriteria_Header:
		{
			cdstring text1;
			mText1.GetText(text1);
			cdstring text2;
			mText2.GetText(text2);
			return new CSearchItem(CSearchItem::eHeader, text2, text1);
		}
	case eCriteria_Size:
		return ParseSize();
	case eCriteria_Group:
		return mGroupItems->ConstructSearch();
	case eCriteria_All:
		return new CSearchItem(CSearchItem::eAll);
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseAddress(CSearchItem::ESearchType type) const
{
	cdstring text;
	mText1.GetText(text);

	switch(mPopup2.GetValue())
	{
	case eAddressMethod_Is:
		return new CSearchItem(type, text, CSearchItem::eIs);
	case eAddressMethod_IsNot:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eIs));
	case eAddressMethod_Contains:
		return new CSearchItem(type, text, CSearchItem::eContains);
	case eAddressMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eContains));
	case eAddressMethod_StartsWith:
		return new CSearchItem(type, text, CSearchItem::eStartsWith);
	case eAddressMethod_NotStartsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eStartsWith));
	case eAddressMethod_EndsWith:
		return new CSearchItem(type, text, CSearchItem::eEndsWith);
	case eAddressMethod_NotEndsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eEndsWith));
	case eAddressMethod_IsMe:
		return new CSearchItem(type);
	case eAddressMethod_IsNotMe:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type));
	case eAddressMethod_Matches:
		return new CSearchItem(type, text, CSearchItem::eMatches);
	case eAddressMethod_NotMatches:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eMatches));
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseText(CSearchItem::ESearchType type) const
{
	cdstring text;
	mText1.GetText(text);

	switch(mPopup2.GetValue())
	{
	case eStringMethod_Is:
		return new CSearchItem(type, text, CSearchItem::eIs);
	case eStringMethod_IsNot:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eIs));
	case eStringMethod_Contains:
		return new CSearchItem(type, text, CSearchItem::eContains);
	case eStringMethod_NotContains:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eContains));
	case eStringMethod_StartsWith:
		return new CSearchItem(type, text, CSearchItem::eStartsWith);
	case eStringMethod_NotStartsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eStartsWith));
	case eStringMethod_EndsWith:
		return new CSearchItem(type, text, CSearchItem::eEndsWith);
	case eStringMethod_NotEndsWith:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eEndsWith));
	case eStringMethod_Matches:
		return new CSearchItem(type, text, CSearchItem::eMatches);
	case eStringMethod_NotMatches:
		return new CSearchItem(CSearchItem::eNot, new CSearchItem(type, text, CSearchItem::eMatches));
	default:
		return nil;
	}
}

CSearchItem* CSearchCriteriaSIEVE::ParseSize() const
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

void CSearchCriteriaSIEVE::SetSearchItem(const CSearchItem* spec, bool negate)
{
	long popup1 = eCriteria_From;
	long popup2 = eAddressMethod_Contains;
	long popup3 = eSize_KBytes;
	cdstring text1;
	cdstring text2;
	cdstring text3;

	if (spec)
	{
		switch(spec->GetType())
		{
		case CSearchItem::eAll:			// -
			popup1 = eCriteria_All;
			break;

		case CSearchItem::eAnd:			// CSearchItemList*
			break;

		case CSearchItem::eBcc:			// cdstring*
			popup1 = eCriteria_Bcc;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCC:			// cdstring*
			popup1 = eCriteria_CC;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;
		case CSearchItem::eFrom:		// cdstring*
			popup1 = eCriteria_From;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eGroup:		// CSearchItemList*
			popup1 = eCriteria_Group;
			break;

		case CSearchItem::eHeader:		// cdstrpair*
			popup1 = eCriteria_Header;
			text2 = static_cast<const cdstrpair*>(spec->GetData())->first;
			text1 = static_cast<const cdstrpair*>(spec->GetData())->second;
			break;

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

		case CSearchItem::eNot:			// CSearchItem*
			// Do negated - can only be text based items
			SetSearchItem(static_cast<const CSearchItem*>(spec->GetData()), true);

			// Must exit now without changing any UI item since they have already been done
			return;

		case CSearchItem::eOr:			// CSearchItemList*
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
			popup2 = GetPopup2(spec, negate, false);
			break;

		case CSearchItem::eTo:			// cdstring*
			popup1 = eCriteria_To;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eRecipient:	// cdstring*
			popup1 = eCriteria_Recipient;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eCorrespondent:	// cdstring*
			popup1 = eCriteria_Correspondent;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		case CSearchItem::eSender:	// cdstring*
			popup1 = eCriteria_Sender;
			if (spec->GetData())
				text1 = *static_cast<const cdstring*>(spec->GetData());
			popup2 = spec->GetData() ? GetPopup2(spec, negate, true) :
									(negate ? eAddressMethod_IsNotMe : eAddressMethod_IsMe);
			break;

		default:
			break;
		}
	}

	OnSetCriteria(popup1);
	OnSetMethod(popup2);
	OnSetSize(popup3);
	CUnicodeUtils::SetWindowTextUTF8(&mText1, text1);
	CUnicodeUtils::SetWindowTextUTF8(&mText2, text2);
	CUnicodeUtils::SetWindowTextUTF8(&mText3, text3);
	
	// Set group contents
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
		mGroupItems->InitGroup(CFilterItem::eSIEVE, spec);
}

long CSearchCriteriaSIEVE::GetPopup2(const CSearchItem* spec, bool negate, bool addr) const
{
	switch(spec->GetMatchType())
	{
	case CSearchItem::eDefault:
	case CSearchItem::eContains:
	default:
		return addr ?
				(negate ? eAddressMethod_NotContains : eAddressMethod_Contains) :
				(negate ? eStringMethod_NotContains : eStringMethod_Contains);
	case CSearchItem::eIs:
		return addr ?
				(negate ? eAddressMethod_IsNot : eAddressMethod_Is) :
				(negate ? eStringMethod_IsNot : eStringMethod_Is);
	case CSearchItem::eStartsWith:
		return addr ?
				(negate ? eAddressMethod_NotStartsWith : eAddressMethod_StartsWith) :
				(negate ? eStringMethod_NotStartsWith : eStringMethod_StartsWith);
	case CSearchItem::eEndsWith:
		return addr ?
				(negate ? eAddressMethod_NotEndsWith : eAddressMethod_EndsWith) :
				(negate ? eStringMethod_NotEndsWith : eStringMethod_EndsWith);
	case CSearchItem::eMatches:
		return addr ?
				(negate ? eAddressMethod_NotMatches : eAddressMethod_Matches) :
				(negate ? eStringMethod_NotMatches : eStringMethod_Matches);
	}
}
