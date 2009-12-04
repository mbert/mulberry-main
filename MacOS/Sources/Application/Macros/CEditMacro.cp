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


// Source for CEditMacro class

#include "CEditMacro.h"


#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextDisplay.h"
#include "CTextEngine.h"
#include "CTextFieldX.h"
#include "CRFC822.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S E D I T H E A D F O O T
// __________________________________________________________________________________________________

// Default constructor
CEditMacro::CEditMacro()
{
}

// Constructor from stream
CEditMacro::CEditMacro(LStream *inStream) :
				LDialogBox(inStream)
{
}

// Default destructor
CEditMacro::~CEditMacro()
{
}

// Get details of sub-panes
void CEditMacro::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

}

// Set text in editor
void CEditMacro::SetData(const cdstring& name, const cdstring& text)
{
	// Put name into edit field
	CTextFieldX* theName = (CTextFieldX*) FindPaneByID(paneid_EditMacroName);
	theName->SetText(name);

	// Put text into editor
	CTextDisplay* theText = (CTextDisplay*) FindPaneByID(paneid_EditMacroText);
	theText->SetText(text);

	SetLatentSub(theName);
	SwitchTarget(theName);
}

// Get text from editor
void CEditMacro::GetData(cdstring& name, cdstring& text)
{
	// Put name into edit field
	CTextFieldX* theName = (CTextFieldX*) FindPaneByID(paneid_EditMacroName);
	name = theName->GetText();

	// Copy info from panel into prefs
	CTextDisplay* theText = (CTextDisplay*) FindPaneByID(paneid_EditMacroText);
	theText->GetText(text);
}

// Set current wrap length
void CEditMacro::SetRuler()
{
	unsigned long spaces = CPreferences::sPrefs->spaces_per_tab.GetValue();
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();

	// Make wrap safe
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	CTextDisplay* theText = (CTextDisplay*) FindPaneByID(paneid_EditMacroText);
	theText->SetSpacesPerTab(spaces);

	// Resize to new wrap length
	short change_by = 6*(wrap - 80);
	Rect minmax;
	GetMinMaxSize(minmax);
	minmax.left += change_by;
	minmax.right += change_by;
	SetMinMaxSize(minmax);
	minmax = mUserBounds;
	minmax.right += change_by;
	DoSetBounds(minmax);

	// Get ruler
	CStaticText* ruler = (CStaticText*) FindPaneByID(paneid_EditMacroRuler);

	// Create ruler text
	char ruler_txt[256];

	// Clip wrap to fit in 256
	if (wrap > 120) wrap = 120;

	// Top line of ruler
	for(unsigned long i = 0; i<wrap; i++)
		ruler_txt[i] = '-';
	ruler_txt[wrap] = '\r';

	// Do top line text
	ruler_txt[0] = '<';
	ruler_txt[wrap - 1] = '>';

	cdstring wrapper = wrap;
	wrapper += " characters";
	
	if (wrapper.length() + 4 < wrap)
	{
		unsigned long start = (wrap - wrapper.length())/2;
		::memcpy(&ruler_txt[start], wrapper.c_str(), wrapper.length());
	}
	

	// Bottom line of ruler
	for(short i = 0; i < wrap; i++)
	{
		if (i % spaces)
			ruler_txt[wrap + i + 1] = '\'';
		else
			ruler_txt[wrap + i + 1] = '|';
	}
	ruler_txt[2*wrap+1] = '\0';

	// Set ruler text
	ruler->SetText(ruler_txt);
}

bool CEditMacro::PoseDialog(cdstring& name, cdstring& macro)
{
	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_EditMacroDialog, CMulberryApp::sApp);
		CEditMacro* dlog = (CEditMacro*) theHandler.GetDialog();
		dlog->SetRuler();
		dlog->SetData(name, macro);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				dlog->GetData(name, macro);
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
