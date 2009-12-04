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


// Source for CTextField class

#include "CTextField.h"

#include <JFontStyle.h>
#include <jGlobals.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T F I E L D
// __________________________________________________________________________________________________

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup context
void CTextField::OnCreate()
{
	// Set default context edit menu
	// Derived classes that want their own context menu
	// should set it BEFORE calling this inherited method
	CTextBase::OnCreate(CMainMenu::eContextEdit);
}
	
void CTextField::CTextFieldX()
{
	SetDefaultFontName(JGetDefaultFontName());
	SetDefaultFontSize(kJDefaultFontSize);
}

/******************************************************************************
 HandleFocusEvent (virtual protected)

 ******************************************************************************/

void CTextField::HandleFocusEvent()
{
	JXTEBase16::HandleFocusEvent(); // Bypass removal of ^M shortcut in dialogs
	ClearUndo();
	EditSelectAll();

	// Make it the commander target
	SetTarget(this);
}

/******************************************************************************
 HandleUnfocusEvent (virtual protected)

 ******************************************************************************/

void CTextField::HandleUnfocusEvent()
{
	JXTEBase16::HandleUnfocusEvent(); // Bypass removal of ^M shortcut in dialogs
	ClearUndo();

	// Remove the commander target
	//SetTarget(GetSuperCommander());
}
