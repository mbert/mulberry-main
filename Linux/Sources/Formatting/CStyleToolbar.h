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


#ifndef __CSTYLETOOLBAR__MULBERRY__
#define __CSTYLETOOLBAR__MULBERRY__

#include <JXWidgetSet.h>

#include "CMIMETypes.h"

#include "CFontPopupMenu.h"

#include "cdstring.h"

class CColorPopup;
class CEditFormattedTextDisplay;
class JXMultiImageCheckbox;
class JXWindowDirector;

const int sStyleToolbarHeight = 26;

class CStyleToolbar : public JXWidgetSet
{
	friend class CEditFormattedTextDisplay;

public:
	CStyleToolbar(JXContainer* enclosure,
			  const HSizingOption hSizing, 
			  const VSizingOption vSizing,
			  const JCoordinate x, const JCoordinate y,
			  const JCoordinate w, const JCoordinate h);

	virtual	~CStyleToolbar();

	void OnCreate(JXWindowDirector* director);

	bool HasCommand(unsigned long cmd) const;

	void SetHiliteState(unsigned long cmd, bool state);
	void Ambiguate(unsigned long cmd);

	virtual void AddListener(CEditFormattedTextDisplay* listener);

	void GetFont(cdstring& name);
	JSize GetSize();

	void SetFontItem(JIndex menuItem);
	void SetFontName(const cdstring& fontName);

	void SetSizeItem(JIndex menuItem);
	void SetSize(JSize size);

	void SelectColor(JIndex color);
	void SetColor(const RGBColor& color);
	RGBColor GetColor();

	void Setup(EContentSubType type);
	
protected:
// begin JXLayout1

    JXMultiImageCheckbox* mBold;
    JXMultiImageCheckbox* mItalic;
    JXMultiImageCheckbox* mUnderline;
    JXMultiImageCheckbox* mAlignLeft;
    JXMultiImageCheckbox* mAlignCenter;
    JXMultiImageCheckbox* mAlignRight;
    CFontNamePopup*       mFont;
    CFontSizePopup*       mSize;
    CColorPopup*          mColor;

// end JXLayout1
	bool				  mInited;
	EContentSubType		  mType;
};

#endif
