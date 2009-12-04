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

// Dictionary Page Scroller

#include "CDictionaryPageList.h"

#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

BEGIN_MESSAGE_MAP(CDictionaryPageList, CListBox)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CDictionaryPageList::CDictionaryPageList()
{
	mWordTop = NULL;
	mWordBottom = NULL;
	mPageCount = NULL;
	mDisplayPages = false;
}

CDictionaryPageList::~CDictionaryPageList()
{
}

// Manual scroll
void CDictionaryPageList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch(nSBCode)
	{
	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		if (mSpeller->CurrentPage() != mSpeller->CountPages())
			SetPage(mSpeller->CurrentPage() + 1);
		break;
	case SB_PAGEUP:
	case SB_LINEUP:
		if (mSpeller->CurrentPage() != 0)
			SetPage(mSpeller->CurrentPage() - 1);
		break;
	case SB_THUMBTRACK:
		SetPage(nPos);
		break;
	case SB_THUMBPOSITION:
		SetPage(nPos);
		break;
	case SB_TOP:
		SetPage(0);
		break;
	case SB_BOTTOM:
		SetPage(mSpeller->CountPages());
		break;
	}
}

// Display dictionary in list
bool CDictionaryPageList::DisplayDictionary(const char* word)
{
	bool result = false;

	// Get page buffer and find closest location if switching from guesses
	const char* buf;
	size_t buflen;
	int match = mSpeller->GetPage(word, &buf, &buflen);
	SetScrollPos(SB_HORZ, mSpeller->CurrentPage(), true);

	// Set page counter if more than one
	if (mPageCount && mDisplayPages && mSpeller->CountPages())
	{
		char str[256];
		::snprintf(str, 256, "%d of %d", mSpeller->CurrentPage() + 1, mSpeller->CountPages() + 1);
		CUnicodeUtils::SetWindowTextUTF8(mPageCount, str);
	}

	{
		SetRedraw(false);
		
		// Remove any existing
		ResetContent();

		// Do for each word
		const char* start = buf;
		const char* stop = buf;
		const char* current = buf;
		int row = 0;
		while(current - buf < buflen)
		{
			stop = current;
			AddString(cdstring(current).win_str());
			row++;
			current += ::strlen(current) + 1;
		}

		// Set word range if more than one page
		if (mWordTop && mDisplayPages && mSpeller->CountPages())
			CUnicodeUtils::SetWindowTextUTF8(mWordTop, start);
		if (mWordBottom && mDisplayPages && mSpeller->CountPages())
			CUnicodeUtils::SetWindowTextUTF8(mWordBottom, stop);

		// Only select row if some entries and was previously showing guesses
		if ((row > 1) && (match >= 0))
		{
			// Select matching cell
			SetCurSel(match);
			SetSel(match);
			result = true;
		}
		
		SetRedraw(true);
	}
	
	return result;
}

void CDictionaryPageList::DisplayPages(bool display)
{
	mDisplayPages = display;
	if (!mDisplayPages)
	{
		CUnicodeUtils::SetWindowTextUTF8(mWordTop, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mWordBottom, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mPageCount, cdstring::null_str);
		ShowScrollBar(SB_HORZ, false);
		EnableScrollBarCtrl(SB_HORZ, false);
	}
	else
	{
		ShowScrollBar(SB_BOTH, true);
		EnableScrollBarCtrl(SB_HORZ, true);
	}
}

void CDictionaryPageList::SetPage(SInt32 inNewValue)
{
	mSpeller->SetPage(inNewValue);
	DisplayDictionary(NULL);
}

void CDictionaryPageList::InitPage(void)
{
	// Do after dictionary has loaded as possible change to page number
	SetPage(0);
	SetScrollRange(SB_HORZ, 0, mSpeller->CountPages(), false);
	SetScrollPos(SB_HORZ, mSpeller->CurrentPage(), true);
	
	// Adjust size
	CRect prect;
	mPageCount->GetWindowRect(prect);
	CRect srect;
	CWnd* sbar = GetDlgItem(SB_HORZ);
	if (sbar)
	{
		sbar->GetWindowRect(srect);
	
		srect.left += (prect.right - prect.left);
		sbar->MoveWindow(srect);
	}
}

LRESULT CDictionaryPageList::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// Inherited
	LRESULT result = CListBox::WindowProc(nMsg, wParam, lParam);
	
	// Force scroll bar
	if (mDisplayPages)
	{
		SetScrollRange(SB_HORZ, 0, mSpeller->CountPages(), false);
		SetScrollPos(SB_HORZ, mSpeller->CurrentPage(), true);
	}
	
	return result;
}