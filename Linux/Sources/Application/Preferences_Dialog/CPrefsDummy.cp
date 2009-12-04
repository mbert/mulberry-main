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


// CPrefsDummy.cpp : implementation file
//


#include "CPrefsDummy.h"

#include <JXStaticText.h>
#include <JXColormap.h>

/////////////////////////////////////////////////////////////////////////////
// CPrefsDummy property page


CPrefsDummy::CPrefsDummy( JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

void CPrefsDummy::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Not implemented yet", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 250,30);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);
// end JXLayout1

	CPrefsPanel::OnCreate();
}

// Set up params for DDX
void CPrefsDummy::SetPrefs(CPreferences* prefs)
{
	// Save ref to prefs
	mCopyPrefs = prefs;
}

// Get params from controls
void CPrefsDummy::UpdatePrefs(CPreferences* prefs)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDummy message handlers
