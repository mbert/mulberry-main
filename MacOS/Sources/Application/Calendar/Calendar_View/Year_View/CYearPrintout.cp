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

// ===========================================================================
//	CYearPrintout.cp
// ===========================================================================

#include "CYearPrintout.h"

#include "CPrintText.h"

// ---------------------------------------------------------------------------
//	CYearPrintout														  [public]
/**
	Default constructor */

CYearPrintout::CYearPrintout(LStream *inStream) :
	CCalendarViewPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CYearPrintout														  [public]
/**
	Destructor */

CYearPrintout::~CYearPrintout()
{
}

#pragma mark -

void CYearPrintout::FinishCreateSelf()
{
	// Do inherited
	CCalendarViewPrintout::FinishCreateSelf();

	// Get the UI objects
	mYear = dynamic_cast<CPrintText*>(FindPaneByID(eYear_ID));
	
	mTable = dynamic_cast<CYearTable*>(FindPaneByID(eTable_ID));
}

void CYearPrintout::SetDetails(int32_t year, NCalendarView::EYearLayout layout)
{
	// Set static text
	char buf[256];
	std::snprintf(buf, 256, "%ld", year);
	mYear->SetText(buf);
	
	// Reset table
	mTable->ResetTable(year, layout);

	// Resize keeping aspect ratio the same, but fill the page
	SDimension16 frame;
	GetFrameSize(frame);
	SDimension16 tframe;
	mTable->GetFrameSize(tframe);
	SPoint32 torigin;
	mTable->GetFrameLocation(torigin);
	
	// Scale the smallest amount to fit on the page
	float scale_width = frame.width / tframe.width;
	float scale_height = (frame.height - torigin.v) / tframe.height;
	float scale = std::min(scale_height, scale_width);
	SInt32 old_width = tframe.width;
	tframe.width *= scale;
	tframe.height *= scale;
	
	// Center horizontally
	torigin.h = (old_width - tframe.width) / 2.0;

	mTable->ResizeFrameTo(tframe.width, tframe.height, false);
	mTable->MoveBy(torigin.h, 0, false);
}
