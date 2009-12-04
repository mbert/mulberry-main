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


// Header for CFontManager classes

#ifndef __CFONTAMANAGER__MULBERRY__
#define __CFONTAMANAGER__MULBERRY__

#include "CCharSets.h"
#include "CFontMapper.h"

// Classes
class CPreferences;

class CFontManager
{
	typedef map<ECharset, cdstrvect> CFontMap;

public:
	static CFontManager sFontManager;
	
	// Initialise a preference set
	void InitPreferences(CPreferences& prefs);

	// Font lists
	const cdstrvect& GetFontList(ECharset charset);		// Get list of font names
	
	// Default mappings
	CFontMapper& GetDefaultFontMapper()					// Get the default font mapper
		{ return mDefaultFontMap; }

private:
	CFontMap	mMap;
	CFontMapper mDefaultFontMap;
	CFontMapper mListFontMap;
	CFontMapper mDisplayFontMap;
	CFontMapper mPrintFontMap;
	CFontMapper mCaptionFontMap;
	CFontMapper mFixedFontMap;
	CFontMapper mHTMLFontMap;

	// Only one static created
	CFontManager() {}
	~CFontManager();
	
	void LookupAllFonts();								// Get list of all fonts
	void LookupFontList(ECharset charset);				// Get list of fonts
	
	void PrepareDefaultFontMapper(ECharset charset);	// Create a set of default font mappings
};

#endif
