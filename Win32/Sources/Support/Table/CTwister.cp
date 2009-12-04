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


// CTwister.cpp : implementation file
//


#include "CTwister.h"

#include "CDrawUtils.h"
#include "CMulberryApp.h"

IMPLEMENT_DYNAMIC(CTwister, CIconButton)

BEGIN_MESSAGE_MAP(CTwister, CIconButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTwister

CTwister::CTwister()
{
	mSize = 16;
}

CTwister::~CTwister()
{
}

BOOL CTwister::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CIconButton::Create(_T(""), rect, pParentWnd, nID, 0, IDI_TWISTDOWN, IDI_TWISTDOWNPUSHED, IDI_TWISTUP, IDI_TWISTUPPUSHED, false);
}
