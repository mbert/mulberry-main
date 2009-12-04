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

#ifndef H_CNUMBEREDIT
#define H_CNUMBEREDIT
#pragma once

#include "CTextFieldX.h"
#include <LListener.h>

#include <LLittleArrows.h>

#include <SysCFString.h>

// ===========================================================================
//	CNumberEdit

class	CNumberEdit : public CTextFieldX,
						public LListener
{
public:
	enum { class_ID = 'Nedt' };

						CNumberEdit(LStream *inStream);
	virtual				~CNumberEdit();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	void		SetArrows(LLittleArrows* arrows, SInt32 min_value, SInt32 max_value, UInt32 width);

	SInt32		GetNumberValue() const;
	void		SetNumberValue(SInt32 value);
	
protected:
	LLittleArrows*	mArrows;
	SInt32			mMaxChars;

	virtual void		FinishCreateSelf();
};

#endif
