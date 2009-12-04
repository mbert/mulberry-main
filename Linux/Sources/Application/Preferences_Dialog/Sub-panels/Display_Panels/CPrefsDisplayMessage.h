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


// Header for CPrefsDisplayMessage class

#ifndef __CPREFSDISPLAYMESSAGE__MULBERRY__
#define __CPREFSDISPLAYMESSAGE__MULBERRY__

#include "CPrefsDisplayPanel.h"

// Classes
class JXColorButton;
class JXTextCheckbox;

class CPrefsDisplayMessage : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayMessage(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    JXColorButton*  mUnseenColour;
    JXTextCheckbox* mUnseenBold;
    JXTextCheckbox* mUnseenItalic;
    JXTextCheckbox* mUnseenStrike;
    JXTextCheckbox* mUnseenUnderline;
    JXColorButton*  mSeenColour;
    JXTextCheckbox* mSeenBold;
    JXTextCheckbox* mSeenItalic;
    JXTextCheckbox* mSeenStrike;
    JXTextCheckbox* mSeenUnderline;
    JXColorButton*  mAnsweredColour;
    JXTextCheckbox* mAnsweredBold;
    JXTextCheckbox* mAnsweredItalic;
    JXTextCheckbox* mAnsweredStrike;
    JXTextCheckbox* mAnsweredUnderline;
    JXColorButton*  mImportantColour;
    JXTextCheckbox* mImportantBold;
    JXTextCheckbox* mImportantItalic;
    JXTextCheckbox* mImportantStrike;
    JXTextCheckbox* mImportantUnderline;
    JXColorButton*  mDeletedColour;
    JXTextCheckbox* mDeletedBold;
    JXTextCheckbox* mDeletedItalic;
    JXTextCheckbox* mDeletedStrike;
    JXTextCheckbox* mDeletedUnderline;
    JXTextCheckbox* mMultiAddressBold;
    JXTextCheckbox* mMultiAddressItalic;
    JXTextCheckbox* mMultiAddressStrike;
    JXTextCheckbox* mMultiAddressUnderline;
    JXColorButton*  mMatchColour;
    JXTextCheckbox* mUseMatchBtn;
    JXColorButton*  mNonMatchColour;
    JXTextCheckbox* mUseNonMatchBtn;
    JXTextCheckbox* mUseLocalTimezone;

// end JXLayout1

	SFullStyleItems		mUnseen;
	SFullStyleItems		mSeen;
	SFullStyleItems		mAnswered;
	SFullStyleItems		mImportant;
	SFullStyleItems		mDeleted;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnUseMatch();
			void OnUseNonMatch();
};

#endif
