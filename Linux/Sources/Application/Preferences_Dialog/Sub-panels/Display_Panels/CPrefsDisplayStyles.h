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


// Header for CPrefsDisplayStyles class

#ifndef __CPREFSDISPLAYSTYLES__MULBERRY__
#define __CPREFSDISPLAYSTYLES__MULBERRY__

#include "CPrefsDisplayPanel.h"

// Classes
class JXColorButton;
class CTextInputDisplay;
class JXTextCheckbox;

class CPrefsDisplayStyles : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayStyles(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    JXColorButton*     mURLColour;
    JXTextCheckbox*    mURLBold;
    JXTextCheckbox*    mURLItalic;
    JXTextCheckbox*    mURLUnderline;
    JXColorButton*     mURLSeenColour;
    JXTextCheckbox*    mURLSeenBold;
    JXTextCheckbox*    mURLSeenItalic;
    JXTextCheckbox*    mURLSeenUnderline;
    JXColorButton*     mHeaderColour;
    JXTextCheckbox*    mHeaderBold;
    JXTextCheckbox*    mHeaderItalic;
    JXTextCheckbox*    mHeaderUnderline;
    JXColorButton*     mTagColour;
    JXTextCheckbox*    mTagBold;
    JXTextCheckbox*    mTagItalic;
    JXTextCheckbox*    mTagUnderline;
    JXColorButton*     mSelectionColour;
    CTextInputDisplay* mURLs;

// end JXLayout1

	SStyleItems		mURL;
	SStyleItems		mURLSeen;
	SStyleItems		mHeader;
	SStyleItems		mTag;
};

#endif
