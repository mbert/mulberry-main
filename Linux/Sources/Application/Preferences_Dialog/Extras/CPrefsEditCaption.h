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


// Header for CPrefsEditCaption class

#ifndef __CPREFSEDITCAPTION__MULBERRY__
#define __CPREFSEDITCAPTION__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"
// Classes

class CPreferences;
template <class T> class CPreferenceValueMap;
class CTextInputDisplay;
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;

class CPrefsEditCaption : public CDialogDirector
{
public:
	CPrefsEditCaption(JXDirector* supervisor);

	static void PoseDialog(CPreferences* prefs,
							CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary);

protected:
// begin JXLayout

    JXTextButton*      mBtn1;
    JXTextButton*      mBtn2;
    JXTextButton*      mBtn3;
    JXTextButton*      mBtn4;
    JXTextButton*      mBtn5;
    JXTextButton*      mBtn6;
    JXTextButton*      mBtn7;
    JXTextButton*      mBtn8;
    JXTextButton*      mBtn9;
    JXTextButton*      mBtn10;
    JXTextButton*      mBtn11;
    JXTextButton*      mBtn12;
    JXTextButton*      mBtn13;
    JXTextButton*      mBtn14;
    JXTextButton*      mBtn15;
    JXTextButton*      mBtn16;
    JXTextButton*      mBtn17;
    JXTextButton*      mBtn18;
    JXTextButton*      mBtn19;
    JXTextButton*      mBtn20;
    JXTextButton*      mBtn21;
    JXTextButton*      mBtn22;
    JXTextButton*      mBtn23;
    JXTextButton*      mBtn24;
    JXTextButton*      mBtn25;
    JXTextCheckbox*    mSummary;
    JXTextButton*      mOkBtn;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mRevertBtn;
    JXRadioGroup*      mMessageDraft1;
    JXTextCheckbox*    mCursorTop;
    JXTextCheckbox*    mUseBox1;
    CTextInputDisplay* mEditHeader;
    JXRadioGroup*      mMessageDraft2;
    JXTextCheckbox*    mUseBox2;
    CTextInputDisplay* mEditFooter;

// end JXLayout

	virtual void OnCreate();								// Do odds & ends
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	SetData(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary);		// Set text in editor
			void	GetEditorText(void);								// Get text from editor

private:
	bool showing1;
	bool showing3;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	cdstring text4;
	
	CPreferenceValueMap<cdstring>*		mText1Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText2Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText3Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText4Item;			// Prefs field
	CPreferenceValueMap<bool>*			mCursorTopItem;		// Cursor top item
	CPreferenceValueMap<bool>*			mBox1Item;			// Box state
	CPreferenceValueMap<bool>*			mBox2Item;			// Box state
	CPreferenceValueMap<bool>*			mSummaryItem;		// Summary state

	void ResizeToWrap(CPreferences* prefs);					// Resize Window to wrap length

	void SetDisplay1(bool show1);
	void UpdateDisplay1();
	void SetDisplay2(bool show2);
	void UpdateDisplay2();

	void AddCaptionItem(const char* item);

	void OnCaptionMessage1();
	void OnCaptionDraft1();
	void OnCaptionMessage2();
	void OnCaptionDraft2();
};

#endif
