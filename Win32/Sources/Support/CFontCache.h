/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CFontCache : class to handle common fonts

#ifndef __CFONTCACHE__MULBERRY__
#define __CFONTCACHE__MULBERRY__

class CPreferences;

class CFontCache
{
public:
	static CFont*	GetListFont();
	static CFont*	GetDisplayFont();
	static CFont*	GetPrintFont();
	static CFont*	GetCaptionFont();
	static CFont*	GetFixedFont();
	static CFont*	GetHTMLFont();

	static CFont*	GetListFontBold();
	static CFont*	GetListFontItalic();
	static CFont*	GetListFontBoldItalic();
	static CFont*	GetListFontUnderline();
	static CFont*	GetListFontBoldUnderline();
	static CFont*	GetListFontItalicUnderline();
	static CFont*	GetListFontBoldItalicUnderline();

	static CFont*	GetListFontScaled(unsigned long size);

	static void		ResetFonts(CPreferences* prefs);

private:
	static CFont*	sListFont;
	static CFont*	sDisplayFont;
	static CFont*	sPrintFont;
	static CFont*	sCaptionFont;
	static CFont*	sFixedFont;
	static CFont*	sHTMLFont;
	
	static CFont*	sListFontBold;
	static CFont*	sListFontItalic;
	static CFont*	sListFontBoldItalic;
	static CFont*	sListFontUnderline;
	static CFont*	sListFontBoldUnderline;
	static CFont*	sListFontItalicUnderline;
	static CFont*	sListFontBoldItalicUnderline;
	
	typedef std::map<unsigned long, CFont*>	CFontSizeMap;

	static CFontSizeMap		sListFontScaled;

	CFontCache() {}
	~CFontCache() {}
};

#endif
