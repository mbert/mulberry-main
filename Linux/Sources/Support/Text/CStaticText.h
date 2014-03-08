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


#ifndef _H_CStaticText
#define _H_CStaticText

#include <JXTEBase16.h>

class cdstring;
class cdustring;

class CStaticText : public JXTEBase16
{
public:

	CStaticText(const JCharacter* text, JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);

	CStaticText(const JCharacter* text,
				 const JBoolean wordWrap, const JBoolean selectable,
				 JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);

	virtual ~CStaticText();

	void	SetFontName(const JCharacter* name);
	void	SetFontSize(const JSize size);
	void	SetFontStyle(const JFontStyle& style);
	void	SetFontAlign(const AlignmentType align);
	void	SetFont(const JCharacter* name, const JSize size,
					const JFontStyle& style = JFontStyle(), const AlignmentType align = kAlignLeft);

	void	SetBackgroundColor(const JColorIndex color);
	void	SetTransparent(bool transparent)
		{ mTransparent = transparent; }

			void	SetText(const cdstring& txt);				// UTF8 in
			void	SetText(const char* txt, size_t size = -1);	// UTF8 in
			void	SetText(const cdustring& txt);				// UTF16 in
			void	SetNumber(long num);						// Number in

			void	GetText(cdstring& txt) const;				// UTF8 out
		cdstring	GetText() const;							// UTF8 out
			void	GetText(cdustring& txt) const;				// UTF16 out

protected:
	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frame);

private:
	bool	mTransparent;

	void	CStaticTextX(const JCharacter* text,
						  const JCoordinate w, const JCoordinate h);

	// not allowed

	CStaticText(const CStaticText& source);
	const CStaticText& operator=(const CStaticText& source);
};


/******************************************************************************
 Change font

 ******************************************************************************/

inline void
CStaticText::SetFontName
	(
	const JCharacter* name
	)
{
	SelectAll();
	SetCurrentFontName(name);
	SetDefaultFontName(name);
}

inline void
CStaticText::SetFontSize
	(
	const JSize size
	)
{
	SelectAll();
	SetCurrentFontSize(size);
	SetDefaultFontSize(size);
}

inline void
CStaticText::SetFontStyle
	(
	const JFontStyle& style
	)
{
	SelectAll();
	SetCurrentFontStyle(style);
	SetDefaultFontStyle(style);
}

inline void
CStaticText::SetFontAlign
	(
	const AlignmentType align
	)
{
	SelectAll();
	SetCurrentFontAlign(align);
	SetDefaultFontAlign(align);
}

inline void
CStaticText::SetFont
	(
	const JCharacter*	name,
	const JSize			size,
	const JFontStyle&	style,
	const AlignmentType align
	)
{
	SelectAll();
	SetCurrentFont(name, size, style, align);
	SetDefaultFont(name, size, style, align);
}

/******************************************************************************
 SetBackgroundColor

 ******************************************************************************/

inline void
CStaticText::SetBackgroundColor
	(
	const JColorIndex color
	)
{
	SetBackColor(color);
	SetFocusColor(color);
}

#endif
