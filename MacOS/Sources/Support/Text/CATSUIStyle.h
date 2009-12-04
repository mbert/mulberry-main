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

#ifndef _H_CATSUIStyle
#define _H_CATSUIStyle
#pragma once

#include <UATSUI.h>

// ---------------------------------------------------------------------------

class CATSUIStyle : public LATSUIStyle
{
public:
	CATSUIStyle() {}
	CATSUIStyle(const CATSUIStyle& copy) :
		LATSUIStyle(copy) {}
	virtual ~CATSUIStyle()  {}
	
	bool GetOneAttribute(
			ATSUAttributeTag		inTag,
			ByteCount				inDataSize,
			ATSUAttributeValuePtr	inValuePtr) const;
							
	bool IsBoldface() const;
	bool IsItalic() const;
	bool IsUnderline() const;
	
	ATSUFontID GetFontID() const;
	short GetFontNum() const;
	void GetFont(Str255 font) const;
	Fixed GetFontSize() const;
	RGBColor GetColor() const;
	
	Style	GetStyle() const;
	void	SetStyle(Style styles, bool add = true);

	void	SetSpelling(bool isSpelling);
	bool	IsSpelling() const;
};

#endif
