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


// Source for CFontMapper

#include "CFontMapper.h"

#include "CFontManager.h"
#if __dest_os == __win32_os
#include "CFontMenu.h"
#endif
#include "CLog.h"

#include "char_stream.h"

extern const char* cSpace;

#pragma mark ____________________________CFontDescriptor

CFontDescriptor::CFontDescriptor()
{
	Reset(cdstring::null_str, 0);
}

CFontDescriptor::CFontDescriptor(const CFontDescriptor& copy)
{
	mTraits = copy.mTraits;
}

void CFontDescriptor::Reset(const char* name, unsigned long size)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::strncpy(reinterpret_cast<char*>(mTraits.traits.fontName), name, 255);
	mTraits.traits.fontName[255] = 0;
	c2pstr(reinterpret_cast<char*>(mTraits.traits.fontName));
	mTraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	mTraits.traits.size = size;

	mTraits.traits.style = 0;
	mTraits.traits.justification = teFlushDefault;
	mTraits.traits.mode = srcOr;
	mTraits.traits.color.red = 0;
	mTraits.traits.color.green = 0;
	mTraits.traits.color.blue = 0;
#elif __dest_os == __win32_os
#else
#error __dest_os
#endif
}

// Read/write prefs
cdstring CFontDescriptor::GetInfo(void) const
{
	cdstring info;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	info += cdstring(static_cast<unsigned long>(mTraits.traits.size));
	info += cSpace;

	cdstring temp(mTraits.traits.fontName);
	temp.quote();
	info += temp;
#elif __dest_os == __win32_os
	info += cdstring(static_cast<unsigned long>(-mTraits.logfont.lfHeight));
	info += cSpace;

	cdstring temp(mTraits.logfont.lfFaceName);
	temp.quote();
	info += temp;
#else
#error __dest_os
#endif
	return info;
}

bool CFontDescriptor::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	unsigned long size;
	txt.get(size);
	mTraits.traits.size = size;
	cdstring temp;
	txt.get(temp);
	::strncpy(reinterpret_cast<char*>(mTraits.traits.fontName), temp, 255);
	mTraits.traits.fontName[255] = 0;
	c2pstr(reinterpret_cast<char*>(mTraits.traits.fontName));

	// Init others
	mTraits.traits.style = normal;
	mTraits.traits.justification = teFlushLeft;
	mTraits.traits.mode = srcOr;
	mTraits.traits.color.red = 0;
	mTraits.traits.color.green = 0;
	mTraits.traits.color.blue = 0;
	mTraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
#elif __dest_os == __win32_os
	// Count items
	char* p = txt;
	int count = 0;
	while(*p)
	{
		if (*p++ == ',') count++;
	}

	// Check for new style format of prefs
	if (count <= 1)
	{
		LOGFONT temp;
		temp.lfHeight = 0;
		temp.lfWidth = 0;
		temp.lfEscapement = 0;
		temp.lfOrientation = 0;
		temp.lfWeight = 400;
		temp.lfItalic = 0;
		temp.lfUnderline = 0;
		temp.lfStrikeOut = 0;
		temp.lfCharSet = 0;
		temp.lfOutPrecision = 1;
		temp.lfClipPrecision = 2;
		temp.lfQuality = 1;
		temp.lfPitchAndFamily = 0;
		*temp.lfFaceName = 0;

		unsigned long size;
		txt.get(size);
		temp.lfHeight = size;
		cdstring stemp;
		txt.get(stemp);
		::strcpy(temp.lfFaceName, stemp);

#ifdef __MULBERRY
		// Look for font in global list and match characteristics
		if (CFontPopup::GetInfo(temp.lfFaceName, &temp.lfCharSet, &temp.lfPitchAndFamily))
#endif
		{
			temp.lfHeight *= -1;
			mTraits.logfont = temp;
		}
	}
	else
		::sscanf(txt, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %[^,]",
					&mTraits.logfont.lfHeight, &mTraits.logfont.lfWidth, &mTraits.logfont.lfEscapement, &mTraits.logfont.lfOrientation,
					&mTraits.logfont.lfWeight, &mTraits.logfont.lfItalic, &mTraits.logfont.lfUnderline, &mTraits.logfont.lfStrikeOut,
					&mTraits.logfont.lfCharSet, &mTraits.logfont.lfOutPrecision, &mTraits.logfont.lfClipPrecision,
					&mTraits.logfont.lfQuality, &mTraits.logfont.lfPitchAndFamily, &mTraits.logfont.lfFaceName);
#else
#error __dest_os
#endif

	return true;
}

#pragma mark ____________________________CFontMapper

void CFontMapper::Add(ECharset charset, const char* name, unsigned long size)
{
	// Special: map eUSAscii to eISO8859_1
	if (charset == eUSAscii)
		charset = eISO8859_1;

	// Look for matching item
	cdstring charset_name = cCharsets[charset];
	iterator found = find(charset_name);
	
	// Only add if not already present
	if (found == end())
	{
		CFontDescriptor font(name, size);
#if __dest_os == __win32_os
		// Set charset value in LOGFONT
		font.mTraits.logfont.lfCharSet = ::CharsetToScript(charset);
#endif
		insert(value_type(charset_name, font));
	}
}

// Lookup
const CFontDescriptor& CFontMapper::GetCharsetFontDescriptor(ECharset charset) const
{
	return const_cast<CFontMapper*>(this)->GetCharsetFontDescriptor(charset);
}

CFontDescriptor& CFontMapper::GetCharsetFontDescriptor(ECharset charset)
{
	// Special: map eUSAscii to eISO8859_1
	if (charset == eUSAscii)
		charset = eISO8859_1;

	// Look for matching item
	cdstring charset_name = cCharsets[charset];
	iterator found = find(charset_name);
	if (found != end())
		return (*found).second;
	else
	{
		// Get the default one if we're not already it
		if (this != &CFontManager::sFontManager.GetDefaultFontMapper())
			return CFontManager::sFontManager.GetDefaultFontMapper().GetCharsetFontDescriptor(charset);

		// Must be the default one so try ISO-8859-1
		charset_name = cCharsets[eISO8859_1];
		iterator found = find(charset_name);
		if (found != end())
			return (*found).second;
		else
		{
			// This is dire! We really should not get here as ISO-8859-1 ought to be defined somewhere!
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
			return (*found).second;
		}
	}
}

// Read/write prefs
cdstring CFontMapper::GetInfo(void) const
{
	cdstring all;
	
	// Start S-Expression
	all += '(';
	
	// Iterate over all entries
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		// Start S-Expression
		all += '(';
		
		// Add items
		cdstring temp = (*iter).first;
		temp.quote();
		all += temp;
		all += cSpace;
		
		all += (*iter).second.GetInfo();

		// End S-Expression
		all += ')';
	}

	// End S-Expression
	all += ')';
	
	return all;
}

bool CFontMapper::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// Clear existing
	clear();

	// Must have leading '('
	if (!txt.start_sexpression()) return false;

	while(txt.start_sexpression())
	{
		cdstring charset;
		txt.get(charset);
		
		CFontDescriptor font;
		result = font.SetInfo(txt, vers_prefs) && result;

#if __dest_os == __win32_os
		// Set charset value in LOGFONT
		font.mTraits.logfont.lfCharSet = ::CharsetToScript(::GetCharset(charset));
#endif
		insert(value_type(charset, font));

		txt.end_sexpression();
	}

	// Bump past trailing )
	txt.end_sexpression();

	return result;
}
