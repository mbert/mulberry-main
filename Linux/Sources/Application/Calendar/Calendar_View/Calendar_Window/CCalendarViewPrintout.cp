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

#include "CCalendarViewPrintout.h"

// ---------------------------------------------------------------------------
//	CCalendarViewPrintout														  [public]
/**
	Default constructor */

CCalendarViewPrintout::CCalendarViewPrintout(LStream *inStream) :
	LPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CCalendarViewPrintout														  [public]
/**
	Destructor */

CCalendarViewPrintout::~CCalendarViewPrintout()
{
}

#pragma mark -

void CCalendarViewPrintout::FinishCreateSelf()
{
	// Get the UI objects
	mPage = dynamic_cast<LView*>(FindPaneByID(ePage_ID));
}

void CCalendarViewPrintout::AdjustToPaperSize()
{
	// Do inherited
	LPrintout::AdjustToPaperSize();
	
	// Set the page panel to the page area, which is smaller than the paper area
	Rect	paperRect;
	mPrintSpec->GetPaperRect(paperRect);
	Rect	pageRect;
	mPrintSpec->GetPageRect(pageRect);
	
	mPage->ResizeFrameTo( (SInt16) (pageRect.right - pageRect.left),
				   (SInt16) (pageRect.bottom - pageRect.top), Refresh_No );

	mPage->ResizeImageTo( pageRect.right - pageRect.left,
				   pageRect.bottom - pageRect.top, Refresh_No );

	mPage->PlaceInSuperImageAt(-paperRect.left, -paperRect.top, Refresh_No);
	
}
