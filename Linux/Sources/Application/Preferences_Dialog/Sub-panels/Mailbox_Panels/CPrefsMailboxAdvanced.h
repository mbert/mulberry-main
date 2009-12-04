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


// Header for CPrefsMailboxAdvanced class

#ifndef __CPREFSMAILBOXADVANCED__MULBERRY__
#define __CPREFSMAILBOXADVANCED__MULBERRY__

#include "CTabPanel.h"

// Classes
class JXIntegerInput;
class JXRadioGroup;
class JXSecondaryRadioGroup;
class JXTextCheckbox;
template<class T> class CInputField;

class CPrefsMailboxAdvanced : public CTabPanel
{
public:
	CPrefsMailboxAdvanced(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    JXSecondaryRadioGroup*       mNewGroup;
    JXTextCheckbox*              mFavouriteCopyTo;
    JXTextCheckbox*              mFavouriteAppendTo;
    CInputField<JXIntegerInput>* mMaximumMRU;
    CInputField<JXIntegerInput>* mRLoCache;
    JXRadioGroup*                mRCacheGroup;
    CInputField<JXIntegerInput>* mRHiCache;
    JXRadioGroup*                mRIncrementGroup;
    CInputField<JXIntegerInput>* mRCacheIncrement;
    CInputField<JXIntegerInput>* mRSortCache;
    CInputField<JXIntegerInput>* mLLoCache;
    JXRadioGroup*                mLCacheGroup;
    CInputField<JXIntegerInput>* mLHiCache;
    JXRadioGroup*                mLIncrementGroup;
    CInputField<JXIntegerInput>* mLCacheIncrement;
    CInputField<JXIntegerInput>* mLSortCache;

// end JXLayout1

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnForceMailboxListReset();
			void OnRCacheGroup();
			void OnRIncrementGroup();
			void OnLCacheGroup();
			void OnLIncrementGroup();
};

#endif
