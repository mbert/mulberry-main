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


// Header for CPrefsDisplayServer class

#ifndef __CPREFSDISPLAYSERVER__MULBERRY__
#define __CPREFSDISPLAYSERVER__MULBERRY__

#include "CPrefsDisplayPanel.h"


// Classes
class JXColorButton;
class JXTextCheckbox;

class CPrefsDisplayServer : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayServer(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data


protected:
// begin JXLayout1

    JXColorButton*  mOpenColour;
    JXTextCheckbox* mOpenBold;
    JXTextCheckbox* mOpenItalic;
    JXTextCheckbox* mOpenStrike;
    JXTextCheckbox* mOpenUnderline;
    JXColorButton*  mClosedColour;
    JXTextCheckbox* mClosedBold;
    JXTextCheckbox* mClosedItalic;
    JXTextCheckbox* mClosedStrike;
    JXTextCheckbox* mClosedUnder;
    JXColorButton*  mServerColour;
    JXTextCheckbox* mUseServerBtn;
    JXColorButton*  mFavouriteColour;
    JXTextCheckbox* mUseFavouriteBtn;
    JXColorButton*  mHierarchyColour;
    JXTextCheckbox* mUseHierarchyBtn;

// end JXLayout1
	SFullStyleItems		mOpen;
	SFullStyleItems		mClosed;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnUseServer();
			void OnUseFavourite();
			void OnUseHierarchy();

};

#endif
