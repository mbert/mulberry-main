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


// Source for CPrefsDisplayStyles class

#include "CPrefsDisplayStyles.h"

#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayStyles::CPrefsDisplayStyles()
{
}

// Constructor from stream
CPrefsDisplayStyles::CPrefsDisplayStyles(LStream *inStream)
		: CPrefsDisplayPanel(inStream)
{
}

// Default destructor
CPrefsDisplayStyles::~CPrefsDisplayStyles()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayStyles::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsDisplayPanel::FinishCreateSelf();

	// Get controls
	GetControls(mURL, paneid_DSURLColor, paneid_DSURLBold, paneid_DSURLItalic, paneid_DSURLUnderline);
	GetControls(mURLSeen, paneid_DSURLSeenColor, paneid_DSURLSeenBold, paneid_DSURLSeenItalic, paneid_DSURLSeenUnderline);
	GetControls(mHeader, paneid_DSHeaderColor, paneid_DSHeaderBold, paneid_DSHeaderItalic, paneid_DSHeaderUnderline);
	GetControls(mTag, paneid_DSTagColor, paneid_DSTagBold, paneid_DSTagItalic, paneid_DSTagUnderline);

	mURLs = (CTextDisplay*) FindPaneByID(paneid_DSURLs);
}

// Set prefs
void CPrefsDisplayStyles::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mURL, copyPrefs->mURLStyle.GetValue());
	SetStyle(mURLSeen, copyPrefs->mURLSeenStyle.GetValue());
	SetStyle(mHeader, copyPrefs->mHeaderStyle.GetValue());
	SetStyle(mTag, copyPrefs->mTagStyle.GetValue());

	cdstring urls;
	for(cdstrvect::const_iterator iter = copyPrefs->mRecognizeURLs.GetValue().begin(); iter != copyPrefs->mRecognizeURLs.GetValue().end(); iter++)
	{
		urls += *iter;
		urls += os_endl;
	}
	mURLs->SetText(urls);
}

// Force update of prefs
void CPrefsDisplayStyles::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	SStyleTraits temp;
	copyPrefs->mURLStyle.SetValue(GetStyle(mURL, temp));
	copyPrefs->mURLSeenStyle.SetValue(GetStyle(mURLSeen, temp));
	copyPrefs->mHeaderStyle.SetValue(GetStyle(mHeader, temp));
	copyPrefs->mTagStyle.SetValue(GetStyle(mTag, temp));

	// Only copy text if dirty
	if (mURLs->IsDirty())
	{
		// Copy handle to text with null terminator
		cdstring txt;
		mURLs->GetText(txt);

		char* s = ::strtok(txt.c_str_mod(), CR);
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, CR);
		}
		copyPrefs->mRecognizeURLs.SetValue(accumulate);
	}
}
