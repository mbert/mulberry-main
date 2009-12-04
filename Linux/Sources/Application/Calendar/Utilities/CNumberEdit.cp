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

#include "CNumberEdit.h"

#include "JXMultiImageButton.h"

#include <jASCIIConstants.h>
#include <jGlobals.h>

#include <stdlib.h>

CNumberEdit::CNumberEdit(const JCharacter* text, JXContainer* enclosure,
								JXTextMenu* menu,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
				: CTextInputField(text, enclosure, menu, hSizing, vSizing, x, y, w - 20, h)
{
	mMin = 0;
	mMax = 100;
	mFiller = 0;
}

CNumberEdit::CNumberEdit(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
				: CTextInputField(enclosure, hSizing, vSizing, x, y, w - 20, h)
{
	mMin = 0;
	mMax = 100;
	mFiller = 0;
}

#pragma mark -

// Get details of sub-panes
void CNumberEdit::OnCreate(long min, long max, unsigned long filler)
{
	JFontStyle style;
	SetFont(JGetDefaultFontName(), kJDefaultFontSize, style);
	SetBreakCROnly(true);

	WantInput(kTrue);

	JRect frame = GetFrame();

	// Create spin up/down buttons
    mSpinUp =
        new JXMultiImageButton(const_cast<JXContainer*>(GetEnclosure()),
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, frame.right, frame.top, 20, frame.height() / 2);
	mSpinUp->SetImage(IDI_SPIN_UP);

    mSpinDown =
        new JXMultiImageButton(const_cast<JXContainer*>(GetEnclosure()),
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, frame.right, frame.top + frame.height() / 2, 20, frame.height() / 2);
	mSpinDown->SetImage(IDI_SPIN_DOWN);

	ListenTo(mSpinUp);
	ListenTo(mSpinDown);
	
	SetText("0");
	SetRange(min, max, filler);
}

// Handle OK button
void CNumberEdit::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mSpinUp)
			Nudge(true);
		else if (sender == mSpinDown)
			Nudge(false);
	}
}

void CNumberEdit::SetRange(long min, long max, unsigned long filler)
{
	mMin = min;
	mMax = max;
	mFiller = filler;

	// Clip current value to range
	ValidNumber();
}

long CNumberEdit::GetNumberValue() const
{
	cdstring temp = GetText();
	return ::atoi(temp);
}

void CNumberEdit::SetNumberValue(long value)
{
	cdstring temp(value);

	// Handle minimum field width
	if ((mFiller > 0) && (temp.length() < mFiller))
	{
		// Create prefix to fill width
		cdstring filler("0");
		for(unsigned long i = temp.length() + 1; i < mFiller; i++)
			filler += "0";
		temp = filler + temp;
	}

	SetText(temp);
}
bool CNumberEdit::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	return CTextInputField::ObeyCommand(cmd, menu);
}

void CNumberEdit::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	CTextInputField::UpdateCommand(cmd, cmdui);
}

bool CNumberEdit::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	const JBoolean controlOn = modifiers.control();
	const JBoolean metaOn    = modifiers.meta();
	const JBoolean shiftOn   = modifiers.shift();
	
	// Some mods must be off
	if (controlOn || metaOn)
		return true;
		
	switch(key)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case kJDeleteKey:
	case kJForwardDeleteKey:
	{
		bool result = CTextInputField::HandleChar(key, modifiers);
		ValidNumber();
		return result;
	}
	case kJUpArrow:
		Nudge(true);
		return true;
	case kJDownArrow:
		Nudge(false);
		return true;
	default:
		return true;
	}
	
	return true;
}

void CNumberEdit::Show()
{
	CTextInputField::Show();
	mSpinUp->Show();
	mSpinDown->Show();
}

void CNumberEdit::Hide()
{
	CTextInputField::Hide();
	mSpinUp->Hide();
	mSpinDown->Hide();
}

void CNumberEdit::Activate()
{
	CTextInputField::Activate();
	mSpinUp->SetActive(true);
	mSpinDown->SetActive(true);
}

void CNumberEdit::Deactivate()
{
	CTextInputField::Deactivate();
	mSpinUp->SetActive(false);
	mSpinDown->SetActive(false);
}

void CNumberEdit::Nudge(bool up)
{
	long current = GetNumberValue();
	if (up && (current < mMax))
	{
		current++;
		SetNumberValue(current);
		SetSel(0, GetTextLength());
	}
	else if (!up && (current > mMin))
	{
		current--;
		SetNumberValue(current);
		SetSel(0, GetTextLength());
	}
}

void CNumberEdit::ValidNumber()
{
	long current = GetNumberValue();
	if (current < mMin)
		SetNumberValue(mMin);
	else if (current > mMax)
		SetNumberValue(mMax);
}
