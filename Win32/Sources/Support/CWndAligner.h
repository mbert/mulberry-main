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


// CWndaligner.h : header file
//

#ifndef __CWNDALIGNER__MULBERRY__
#define __CWNDALIGNER__MULBERRY__

#include "ptrvector.h"

/////////////////////////////////////////////////////////////////////////////
// CWndAligner view

// Class that encapsulates child windows to be aligned

class CWndAlignment
{
	friend class CWndAligner;

public:
	enum
	{
		eAlign_Left = 0,
		eAlign_Top,
		eAlign_Right,
		eAlign_Bottom
	};

	enum EAlignmentType
	{
		eAlign_TopLeft = 0,
		eAlign_TopRight,
		eAlign_BottomLeft,
		eAlign_BottomRight,
		eAlign_TopWidth,
		eAlign_BottomWidth,
		eAlign_LeftHeight,
		eAlign_RightHeight,
		eAlign_WidthHeight
	};

	CWndAlignment(CWnd* child, bool left = true, bool top = true, bool right = false, bool bottom = false);
	CWndAlignment(CWnd* child, EAlignmentType type);
	
	~CWndAlignment() {}

	void SetAlignment(bool left, bool top, bool right, bool bottom)
		{ mAlign[eAlign_Left] = left;
		  mAlign[eAlign_Top] = top;
		  mAlign[eAlign_Right] = right;
		  mAlign[eAlign_Bottom] = bottom; }
	void SetAlignment(EAlignmentType type);
	void GetAlignment(bool& left, bool& top, bool& right, bool& bottom) const
		{ left = mAlign[eAlign_Left] ;
		  top = mAlign[eAlign_Top];
		  right = mAlign[eAlign_Right];
		  bottom = mAlign[eAlign_Bottom]; }

protected:
	HWND  mChild;
	CSize mSize;
	bool  mAlign[4];

	bool ParentResizedBy(HDWP& dwp, const CWnd* parent, int dx, int dy);

private:
	void InitAlignment(CWnd* child, bool left, bool top, bool right, bool bottom);
	void GetAlignment(EAlignmentType type, bool& left, bool& top, bool& right, bool& bottom);
};

// Mix-in class that aligns child windows

typedef ptrvector<CWndAlignment> CAlignments;

class CWndAligner
{
public:
	CWndAligner();
	virtual ~CWndAligner() {}

	void MinimumResize(int cx, int cy)
		{ mMinWidth = cx; mMinHeight = cy; }

	void AddAlignment(CWndAlignment* align)
		{ mAligns.push_back(align); }
	void RemoveChildAlignment(CWnd* child);

protected:
	CAlignments mAligns;
	int mCurrentWidth;
	int mCurrentHeight;
	int mMinWidth;
	int mMinHeight;

	void InitResize(int cx, int cy)
		{ mCurrentWidth = cx; mCurrentHeight = cy; }
	void SizeChanged(int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

#endif
