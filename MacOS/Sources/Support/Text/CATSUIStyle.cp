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

#include "CATSUIStyle.h"

#include "CGUtils.h"
#include "CLog.h"

#include "MoreATSUnicode.h"

// ---------------------------------------------------------------------------
//	¥ GetOneAttribute												  [public]
// ---------------------------------------------------------------------------

bool
CATSUIStyle::GetOneAttribute(
	ATSUAttributeTag		inTag,
	ByteCount				inDataSize,
	ATSUAttributeValuePtr	inValuePtr) const
{
	ByteCount actualDataSize;
	OSStatus err = ::ATSUGetAttribute( mStyle,
										   inTag,
										   inDataSize,
										   inValuePtr,
										   &actualDataSize);
	
	if ((err != noErr) && (err != kATSUNotSetErr))
		ThrowIfOSStatus_(err);

	return err == noErr;
}


bool CATSUIStyle::IsBoldface() const
{
	Boolean value;;
	if (GetOneAttribute(kATSUQDBoldfaceTag, sizeof(Boolean), &value))
		return value;
	else
		return false;
}

bool CATSUIStyle::IsItalic() const
{
	Boolean value;;
	if (GetOneAttribute(kATSUQDItalicTag, sizeof(Boolean), &value))
		return value;
	else
		return false;
}

bool CATSUIStyle::IsUnderline() const
{
	Boolean value;;
	if (GetOneAttribute(kATSUQDUnderlineTag, sizeof(Boolean), &value))
		return value;
	else
		return false;
}

ATSUFontID CATSUIStyle::GetFontID() const
{
	ATSUFontID result = 0;
	if (GetOneAttribute(kATSUFontTag, sizeof(ATSUFontID), &result))
		return result;
	else
		return 0;
}

short CATSUIStyle::GetFontNum() const
{
	short fontNumber;
	Str255 fontName;
	GetFont(fontName);
	::GetFNum(fontName, &fontNumber);

	return fontNumber;
}

void CATSUIStyle::GetFont(Str255 font) const
{
	ATSUFontID fontID = GetFontID();

	OSStatus status = noErr;
	ByteCount		familyNameLength = 0, styleNameLength = 0;
	ItemCount		familyIndex;
	FontPlatformCode	familyPlatform = kFontNoPlatform;
	FontScriptCode		familyScript = kFontNoScript;
	FontLanguageCode	familyLanguage = kFontNoLanguage;
	

	status = ::atsuFindBestFontName(fontID, kFontFamilyName,
									&familyPlatform, &familyScript, &familyLanguage,
									255, (char*)font, &familyNameLength, &familyIndex );
	
	if ((familyNameLength != 0) && (familyNameLength < 255))
	{
		font[familyNameLength] = 0;
		c2pstr((char*)font);
	}
	if ((status != noErr) || (familyPlatform != kFontMacintoshPlatform))
		*font = 0;

}

Fixed CATSUIStyle::GetFontSize() const
{
	Fixed result = 12.0;
	GetOneAttribute(kATSUSizeTag, sizeof(Fixed), &result);
	return result;
}

RGBColor CATSUIStyle::GetColor() const
{
	RGBColor color = { 0, 0, 0 };
	GetOneAttribute(kATSUColorTag, sizeof(RGBColor), (void*) &color);
	return color;
}

Style CATSUIStyle::GetStyle() const
{
	Style result = normal;
	if (IsBoldface())
		result |= bold;
	if (IsItalic())
		result |= italic;
	if (IsUnderline())
		result |= underline;
	
	return result;
}

void CATSUIStyle::SetStyle(Style styles, bool add)
{
	if (styles == normal)
	{
		SetBoldface(false);
		SetItalic(false);
		SetUnderline(false);
	}
	else
	{
		if (styles & bold)
			SetBoldface(add);
		if (styles & italic)
			SetItalic(add);
		if (styles & underline)
			SetUnderline(add);
	}
}

void CATSUIStyle::SetSpelling(bool isSpelling)
{
	ATSUAttributeTag	tags[] = { kATSUStyleUnderlineColorOptionTag,
								   kATSUQDUnderlineTag };
								   
	ByteCount			sizes[] = { sizeof(CGColorRef),
									sizeof(Boolean) };
									
	ATSUAttributeValuePtr	values[2];
	
	Boolean use_underline = isSpelling;

	CGColorSpaceRef colorSpace = isSpelling ? ::CGColorSpaceCreateDeviceRGB() : NULL;
	float components[] = { 1.0, 0.0, 0.0, 1.0 };
	CGColorRef color = isSpelling ? ::CGColorCreate(colorSpace, components) : NULL;
	
	values[0] = &color;
	values[1] = &use_underline;

	try
	{
		SetMultipleAttributes(2, tags, sizes, values);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	if (isSpelling)
	{
		::CGColorRelease(color);
		::CGColorSpaceRelease(colorSpace);
	}
}

bool CATSUIStyle::IsSpelling() const
{
	bool result = false;

	// Look for underline first
	if (IsUnderline())
	{
		CGColorSpaceRef colorSpace = ::CGColorSpaceCreateDeviceRGB();
		float components[] = { 1.0, 0.0, 0.0, 1.0 };
		CGColorRef color = ::CGColorCreate(colorSpace, components);
	
		CGColorRef cresult = NULL;
		if (GetOneAttribute(kATSUStyleUnderlineColorOptionTag, sizeof(CGColorRef), (void*) &cresult))
		{
			if (cresult != NULL)
				result = ::CGColorEqualToColor(cresult, color);
		}

		::CGColorRelease(color);
		::CGColorSpaceRelease(colorSpace);
	}

	return result;
}
