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


// Header for CPrefsDisplayLabel class

#ifndef __CPREFSDISPLAYLABEL__MULBERRY__
#define __CPREFSDISPLAYLABEL__MULBERRY__

#include "CPrefsDisplayPanel.h"

#include "CMessageFwd.h"

#include "cdstring.h"

// Classes
class CTextInputField;
class JXColorButton;
class JXTextButton;
class JXTextCheckbox;

class CPrefsDisplayLabel : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayLabel(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
/* Not used directly
// begin JXLayout1

    CTextInputField* mLabels[0].mName;
    JXTextCheckbox*  mLabels[0].mUseColor;
    JXColorButton*   mLabels[0].mColor;
    JXTextCheckbox*  mLabels[0].mUseBkgColor;
    JXColorButton*   mLabels[0].mBkgColor;
    JXTextCheckbox*  mLabels[0].mBold;
    JXTextCheckbox*  mLabels[0].mItalic;
    JXTextCheckbox*  mLabels[0].mStrike;
    JXTextCheckbox*  mLabels[0].mUnderline;
    CTextInputField* mLabels[1].mName;
    JXTextCheckbox*  mLabels[1].mUseColor;
    JXColorButton*   mLabels[1].mColor;
    JXTextCheckbox*  mLabels[1].mUseBkgColor;
    JXColorButton*   mLabels[1].mBkgColor;
    JXTextCheckbox*  mLabels[1].mBold;
    JXTextCheckbox*  mLabels[1].mItalic;
    JXTextCheckbox*  mLabels[1].mStrike;
    JXTextCheckbox*  mLabels[1].mUnderline;
    CTextInputField* mLabels[2].mName;
    JXTextCheckbox*  mLabels[2].mUseColor;
    JXColorButton*   mLabels[2].mColor;
    JXTextCheckbox*  mLabels[2].mUseBkgColor;
    JXColorButton*   mLabels[2].mBkgColor;
    JXTextCheckbox*  mLabels[2].mBold;
    JXTextCheckbox*  mLabels[2].mItalic;
    JXTextCheckbox*  mLabels[2].mStrike;
    JXTextCheckbox*  mLabels[2].mUnderline;
    CTextInputField* mLabels[3].mName;
    JXTextCheckbox*  mLabels[3].mUseColor;
    JXColorButton*   mLabels[3].mColor;
    JXTextCheckbox*  mLabels[3].mUseBkgColor;
    JXColorButton*   mLabels[3].mBkgColor;
    JXTextCheckbox*  mLabels[3].mBold;
    JXTextCheckbox*  mLabels[3].mItalic;
    JXTextCheckbox*  mLabels[3].mStrike;
    JXTextCheckbox*  mLabels[3].mUnderline;
    CTextInputField* mLabels[4].mName;
    JXTextCheckbox*  mLabels[4].mUseColor;
    JXColorButton*   mLabels[4].mColor;
    JXTextCheckbox*  mLabels[4].mUseBkgColor;
    JXColorButton*   mLabels[4].mBkgColor;
    JXTextCheckbox*  mLabels[4].mBold;
    JXTextCheckbox*  mLabels[4].mItalic;
    JXTextCheckbox*  mLabels[4].mStrike;
    JXTextCheckbox*  mLabels[4].mUnderline;
    CTextInputField* mLabels[5].mName;
    JXTextCheckbox*  mLabels[5].mUseColor;
    JXColorButton*   mLabels[5].mColor;
    JXTextCheckbox*  mLabels[5].mUseBkgColor;
    JXColorButton*   mLabels[5].mBkgColor;
    JXTextCheckbox*  mLabels[5].mBold;
    JXTextCheckbox*  mLabels[5].mItalic;
    JXTextCheckbox*  mLabels[5].mStrike;
    JXTextCheckbox*  mLabels[5].mUnderline;
    CTextInputField* mLabels[6].mName;
    JXTextCheckbox*  mLabels[6].mUseColor;
    JXColorButton*   mLabels[6].mColor;
    JXTextCheckbox*  mLabels[6].mUseBkgColor;
    JXColorButton*   mLabels[6].mBkgColor;
    JXTextCheckbox*  mLabels[6].mBold;
    JXTextCheckbox*  mLabels[6].mItalic;
    JXTextCheckbox*  mLabels[6].mStrike;
    JXTextCheckbox*  mLabels[6].mUnderline;
    CTextInputField* mLabels[7].mName;
    JXTextCheckbox*  mLabels[7].mUseColor;
    JXColorButton*   mLabels[7].mColor;
    JXTextCheckbox*  mLabels[7].mUseBkgColor;
    JXColorButton*   mLabels[7].mBkgColor;
    JXTextCheckbox*  mLabels[7].mBold;
    JXTextCheckbox*  mLabels[7].mItalic;
    JXTextCheckbox*  mLabels[7].mStrike;
    JXTextCheckbox*  mLabels[7].mUnderline;

// end JXLayout1
*/
// begin JXLayout2

    JXTextButton* mIMAPLabelsBtn;

// end JXLayout2

	SFullStyleItems2	mLabels[NMessage::eMaxLabels];
	cdstrvect			mIMAPLabels;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnUseColor(JIndex nID);
			void OnUseBkgnd(JIndex nID);
			void OnIMAPLabels();
};

#endif
