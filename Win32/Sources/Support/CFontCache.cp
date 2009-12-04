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


// CFontCache : class to handle commonly used fonts in app

#include "CFontCache.h"

#include "CPreferences.h"

CFont* CFontCache::sListFont = NULL;
CFont* CFontCache::sDisplayFont = NULL;
CFont* CFontCache::sPrintFont = NULL;
CFont* CFontCache::sCaptionFont = NULL;
CFont* CFontCache::sFixedFont = NULL;
CFont* CFontCache::sHTMLFont = NULL;

CFont* CFontCache::sListFontBold = NULL;
CFont* CFontCache::sListFontItalic = NULL;
CFont* CFontCache::sListFontBoldItalic = NULL;
CFont* CFontCache::sListFontUnderline = NULL;
CFont* CFontCache::sListFontBoldUnderline = NULL;
CFont* CFontCache::sListFontItalicUnderline = NULL;
CFont* CFontCache::sListFontBoldItalicUnderline = NULL;

CFontCache::CFontSizeMap CFontCache::sListFontScaled;

CFont* CFontCache::GetListFont()
{
	return sListFont;
}

CFont* CFontCache::GetDisplayFont()
{
	return sDisplayFont;
}

CFont* CFontCache::GetPrintFont()
{
	return sPrintFont;
}

CFont* CFontCache::GetCaptionFont()
{
	return sCaptionFont;
}

CFont* CFontCache::GetFixedFont()
{
	return sFixedFont;
}

CFont* CFontCache::GetHTMLFont()
{
	return sHTMLFont;
}

CFont* CFontCache::GetListFontBold(void)
{
	if (!sListFontBold)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfWeight = FW_BOLD;
		sListFontBold = new CFont;
		sListFontBold->CreateFontIndirect(&info);
	}
	
	return sListFontBold;
}

CFont* CFontCache::GetListFontItalic(void)
{
	if (!sListFontItalic)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfItalic = TRUE;
		sListFontItalic = new CFont;
		sListFontItalic->CreateFontIndirect(&info);
	}
	
	return sListFontItalic;
}

CFont* CFontCache::GetListFontBoldItalic(void)
{
	if (!sListFontBoldItalic)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfWeight = FW_BOLD;
		info.lfItalic = TRUE;
		sListFontBoldItalic = new CFont;
		sListFontBoldItalic->CreateFontIndirect(&info);
	}
	
	return sListFontBoldItalic;
}

CFont* CFontCache::GetListFontUnderline(void)
{
	if (!sListFontUnderline)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfUnderline = TRUE;
		sListFontUnderline = new CFont;
		sListFontUnderline->CreateFontIndirect(&info);
	}
	
	return sListFontUnderline;
}

CFont* CFontCache::GetListFontBoldUnderline(void)
{
	if (!sListFontBoldUnderline)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfWeight = FW_BOLD;
		info.lfUnderline = TRUE;
		sListFontBoldUnderline = new CFont;
		sListFontBoldUnderline->CreateFontIndirect(&info);
	}
	
	return sListFontBoldUnderline;
}

CFont* CFontCache::GetListFontItalicUnderline(void)
{
	if (!sListFontItalicUnderline)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfItalic = TRUE;
		info.lfUnderline = TRUE;
		sListFontItalicUnderline = new CFont;
		sListFontItalicUnderline->CreateFontIndirect(&info);
	}
	
	return sListFontItalicUnderline;
}

CFont* CFontCache::GetListFontBoldItalicUnderline(void)
{
	if (!sListFontBoldItalicUnderline)
	{
		LOGFONT info;
		sListFont->GetLogFont(&info);

		info.lfWeight = FW_BOLD;
		info.lfItalic = TRUE;
		info.lfUnderline = TRUE;
		sListFontBoldItalicUnderline = new CFont;
		sListFontBoldItalicUnderline->CreateFontIndirect(&info);
	}
	
	return sListFontBoldItalicUnderline;
}

CFont* CFontCache::GetListFontScaled(unsigned long size)
{
	// Look for existing map entry and use that
	CFontSizeMap::const_iterator found = sListFontScaled.find(size);
	
	if (found != sListFontScaled.end())
	{
		return (*found).second;
	}
	
	// Create new scaled font
	LOGFONT info;
	sListFont->GetLogFont(&info);

	info.lfHeight = size;
	info.lfWidth = 0;
	CFont* temp = new CFont;
	temp->CreateFontIndirect(&info);
	
	sListFontScaled.insert(CFontSizeMap::value_type(size, temp));
	
	return temp;
}

void CFontCache::ResetFonts(CPreferences* prefs)
{
	// Initialise new base fonts
	if (sListFont)
		delete sListFont;
	if (prefs)
	{
		sListFont = new CFont;
		sListFont->CreateFontIndirect(&prefs->mListTextFontInfo.GetValue().logfont);
	}

	if (sDisplayFont)
		delete sDisplayFont;
	if (prefs)
	{
		sDisplayFont = new CFont;
		sDisplayFont->CreateFontIndirect(&prefs->mDisplayTextFontInfo.GetValue().logfont);
	}

	if (sPrintFont)
		delete sPrintFont;
	if (prefs)
	{
		sPrintFont = new CFont;
		sPrintFont->CreateFontIndirect(&prefs->mPrintTextFontInfo.GetValue().logfont);
	}

	if (sCaptionFont)
		delete sCaptionFont;
	if (prefs)
	{
		sCaptionFont = new CFont;
		sCaptionFont->CreateFontIndirect(&prefs->mCaptionTextFontInfo.GetValue().logfont);
	}

	if (sFixedFont)
		delete sFixedFont;
	if (prefs)
	{
		sFixedFont = new CFont;
		sFixedFont->CreateFontIndirect(&prefs->mFixedTextFontInfo.GetValue().logfont);
	}

	if (sHTMLFont)
		delete sHTMLFont;
	if (prefs)
	{
		sHTMLFont = new CFont;
		sHTMLFont->CreateFontIndirect(&prefs->mHTMLTextFontInfo.GetValue().logfont);
	}

	delete sListFontBold;
	sListFontBold = NULL;
	delete sListFontItalic;
	sListFontItalic = NULL;
	delete sListFontBoldItalic;
	sListFontBoldItalic = NULL;
	delete sListFontUnderline;
	sListFontUnderline = NULL;
	delete sListFontBoldUnderline;
	sListFontBoldUnderline = NULL;
	delete sListFontItalicUnderline;
	sListFontItalicUnderline = NULL;
	delete sListFontBoldItalicUnderline;
	sListFontBoldItalicUnderline = NULL;
	
	// Remove scaled list fonts
	for(CFontSizeMap::iterator iter = sListFontScaled.begin(); iter != sListFontScaled.end(); iter++)
	{
		delete (*iter).second;
	}
	sListFontScaled.clear();
}