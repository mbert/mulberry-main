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


// CDrawUtils.h : header file
//

#ifndef __CDRAWUTILS__MULBERRY__
#define __CDRAWUTILS__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CDrawUtils

class CDrawUtils
{

public:
	static CPen sBlackPen;
	static CPen sWhitePen;
	static CPen sGrayPen;
	static CPen sDkGrayPen;
	static CPen sSmallHilitePen;
	static CPen sHilitePen;
	static CBrush sBlackBrush;
	static CBrush sWhiteBrush;
	static CBrush sGrayBrush;
	static CBrush sDkGrayBrush;

	static COLORREF sWhiteColor;
	static COLORREF sBlackColor;
	static COLORREF sGrayColor;
	static COLORREF sDkGrayColor;
	static COLORREF sBtnTextColor;
	static COLORREF sGrayTextColor;
	static COLORREF sHiliteColor;
	static COLORREF sHiliteTextColor;
	static COLORREF sWindowTextColor;

	static void Draw3DFrame(CDC* pDC, const CRect& rect,
									bool selected = false,
									bool enabled = true,
									bool frame = true,
									bool filled = true);
	static void DrawSimpleFrame(CDC* pDC, const CRect& rect,
									bool selected = false,
									bool enabled = true,
									bool frame = true,
									bool filled = true);
	static void DrawFocusFrame(CDC* pDC, const CRect& rect, bool small_frame = false);

	static void DrawSimpleLine(CDC* pDC, const CRect& rect,
									bool vertical = true,
									bool selected = false,
									bool enabled = true);

private:
	// Never created
	CDrawUtils();
	~CDrawUtils();
};

// Stack based class to save restore drawing state
class StDCState
{
public:
	CDC* mDC;
	int mState;

	StDCState(CDC* pDC)
		{ mState = pDC->SaveDC(); mDC = pDC; }
	~StDCState()
		{ mDC->RestoreDC(mState); }
};

/////////////////////////////////////////////////////////////////////////////

#endif
