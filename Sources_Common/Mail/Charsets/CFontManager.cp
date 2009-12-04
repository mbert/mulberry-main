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


// Source for CFontManager

#include "CFontManager.h"

#include "CPreferences.h"

CFontManager CFontManager::sFontManager;

CFontManager::~CFontManager()
{
}

// Initialise a set of preferences
void CFontManager::InitPreferences(CPreferences& prefs)
{
	// Initialise map if its empty
	if (mMap.empty())
	{
		LookupAllFonts();
	
		// For each charset, set up some defaults
	
		// eUSAscii/eISO8859_1
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mListFontMap.Add(eISO8859_1, "Geneva", 10);
		mDisplayFontMap.Add(eISO8859_1, "Monaco", 9);
		mPrintFontMap.Add(eISO8859_1, "Monaco", 9);
		mCaptionFontMap.Add(eISO8859_1, "Monaco", 9);
		mFixedFontMap.Add(eISO8859_1, "Monaco", 9);
		mHTMLFontMap.Add(eISO8859_1, "Times", 12);
#elif __dest_os == __win32_os
		mListFontMap.Add(eISO8859_1, "MS Sans Serif", 80);
		mDisplayFontMap.Add(eISO8859_1, "Courier New", 80);
		mPrintFontMap.Add(eISO8859_1, "Courier New", 80);
		mCaptionFontMap.Add(eISO8859_1, "Courier New", 80);
		mFixedFontMap.Add(eISO8859_1, "Courier New", 80);
		mHTMLFontMap.Add(eISO8859_1, "Times New Roman", 80);
#else
#error __dest_os
#endif
	
		// Ignore others for now
	}
	
	// Copy into prefs
#ifdef USE_FONTMAPPER
	prefs.mListFontMap.SetValue(mListFontMap);
	prefs.mDisplayFontMap.SetValue(mDisplayFontMap);
	prefs.mPrintFontMap.SetValue(mPrintFontMap);
	prefs.mCaptionFontMap.SetValue(mCaptionFontMap);
	prefs.mFixedFontMap.SetValue(mFixedFontMap);
	prefs.mHTMLFontMap.SetValue(mHTMLFontMap);
#endif
}

// Get list of font names
const cdstrvect& CFontManager::GetFontList(ECharset charset)
{
	// Does list exist
	CFontMap::const_iterator found = mMap.find(charset);
	if (found != mMap.end())
		return (*found).second;
	else
	{
		LookupFontList(charset);
		return GetFontList(charset);
	}
}

// Get list of all fonts
void CFontManager::LookupAllFonts()
{
	// For now iterate over all know charsets and find corresponding fonts
	
	for(int i = eUSAscii; i < eCharsetLast; i++)
		LookupFontList(static_cast<ECharset>(i));
	
	for(int i = eUSAscii; i < eCharsetLast; i++)
		PrepareDefaultFontMapper(static_cast<ECharset>(i));
}

// Get list of fonts
void CFontManager::LookupFontList(ECharset charset)
{
	// Only bother if not present
	if (mMap.count(charset))
		return;
	
	// Look at each font and see if it matches the script
	cdstrvect fonts;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Get appropriate script for charset
	unsigned long script = ::CharsetToScript(charset);
	
	// Get list of fonts
	unsigned long num_rsrc = ::CountResources('FOND');
	for(unsigned long i = 1; i <= num_rsrc; i++)
	{
		// Get font resource handle
		Handle rsrc = ::GetIndResource('FOND', i);
		if (rsrc && (::ResError() == noErr))
		{
			// Get name of font
			short id;
			ResType type;
			Str255 name;
			::GetResInfo(rsrc, &id, &type, name);
			if ((::ResError() == noErr) && *name)
			{
				// See if font matches script and add to list
				short fNum;
				::GetFNum(name, &fNum);
				if (::FontToScript(fNum) == script)
					fonts.push_back(cdstring(name));
			}
			::ReleaseResource(rsrc);
		}
	}
#elif __dest_os == __win32_os
#else
#error __dest_os
#endif

	// Now add results to map
	mMap.insert(CFontMap::value_type(charset, fonts));
}

// Create a set of default font mappings
void CFontManager::PrepareDefaultFontMapper(ECharset charset)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Lookup script manager default for font and size of charset's script
	unsigned long scriptNum = ::CharsetToScript(charset);
	long fondSize = ::GetScriptVariable(scriptNum, smScriptAppFondSize);
	if (!fondSize)
		fondSize = ::GetScriptVariable(smSystemScript, smScriptAppFondSize);
	TextStyle ts;
    ts.tsFont = HiWord(fondSize);
	ts.tsSize = LoWord(fondSize);

	Str255 name;
	::GetFontName(ts.tsFont, name);
	cdstring cname(name);

	mDefaultFontMap.Add(charset, cname, ts.tsSize);
#elif __dest_os == __win32_os
	// Enumerate matching fonts and pick the best one
#else
#error __dest_os
#endif
}