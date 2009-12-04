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

#include "HTextField.h"

#include "CScrollbarSet.h"

#include <JXButton.h>
#include <JXColormap.h>
#include <JXDialogDirector.h>
#include <JXMenu.h>
#include <JXWindow.h>

HTextField::HTextField
(const JCharacter *text,
 JXContainer* enclosure,
 JXTextMenu* menu,
 const HSizingOption hSizing, const VSizingOption vSizing,
 const JCoordinate x, const JCoordinate y,
 const JCoordinate w, const JCoordinate h,
 Type editorType)
  : super(editorType, kFalse, kFalse,
		sbs = 
		new CScrollbarSet(enclosure,
				   hSizing, vSizing, x, y, w, h),
		sbs->GetScrollEnclosure(),
		hSizing, vSizing,
		0, 0, w, h)
{
	HTextFieldX(text, menu, w, h);
}

HTextField::HTextField
(JXContainer* enclosure,
 const HSizingOption hSizing, const VSizingOption vSizing,
 const JCoordinate x, const JCoordinate y,
 const JCoordinate w, const JCoordinate h,
 Type editorType)
  : super(editorType, kFalse, kFalse,
		sbs = 
		new CScrollbarSet(enclosure,
				   hSizing, vSizing, x, y, w, h),
		sbs->GetScrollEnclosure(),
		hSizing, vSizing,
		0, 0, w, h)
{
	HTextFieldX("", NULL, w, h);
}

void HTextField::HTextFieldX(const JCharacter* text, 
														 JXTextMenu* menu,
														 JCoordinate w, JCoordinate h)
{
	SetDefaultFontSize(kJXDefaultFontSize);
	TESetLeftMarginWidth(kMinLeftMarginWidth);
	SetText(text);

	ShouldAllowDragAndDrop(kTrue);

  SetBackgroundColor(GetColormap()->GetWhiteColor());
  if (menu) {
    ShareEditMenu(menu, kFalse, kFalse);
  }

	bool doSet = false;
	if (w == 0)
		{
		w = GetMinBoundsWidth() + 2*GetBorderWidth();
		doSet = true;
		}
	if (h == 0)
		{
		h = GetMinBoundsHeight() + 2*GetBorderWidth();
		doSet = true;
		}
	if (doSet) {
		SetSize(w,h);
	}
}

void HTextField::Show()
{
	JXTEBase::Show();
	sbs->Show();
}

void HTextField::Hide()
{
	JXTEBase::Hide();
	sbs->Hide();
}

void HTextInputField::HandleFocusEvent()
{
	// Do inherited
	HTextField::HandleFocusEvent();

	// Now see whether we are in a dialog
	JXWindow* wnd = GetWindow();
	JXDialogDirector* dir = dynamic_cast<JXDialogDirector*>(wnd->GetDirector());
	if (dir)
		wnd->ClearShortcuts(dir->GetOKButton());
}

void HTextInputField::HandleUnfocusEvent()
{
	// Do inherited
	HTextField::HandleUnfocusEvent();

	// Now see whether we are in a dialog
	JXWindow* wnd = GetWindow();
	JXDialogDirector* dir = dynamic_cast<JXDialogDirector*>(wnd->GetDirector());
	if (dir)
		wnd->InstallShortcuts(dir->GetOKButton(), "^M");
}
