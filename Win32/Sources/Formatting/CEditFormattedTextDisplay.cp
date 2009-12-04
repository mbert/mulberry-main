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


#include "Mulberry.h"

#include "CEnrichedToolbar.h"
#include "CLetterTextEditView.h"
#include "CCmdEditView.h"
#include "CEditFormattedTextDisplay.h"

IMPLEMENT_DYNCREATE(CEditFormattedTextDisplay, CCmdEditView)


CEditFormattedTextDisplay::CEditFormattedTextDisplay(){
	//SetMargins(CRect(1080, 1080, 1080, 1080));
	
	mFormatter = new CDisplayFormatter(&mTextDisplay);
	mEnrichedToolbar.SetFormatter(mFormatter);
}

CEditFormattedTextDisplay::~CEditFormattedTextDisplay(){
	delete mFormatter;
}



BEGIN_MESSAGE_MAP(CEditFormattedTextDisplay, CContainerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


int CEditFormattedTextDisplay::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Create header pane
	mEnrichedToolbar.CreateEx(0, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 500, 30, GetSafeHwnd(), (HMENU)IDC_LETTERHEADER);
	mEnrichedToolbar.ShowWindow(SW_HIDE);
	mEnrichedShowing = false;
	
	mTextDisplay.CreateEx(0, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 30, 500, 1080, GetSafeHwnd(), (HMENU)IDC_LETTERHEADER);

}