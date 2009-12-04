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

#ifndef H_CCalendarPopup
#define H_CCalendarPopup
#pragma once

#include "CPopupButton.h"

#include "CICalendar.h"

// ===========================================================================
//	CCalendarPopup

class	CCalendarPopup : public CPopupButton
{
public:
						CCalendarPopup();
	virtual				~CCalendarPopup();

	virtual BOOL SubclassDlgItem(UINT nID, CWnd* pParent);

	void		SetCalendar(const iCal::CICalendarRef& calref);
	void		GetCalendar(iCal::CICalendarRef& calref) const;

	void		Reset();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

};

#endif
