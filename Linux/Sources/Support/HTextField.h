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

#ifndef _H_HTEXTFIELD
#define _H_HTEXTFIELD

#include <JXTEBase.h>

class JXTextMenu;
class cdstring;

class HTextField : public JXTEBase
{
	typedef JXTEBase super;
public:
	HTextField(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	HTextField(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);

	void HTextFieldX(const JCharacter* text, 
									 JXTextMenu* menu,
									 JCoordinate w,
									 JCoordinate h);

	JXScrollbarSet* GetScroller()
		{ return sbs; }

	virtual void	Show();					// must call inherited
	virtual void	Hide();					// must call inherited

	void SetFontName ( const JCharacter* name ) {
		SelectAll(); SetCurrentFontName(name); SetDefaultFontName(name); }

	void SetFontSize ( const JSize size ) {
		SelectAll(); SetCurrentFontSize(size); SetDefaultFontSize(size); }

	void SetFontStyle ( const JFontStyle& style ) {
		SelectAll(); SetCurrentFontStyle(style); SetDefaultFontStyle(style); }

	void SetFont ( const JCharacter* name, const
								 JSize size, const JFontStyle& style ) { 
		SelectAll();
		SetCurrentFont(name, size, style); SetDefaultFont(name, size, style);
	}

/******************************************************************************
SetBackgroundColor

 ******************************************************************************/

 void SetBackgroundColor ( const JColorIndex color
) { SetBackColor(color); SetFocusColor(color); }

private:
	JXScrollbarSet* sbs;
};

// Version that is a full text editor - handy when doing jxlayout
class HTextInputField : public HTextField
{
public:
	HTextInputField(const JCharacter* text, JXContainer* enclosure,
						 JXTextMenu* menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h)
	: HTextField(text, enclosure, menu, hSizing, vSizing, x, y, w, h, kFullEditor) {}

	HTextInputField(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h)
	: HTextField(enclosure, hSizing, vSizing, x, y, w, h, kFullEditor) {}

protected:
	virtual void		HandleFocusEvent();
	virtual void		HandleUnfocusEvent();
};

#endif
