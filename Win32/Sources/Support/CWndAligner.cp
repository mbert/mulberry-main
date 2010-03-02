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


// CWndAligner.cpp : implementation file
//

#include "CWndAligner.h"

/////////////////////////////////////////////////////////////////////////////
// CWndAligner

CWndAlignment::CWndAlignment(CWnd* child, bool left, bool top, bool right, bool bottom)
{
	InitAlignment(child, left, top, right, bottom);
}
	
CWndAlignment::CWndAlignment(CWnd* child, EAlignmentType type)
{
	bool left;
	bool top;
	bool right;
	bool bottom;
	GetAlignment(type, left, top, right, bottom);
	InitAlignment(child, left, top, right, bottom);
}

void CWndAlignment::InitAlignment(CWnd* child, bool left, bool top, bool right, bool bottom)
{
	mChild = child->GetSafeHwnd();
	SetAlignment(left, top, right, bottom);
	
	// Cache current size
	if (mChild)
	{
		CRect rect;
		::GetWindowRect(mChild, rect);
		mSize.cx = rect.Width();
		mSize.cy = rect.Height();
	}
}

void CWndAlignment::GetAlignment(EAlignmentType type, bool& left, bool& top, bool& right, bool& bottom)
{
	switch(type)
	{
	case eAlign_TopLeft:
		left = true;
		top = true;
		right = false;
		bottom = false;
		break;
	case eAlign_TopRight:
		left = false;
		top = true;
		right = true;
		bottom = false;
		break;
	case eAlign_BottomLeft:
		left = true;
		top = false;
		right = false;
		bottom = true;
		break;
	case eAlign_BottomRight:
		left = false;
		top = false;
		right = true;
		bottom = true;
		break;
	case eAlign_TopWidth:
		left = true;
		top = true;
		right = true;
		bottom = false;
		break;
	case eAlign_BottomWidth:
		left = true;
		top = false;
		right = true;
		bottom = true;
		break;
	case eAlign_LeftHeight:
		left = true;
		top = true;
		right = false;
		bottom = true;
		break;
	case eAlign_RightHeight:
		left = false;
		top = true;
		right = true;
		bottom = true;
		break;
	case eAlign_WidthHeight:
		left = true;
		top = true;
		right = true;
		bottom = true;
		break;
	}
}

void CWndAlignment::SetAlignment(EAlignmentType type)
{
	bool left;
	bool top;
	bool right;
	bool bottom;
	GetAlignment(type, left, top, right, bottom);
	SetAlignment(left, top, right, bottom);
}

// CWndAligner columns
bool CWndAlignment::ParentResizedBy(HDWP& dwp, const CWnd* parent, int dx, int dy)
{
	// Only if child exists
	if (!mChild)
		return true;

	// Get current client position
	CRect pos;
	::GetWindowRect(mChild, pos);
	parent->ScreenToClient(pos);
	
	// If current size is non-zero then use that instead of cached value
	if (pos.Width())
		mSize. cx = pos.Width();
	if (pos.Height())
		mSize.cy = pos.Height();

	// Now change edges based on alignment
	if (!mAlign[eAlign_Left])
	{
		pos.left += dx;
		mSize.cx -= dx;
	}
	if (!mAlign[eAlign_Top])
	{
		pos.top += dy;
		mSize.cy -= dy;
	}
	if (mAlign[eAlign_Right])
	{
		pos.right += dx;
		mSize.cx += dx;
	}
	if (mAlign[eAlign_Bottom])
	{
		pos.bottom += dy;
		mSize.cy += dy;
	}
	
	// The mSize value for width/height is the accurate size because it
	// takes into account negative sizes, whereas SetWindowPos will always
	// set zero width/height if those are negative
	pos.right = pos.left + (mSize.cx > 0 ? mSize.cx : 0);
	pos.bottom = pos.top + (mSize.cy > 0 ? mSize.cy : 0);

	// Now move to new position
	if (dwp)
	{
		dwp = ::DeferWindowPos(dwp, mChild, NULL, pos.left, pos.top, pos.Width(), pos.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		return dwp != NULL;
	}
	else
		::SetWindowPos(mChild, NULL, pos.left, pos.top, pos.Width(), pos.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CWndAligner

CWndAligner::CWndAligner()
{
	mCurrentWidth = 0;
	mCurrentHeight = 0;
	mMinWidth = 24;
	mMinHeight = 24;
}

// CWndAligner columns
void CWndAligner::SizeChanged(int cx, int cy)
{
	int dx = std::max(cx, mMinWidth) - mCurrentWidth;
	int dy = std::max(cy, mMinHeight) - mCurrentHeight;

	// Only if changed
	if (!dx && !dy)
		return;
	mCurrentWidth = std::max(cx, mMinWidth);
	mCurrentHeight = std::max(cy, mMinHeight);

	// Get wnd object this is mixed with
	CWnd* wnd = dynamic_cast<CWnd*>(this);

	// If any are hidden we must not use deferred window positioning
	bool do_defer = true;
	for(CAlignments::iterator iter = mAligns.begin(); do_defer && (iter != mAligns.end()); iter++)
		do_defer = ::IsWindowVisible((*iter)->mChild);
	
	// Reposition each child
	HDWP dwp = do_defer ? ::BeginDeferWindowPos(mAligns.size()) : NULL;
	bool ok = true;
	for(CAlignments::iterator iter = mAligns.begin(); ok && (iter != mAligns.end()); iter++)
		ok = (*iter)->ParentResizedBy(dwp, wnd, dx, dy);
	if (dwp)
	{
		if (!::EndDeferWindowPos(dwp))
		{
			// Do it again the ugly way if it fails
			dwp = NULL;
			for(CAlignments::iterator iter = mAligns.begin(); iter != mAligns.end(); iter++)
				(*iter)->ParentResizedBy(dwp, wnd, dx, dy);
		}
	}
}

void CWndAligner::RemoveChildAlignment(CWnd* child)
{
	// Scan alignments looking for ones that match and erase them
	for(int pos = 0; pos != mAligns.size(); pos++)
	{
		if (mAligns[pos]->mChild == child->GetSafeHwnd())
		{
			mAligns.erase(mAligns.begin() + pos);
			pos--;
		}
	}
}
