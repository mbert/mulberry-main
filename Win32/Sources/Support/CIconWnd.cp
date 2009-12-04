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


// CIconWnd.cpp : implementation file
//


#include "CIconWnd.h"

#include "CIconLoader.h"

IMPLEMENT_DYNAMIC(CIconWnd, CStatic)

/////////////////////////////////////////////////////////////////////////////
// CIconWnd

BEGIN_MESSAGE_MAP(CIconWnd, CStatic)
	//{{AFX_MSG_MAP(CIconWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Gray background support

BOOL CIconWnd::SubclassDlgItem(UINT nID, CWnd* pParent, UINT nIDIcon)
{
	// Do inherited
	CStatic::SubclassDlgItem(nID, pParent);

	// Get current rect
	CRect cr;
	GetClientRect(&cr);
	CRect wr;
	GetWindowRect(&wr);

	if (cr.Width() >= 32)
		wr.right -= cr.Width() - 32;
	else if (cr.Width() >= 16)
		wr.right -= cr.Width() - 16;
	if (cr.Height() >= 32)
		cr.bottom -= cr.Height() - 32;
	else if (cr.Height() >= 16)
		cr.bottom -= cr.Height() - 16;
	
	pParent->ScreenToClient(&wr);
	MoveWindow(&wr);
			
	// Set icon
	SetIconID(nIDIcon);

	return true;
}

void CIconWnd::SetIconID(UINT nID)
{
	if (mIconID != nID)
	{
		mIconID = nID;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE);
	}
}

void CIconWnd::OnPaint()
{
	// Fill the middle
	CPaintDC dc(this);
	CRect client;
	GetClientRect(client);

	CIconLoader::DrawIcon(&dc, 0, 0, mIconID, client.Width());
}
