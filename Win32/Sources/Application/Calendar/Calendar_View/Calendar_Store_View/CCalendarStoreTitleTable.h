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

#ifndef H_CCALENDARSTORETITLETABLE
#define H_CCALENDARSTORETITLETABLE
#pragma once

#include "CTitleTableView.h"

// ===========================================================================
//	CalendarStoreTitleTable

class	CCalendarStoreTitleTable : public CTitleTableView
{
public:
						CCalendarStoreTitleTable();
	virtual				~CCalendarStoreTitleTable();
	
	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);				// Get text for current tooltip cell

	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

protected:
	virtual void	DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect);

	// message handlers

	DECLARE_MESSAGE_MAP()
};

#endif
