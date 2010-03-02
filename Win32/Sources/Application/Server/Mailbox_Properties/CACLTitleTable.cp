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


// Source for CACLTitleTable class


#include "CACLTitleTable.h"

#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CXStringResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C A C L T I T L E T A B L E
// __________________________________________________________________________________________________



// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CACLTitleTable::CACLTitleTable()
{
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}

// Default destructor
CACLTitleTable::~CACLTitleTable()
{
}

void CACLTitleTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Draw frame
	pDC->FillRect(&inLocalRect, &CDrawUtils::sGrayBrush);
	pDC->SelectObject(CDrawUtils::sWhitePen);
	pDC->MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
	pDC->LineTo(inLocalRect.left, inLocalRect.top);
	pDC->LineTo(inLocalRect.right, inLocalRect.top);
	pDC->SelectObject(CDrawUtils::sDkGrayPen);
	pDC->MoveTo(inLocalRect.right - 2, inLocalRect.top + 1);
	pDC->LineTo(inLocalRect.right - 2, inLocalRect.bottom - 2);
	pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 2);
	pDC->SelectObject(CDrawUtils::sBlackPen);
	pDC->MoveTo(inLocalRect.right - 1, inLocalRect.top);
	pDC->LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
	pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 1);
	pDC->SetTextColor(CDrawUtils::sBtnTextColor);
	pDC->SetBkColor(CDrawUtils::sGrayColor);

	// Only name is drawn as text
	if (inCell.col == 1)
	{
		pDC->SelectObject(CFontCache::GetListFont());
		::DrawClippedStringUTF8(pDC, rsrc::GetString("UI::Titles::ACL"), CPoint(inLocalRect.left + 6, inLocalRect.top + 1), inLocalRect, eDrawString_Left);
	}
	else
	{
		TableIndexT col = inCell.col;
	
		// Adjust for adbk/calendar ACLs
		if (!mMbox)
		{
			if (mAdbk)
			{
				if (col > 4) col += 3;
				else if (col > 3) col += 1;
			}
			else
			{
				if (col > 5) col += 2;
				else if (col > 3) col += 1;
			}
		}
		CIconLoader::DrawIcon(pDC, inLocalRect.left + 1, inLocalRect.top + 1, IDI_ACL_LOOKUP + col - 2, 16);
	}
}
