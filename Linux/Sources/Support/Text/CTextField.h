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


// Header for CTextField class

#ifndef __CTEXTFIELD__MULBERRY__
#define __CTEXTFIELD__MULBERRY__

#include "CTextBase.h"

// Classes
class CTextField : public CTextBase
{
public:
	CTextField(const JCharacter *text, JXContainer* enclosure,
				JXTextMenu *menu,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h,
				Type editorType = kSelectableText)
	: CTextBase(text, NULL, enclosure, menu, hSizing, vSizing, x, y, w, h, editorType)
	{ CTextFieldX(); }

	CTextField(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h,
				Type editorType = kSelectableText)
	: CTextBase(NULL, enclosure, hSizing, vSizing, x, y, w, h, editorType)
	{ CTextFieldX(); }

	virtual void	OnCreate();								// Setup context

protected:
	virtual void	HandleFocusEvent();
	virtual void	HandleUnfocusEvent();

private:
			void	CTextFieldX();
};


// Version that is a full text editor - handy when doing jxlayout
class CTextInputField : public CTextField
{
public:
	CTextInputField(const JCharacter* text, JXContainer* enclosure,
					JXTextMenu* menu,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CTextField(text, enclosure, menu, hSizing, vSizing, x, y, w, h, kFullEditor) {}

	CTextInputField(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CTextField(enclosure, hSizing, vSizing, x, y, w, h, kFullEditor) {}
};

#endif
