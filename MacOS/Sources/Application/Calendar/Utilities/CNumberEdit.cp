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

#include <LControlImp.h>

// ---------------------------------------------------------------------------
//	CNumberEdit														  [public]
/**
	Default constructor */

CNumberEdit::CNumberEdit(LStream *inStream) :
	CTextFieldX(inStream)
{
	mMaxChars = 0;
	mArrows = NULL;
}


// ---------------------------------------------------------------------------
//	~CNumberEdit														  [public]
/**
	Destructor */

CNumberEdit::~CNumberEdit()
{
}

#pragma mark -

void CNumberEdit::FinishCreateSelf()
{
	CTextFieldX::FinishCreateSelf();

	// Always make it right justified
	//SetAlignment(teJustRight);
}

// Respond to clicks in the icon buttons
void CNumberEdit::ListenToMessage(MessageT inMessage,void *ioParam)
{
	if (inMessage == mArrows->GetValueMessage())
	{
		SetNumberValue(mArrows->GetValue());
	}
}

void CNumberEdit::SetArrows(LLittleArrows* arrows, SInt32 min_value, SInt32 max_value, UInt32 width)
{
	if (arrows)
	{
		mArrows = arrows;
		mArrows->AddListener(this);
		
		mMaxChars = width;

		// Set to minimum value initially
		SetNumberValue(min_value);
		mArrows->SetValue(min_value);
	}
}

SInt32 CNumberEdit::GetNumberValue() const
{
	return GetNumber();
}

void CNumberEdit::SetNumberValue(SInt32 value)
{
	cdstring temp(value);

	// Handle minimum field width
	if ((mMaxChars > 0) && (temp.length() < mMaxChars))
	{
		// Create prefix to fill width
		cdstring filler("0");
		for(int32_t i = temp.length() + 1; i < mMaxChars; i++)
			filler += "0";
		temp = filler + temp;
	}

	SetText(temp, false);
}
