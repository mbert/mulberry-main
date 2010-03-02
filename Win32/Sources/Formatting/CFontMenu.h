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


// Header for CFontPopup class

#ifndef __CFONTMENU__MULBERRY__
#define __CFONTMENU__MULBERRY__

#include "CPopupButton.h"

// Classes
class CFontElement;

typedef std::vector<CFontElement*> CFontList;

class CFontPopup : public CPopupButton
{

	DECLARE_DYNCREATE(CFontPopup)

public:
						CFontPopup();
	virtual 			~CFontPopup();

	static CMenu*		GetMainMenu(void)
		{ return &sMenu; }
	virtual CMenu*		GetPopupMenu(void);
	
	static void 		InitFontList();
	static void 		LoadFontMenu();
	static int CALLBACK GetFonts(ENUMLOGFONT *lpelf, NEWTEXTMETRIC *lpntm, int FontType, LPARAM lParam);
	void 				OnUpdateFont(CCmdUI* pCmdUI, bool enable);
	cdstring 			GetFont(UINT nID);
	void				SetValue(char *string);
	void				SetValue(int value);
	static bool 		GetInfo(const char *string, BYTE *charset, BYTE *pitchAndFamily);
	
	static bool			FontExists(const char* font_name);		// Look for named font

private:
	static CMenu		sMenu;
	static CFontList	sFontList;
	static int			fontNumber;

protected:
	DECLARE_MESSAGE_MAP()
};


class CFontElement{
	
public:
						CFontElement(ENUMLOGFONT *lpelf);
	cdstring&				GetString()
							{ return FaceName; }
	BYTE				CharSet;
	BYTE 				PitchAndFamily;
protected:	
	cdstring			FaceName;
};

#endif
