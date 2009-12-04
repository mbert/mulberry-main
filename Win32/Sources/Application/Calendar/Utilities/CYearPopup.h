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

#ifndef H_CYearPopup
#define H_CYearPopup
#pragma once

#include "CPopupButton.h"

// ===========================================================================
//	CYearPopup

class	CYearPopup : public CPopupButton
{
public:
						CYearPopup();
	virtual				~CYearPopup();

	virtual BOOL SubclassDlgItem(UINT nID, CWnd* pParent);

	void		SetYear(int32_t year);
	int32_t		GetYear() const;

	virtual	void	SetValue(UINT inValue);
	
protected:
	int32_t		mYearStart;

	void	Reset();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

};

#endif
