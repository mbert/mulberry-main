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


// Source for CFontPopup class


#include "CFontMenu.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"

IMPLEMENT_DYNCREATE(CFontPopup, CPopupButton)

BEGIN_MESSAGE_MAP(CFontPopup, CPopupButton)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

// Statics
CMenu CFontPopup::sMenu;
CFontList CFontPopup::sFontList;
int CFontPopup::fontNumber = IDM_FONTStart;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFontPopup::CFontPopup()
{
	mMainMenu = true;
	mAlwaysEnable = true;
}

// Default destructor
CFontPopup::~CFontPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CFontPopup::SetValue(char *string)
{
	int FontID = IDM_FONTStart;
	bool done = false;
	
	while(!done && (FontID < fontNumber))
	{
		cdstring nom = CUnicodeUtils::GetMenuStringUTF8(&sMenu, FontID, MF_BYCOMMAND);
		if(::strcmp(nom, string) == 0)
		{
			done = true;
			SetValue(FontID);
		}
		else
		{
			FontID++;
		}
	}
}

void CFontPopup::SetValue(int value)
{
	if (mValue != value)
	{
		for(int i = IDM_FONTStart; i <= fontNumber; i++)
			sMenu.CheckMenuItem(i, ((i == value) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
	}
	CPopupButton::SetValue(value);
}

cdstring CFontPopup::GetFont(UINT nID)
{
	return CUnicodeUtils::GetMenuStringUTF8(&sMenu, nID, MF_BYCOMMAND);
}


void CFontPopup::OnUpdateFont(CCmdUI* pCmdUI, bool enable)
{
	pCmdUI->Enable(enable);
	pCmdUI->SetCheck((pCmdUI->m_nID == mValue) && enable);
}
	

void CFontPopup::InitFontList()
{
	// Only if empty
	if (!sFontList.size())
	{
		HDC hDC= ::GetDC(NULL);
		EnumFontFamilies(hDC, nil, (FONTENUMPROC) CFontPopup::GetFonts, (LPARAM)1);
	}
}

void CFontPopup::LoadFontMenu()
{
#ifdef __MULBERRY
	sMenu.LoadMenu(IDR_POPUP_FONTFONT);
#endif
	
	sMenu.RemoveMenu(0, MF_BYPOSITION);
	
	InitFontList();

	int pos = 0;
	for(CFontList::iterator iter = sFontList.begin(); iter != sFontList.end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(&sMenu, MF_STRING | ::AppendMenuFlags(pos, false), (UINT) fontNumber++, (*iter)->GetString());
}

int CALLBACK CFontPopup::GetFonts(ENUMLOGFONT FAR *lpelf, NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam){
	
	CFontElement* newElem = new CFontElement(lpelf);
	for(CFontList::iterator iter = sFontList.begin(); iter != sFontList.end(); iter++)
	{
		if (::strcmp(newElem->GetString(), (*iter)->GetString()) < 0)
		{
			sFontList.insert(iter, newElem);
			return fontNumber;
		}
	}
	sFontList.push_back(newElem);
	
	return fontNumber;
}


CMenu* CFontPopup::GetPopupMenu(void)
{
	return &sMenu;
}

bool CFontPopup::GetInfo(const char *string, BYTE *charset, BYTE *pitchAndFamily){

	for(CFontList::iterator iter = sFontList.begin(); iter != sFontList.end(); iter++)
	{
		if (::strcmp((*iter)->GetString(), string) == 0)
		{
			*charset = (*iter)->CharSet;
			*pitchAndFamily = (*iter)->PitchAndFamily;
			return true;
		}
	}
	
	return false;
}

// Look for named font
bool CFontPopup::FontExists(const char* font_name)
{
	// Iterate over all font elements looking for name
	for(CFontList::iterator iter = sFontList.begin(); iter != sFontList.end(); iter++)
	{
		if (::strcmp((*iter)->GetString(), font_name) == 0)
			return true;
	}
	
	return false;
}


#pragma mark ____________________________

CFontElement::CFontElement(ENUMLOGFONT *lpelf)
{
	CharSet = lpelf->elfLogFont.lfCharSet;
	PitchAndFamily = lpelf->elfLogFont.lfPitchAndFamily;
	FaceName = lpelf->elfLogFont.lfFaceName;
}
