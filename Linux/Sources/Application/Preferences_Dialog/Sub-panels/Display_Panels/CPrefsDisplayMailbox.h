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


// Header for CPrefsDisplayMailbox class

#ifndef __CPREFSDISPLAYMAILBOX__MULBERRY__
#define __CPREFSDISPLAYMAILBOX__MULBERRY__

#include "CPrefsDisplayPanel.h"


// Classes
class JXColorButton;
class JXTextCheckbox;

class CPrefsDisplayMailbox : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayMailbox(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data


protected:
// begin JXLayout1

    JXColorButton*  mRecentColour;
    JXTextCheckbox* mRecentBold;
    JXTextCheckbox* mRecentItalic;
    JXTextCheckbox* mRecentStrike;
    JXTextCheckbox* mRecentUnderline;
    JXColorButton*  mUnseenColour;
    JXTextCheckbox* mUnseenBold;
    JXTextCheckbox* mUnseenItalic;
    JXTextCheckbox* mUnseenStrike;
    JXTextCheckbox* mUnseenUnderline;
    JXColorButton*  mOpenColour;
    JXTextCheckbox* mOpenBold;
    JXTextCheckbox* mOpenItalic;
    JXTextCheckbox* mOpenStrike;
    JXTextCheckbox* mOpenUnderline;
    JXColorButton*  mFavouriteColour;
    JXTextCheckbox* mFavouriteBold;
    JXTextCheckbox* mFavouriteItalic;
    JXTextCheckbox* mFavouriteStrike;
    JXTextCheckbox* mFavouriteUnderline;
    JXColorButton*  mClosedColour;
    JXTextCheckbox* mClosedBold;
    JXTextCheckbox* mClosedItalic;
    JXTextCheckbox* mClosedStrike;
    JXTextCheckbox* mClosedUnderline;
    JXTextCheckbox* mIgnoreRecent;

// end JXLayout1

	SFullStyleItems		mRecent;
	SFullStyleItems		mUnseen;
	SFullStyleItems		mOpen;
	SFullStyleItems		mFavourite;
	SFullStyleItems		mClosed;
};

#endif
