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


// CMulberryCommon.cp

#include "CMulberryCommon.h"

#include "cdustring.h"

#include <mmsystem.h>

#include <SHLOBJ.H>

// Draw clipped string if too long
void DrawClippedStringUTF8(CDC* pDC, const char* theTxt, CPoint start, CRect clip, EDrawStringAlignment align, EDrawStringClip clip_string)
{
	// Get utf16 string
	cdustring utf16(theTxt);

	// Adjust width for slop
	const short cSlop = 4;
	short width = clip.right - start.x - cSlop;
	size_t len = utf16.length();

	// Check actual width against allowed width
	SIZE size;
	::GetTextExtentPoint32W(*pDC, utf16, len, &size);
	int str_width = size.cx;
	if ((str_width < width) || (len < 2))
	{
		switch(align)
		{
		case eDrawString_Left:
		case eDrawStringJustify:
			// Leave as it is
			break;
		case eDrawString_Center:
			start.x += (width - str_width + cSlop)/2;
			break;
		case eDrawString_Right:
			start.x += width - str_width;
			break;
		}

		// String will fit - draw unmodified
		::ExtTextOutW(*pDC, start.x, start.y, ETO_CLIPPED, clip, utf16, len, NULL);
	}
	else
	{
		switch(clip_string)
		{
		case eClipString_Left:
			// Put 3 '.' at start of string
			if (len > 0)
				utf16[(cdustring::size_type)0] = '.';
			if (len > 1)
				utf16[(cdustring::size_type)1] = '.';
			if (len > 2)
				utf16[(cdustring::size_type)2] = '.';
			break;

		case eClipString_Center:
			// Put 3 '.' at start of string
			if (len > 0)
				utf16[(cdustring::size_type)(len/2)] = '.';
			if (len > 1)
				utf16[(cdustring::size_type)(len/2 - 1)] = '.';
			if (len > 2)
				utf16[(cdustring::size_type)(len/2 + 1)] = '.';
			break;
		
		case eClipString_Right:
			// Put 3 '.' at end of string
			if (len > 0)
				utf16[(cdustring::size_type)(len - 1)] = '.';
			if (len > 1)
				utf16[(cdustring::size_type)(len - 2)] = '.';
			if (len > 2)
				utf16[(cdustring::size_type)(len - 3)] = '.';
			break;
		}

		// Reduce length of string and recalculate width
		for(; len > 0; len--)
		{
			::GetTextExtentPoint32W(*pDC, utf16, len, &size);
			if (size.cx < width)
				break;

			switch(clip_string)
			{
			case eClipString_Left:
				// Move string one char to left
				::memmove(&utf16[(cdustring::size_type)0], &utf16[(cdustring::size_type)1], (len - 1) * sizeof(unichar_t));

				// Shorten string
				if (len > 2)
					utf16[(cdustring::size_type)2] = '.';
				break;
			
			case eClipString_Center:
			{
				// Move string one char to left at its center
				unsigned long center = len/2;
				::memmove(&utf16[(cdustring::size_type)center], &utf16[(cdustring::size_type)(center + 1)], (len - center) * sizeof(unichar_t));
				if (center)
					utf16[(cdustring::size_type)(center - 1)] = '.';
				if (center < len - 1)
					utf16[(cdustring::size_type)(center + 1)] = '.';
				break;
			}

			case eClipString_Right:
				// Add another period
				if (len > 2)
					utf16[(cdustring::size_type)(len - 3)] = '.';

				// Shorten string
				utf16[(cdustring::size_type)len] = 0;
				break;
			}
		}

		// Draw clipped string
		::ExtTextOutW(*pDC, start.x, start.y, ETO_CLIPPED, clip, utf16, len, NULL);
	}
}

void MoveWindowBy(CWnd* pWnd, int dx, int dy, bool redraw)
{
	// Get current position
	CRect r;
	pWnd->GetWindowRect(r);

	// Get any owner
	CWnd* pOwner = pWnd->GetParent();

	// If owned convert rect to client co-ords
	if (pOwner && (pWnd->GetStyle() & WS_CHILD))
		pOwner->ScreenToClient(r);

	// Resize and move
	r.OffsetRect(dx, dy);

	pWnd->MoveWindow(r, redraw);
}

void MoveWindowTo(CWnd* pWnd, int cx, int cy, bool redraw)
{
	// Get current position
	CRect r;
	pWnd->GetWindowRect(r);

	// Get any owner
	CWnd* pOwner = pWnd->GetParent();

	// If owned convert rect to client co-ords
	if (pOwner && (pWnd->GetStyle() & WS_CHILD))
		pOwner->ScreenToClient(r);

	// Resize and move
	r.OffsetRect(cx - r.left, cy - r.top);

	pWnd->MoveWindow(r, redraw);
}

void ResizeWindowBy(CWnd* pWnd, int dx, int dy, bool redraw)
{
	// Get current position
	CRect r;
	pWnd->GetWindowRect(r);

	// Get any owner
	CWnd* pOwner = pWnd->GetParent();

	// If owned convert rect to client co-ords
	if (pOwner && (pWnd->GetStyle() & WS_CHILD))
		pOwner->ScreenToClient(r);

	// Resize and move
	r.right += dx;
	r.bottom += dy;

	pWnd->MoveWindow(r, redraw);
}

void ResizeWindowTo(CWnd* pWnd, int cx, int cy, bool redraw)
{
	// Get current position
	CRect r;
	pWnd->GetWindowRect(r);

	// Get any owner
	CWnd* pOwner = pWnd->GetParent();

	// If owned convert rect to client co-ords
	if (pOwner && (pWnd->GetStyle() & WS_CHILD))
		pOwner->ScreenToClient(r);

	// Resize and move
	r.right = r.left + cx;
	r.bottom = r.top + cy;

	pWnd->MoveWindow(r, redraw);
}

void ExpandChildToFit(CWnd* parent, CWnd* child, bool horiz, bool vert, int border)
{
	// Get client area of container
	CRect client;
	parent->GetClientRect(client);

	// Get current position of child
	CRect r;
	child->GetWindowRect(r);

	// Get any owner
	CWnd* pOwner = child->GetParent();

	// If owned convert rect to client co-ords
	if (pOwner && (child->GetStyle() & WS_CHILD))
		pOwner->ScreenToClient(r);

	// Reset rect based on which dimensions are changing
	if (horiz)
	{
		r.left = 0;
		r.right = client.Width();
	}
	if (vert)
	{
		r.top = 0;
		r.bottom = client.Height();
	}

	// Adjust for border
	if (border)
		r.DeflateRect(border, border);

	// Resize child window
	child->MoveWindow(r);
}

// Make sure some portion of title bar is on screen
void RectOnScreen(CRect& rect, CWnd* owner)
{
	CRect desktop;

	// Check for owener window
	if (owner)
		owner->GetClientRect(desktop);
	else
		// Get task bar pos and see if at top
		::SystemParametersInfo(SPI_GETWORKAREA, 0, (Rect*) desktop, 0);
	desktop.right -= 64;
	desktop.bottom -= 64;

	// Always offset the rect by the left/top margins to account for toolbar placement
	POINT pt;
	pt.x = desktop.left;
	pt.y = desktop.top;
	rect.OffsetRect(pt);

	// Now make sure top-left of window title bar IS on screen
	pt.x = rect.left;
	pt.y = rect.top;
	if (!desktop.PtInRect(pt))
	{
		POINT diff = {0, 0};

		if (rect.left < desktop.left)
			diff.x = desktop.left - rect.left;
		else if (rect.left > desktop.right)
			diff.x = desktop.right - rect.left;

		if (rect.top < desktop.top)
			diff.y = desktop.top - rect.top;
		else if (rect.top > desktop.bottom)
			diff.y = desktop.bottom - rect.top;
		
		rect.OffsetRect(diff);
	}
}
cdstring GetNumericFormat(unsigned long number)
{
	cdstring result;
	result.reserve(64);
	if (number >= 10000000UL)
		::snprintf(result.c_str_mod(), 64, "%d M", (number >> 20) + (number & (1L << 19) ? 1 : 0));
	else if (number >= 10000UL)
		::snprintf(result.c_str_mod(), 64, "%d K", (number >> 10) + (number & (1L << 9) ? 1 : 0));
	else
		::snprintf(result.c_str_mod(), 64, "%d", number);
	
	return result;
}

void PlayNamedSound(const cdstring& name)
{
	if (name.empty() || !::sndPlaySound(name.win_str(), SND_ASYNC))
		::MessageBeep(-1);
}

// Get appropriate flags for tiling menus
UINT AppendMenuFlags(int& pos, bool separator)
{
	static int sMaxMenuEntries = 0;
	
	if (!sMaxMenuEntries)
	{
		// Get screen height for tiling
		int nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
		int nMenuHeight = ::GetSystemMetrics(SM_CYMENU);
		if (nMenuHeight == 0)
			nMenuHeight = 1;
		sMaxMenuEntries = ((double) nScreenHeight / (double) nMenuHeight) * 0.98;
		//CDC* pDC = AfxGetMainWnd()->GetDC();
		//CSize sizeMenu = pDC->GetTextExtent("test", 4);
		//sMaxMenuEntries = ((double) nScreenHeight / (double) sizeMenu.cy) * 0.90;
	}

	// Check for max entries
	bool max_entry = (pos != 0) && (pos % sMaxMenuEntries == 0);

	// If its a separator and menu break, just do separator but do not bump the counter,
	// so that the next (text) item gets to go at the top of the column
	if (max_entry && separator)
		return MF_SEPARATOR;
	else
	{
		pos++;
		return max_entry ? MF_MENUBARBREAK : (separator ? MF_SEPARATOR : 0);
	}
}

bool BrowseForFolder(const cdstring& title, cdstring& folder, CWnd* owner)
{
	bool result = false;

	BROWSEINFO bi;
	TCHAR lpBuffer[MAX_PATH];
	LPITEMIDLIST pidlBrowse;
	HRESULT hr;
	LPMALLOC lpMalloc = NULL;
	
	hr = ::SHGetMalloc(&lpMalloc);

	CString ttitle = title.win_str();

	bi.hwndOwner = owner->GetSafeHwnd();
	bi.pidlRoot = NULL;
	bi.pszDisplayName = lpBuffer;
	bi.lpszTitle = ttitle;
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;
	pidlBrowse = ::SHBrowseForFolder(&bi);
	if (pidlBrowse)
	{
		if (::SHGetPathFromIDList(pidlBrowse, lpBuffer))
		{
			folder = lpBuffer;
			folder += '\\';
			result = true;
		}
		
		if (lpMalloc)
			lpMalloc->Free(pidlBrowse);
	}
	
	if (lpMalloc)
		lpMalloc->Release();
	
	return result;
}

bool GetSpecialFolderPath(int nFolder, cdstring& folder)
{
	// Try Shell folder first
	bool result = false;
	LPITEMIDLIST pidl = NULL;
	LPMALLOC pMalloc = NULL;

	::SHGetMalloc(&pMalloc);

	if (::SHGetSpecialFolderLocation(NULL, nFolder, &pidl) == NOERROR)
	{
		CString temp;
		if (::SHGetPathFromIDList(pidl, temp.GetBuffer(MAX_PATH)))
		{
			temp.ReleaseBuffer(-1);

			// Must end with dir-delim
			folder = temp;
			if (folder[folder.length() - 1] != os_dir_delim)
				folder += os_dir_delim;
			
			result = true;
		}

		if (pMalloc)
			pMalloc->Free(pidl);
	}
	
	// Clean up
	if (pMalloc)
		pMalloc->Release();
	
	return result;
}

bool ResolveShortcut(cdstring& path)
{
	// Check for .lnk extension
	if (!path.compare_end(".lnk", true))
		return true;

	TCHAR szLinkName[_MAX_PATH];
	if (::AfxResolveShortcut(AfxGetMainWnd(), path.win_str(), szLinkName, _MAX_PATH))
	{
		path = szLinkName;
		return true;
	}
	else
		return false;
}

UINT CheckboxToggle(CButton* btn)
{
	UINT result = BST_UNCHECKED;
	switch(btn->GetCheck())
	{
	case BST_CHECKED:
	default:
		result = BST_UNCHECKED;
		break;
	case BST_UNCHECKED:
	case BST_INDETERMINATE:
		result = BST_CHECKED;
		break;
	}
	
	btn->SetCheck(result);
	return result;
}

void OnUpdateMenuTitle(CCmdUI* pCmdUI, CString& new_title)
{
	// Check for accelarator key and add to item
	if (pCmdUI->m_pMenu)
	{
		CString accel;
		pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nIndex, accel, MF_BYPOSITION);
		int accel_pos = accel.ReverseFind('\t');
		if (accel_pos >= 0)
			new_title += accel.Mid(accel_pos);
	}
	pCmdUI->SetText(new_title);
}

StNoRedraw::StNoRedraw(CWnd* wnd)
{
	mWnd = wnd;

	if (mWnd != NULL)
	{
		// Only turn of redraw if its actually not visible
		mWasVisible = mWnd->IsWindowVisible();
		if (mWasVisible)
			mWnd->SetRedraw(false);
	}
}


StNoRedraw::~StNoRedraw()
{
	// Turn on redraw if it was previously visible
	if ((mWnd != NULL) && mWasVisible)
	{
		mWnd->SetRedraw(true);
		mWnd->RedrawWindow();
	}
}
