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


// Header for CEditMacro class

#ifndef __CEDITMACRO__MULBERRY__
#define __CEDITMACRO__MULBERRY__

#include "CDialogDirector.h"

// Classes
class cdstring;

class CTextInputDisplay;
class CTextInputField;
class JXStaticText;
class JXTextButton;

class CEditMacro : public CDialogDirector
{

public:
			CEditMacro(JXDirector* supervisor);
	virtual	~CEditMacro();

	static bool		PoseDialog(cdstring& name, cdstring& macro);


protected:
// begin JXLayout

    JXTextButton*      mOkBtn;
    JXTextButton*      mCancelBtn;
    JXStaticText*      mRuler;
    CTextInputDisplay* mText;
    CTextInputField*   mName;

// end JXLayout

	virtual void	OnCreate();								// Do odds & ends

	void	SetData(const cdstring& name, const cdstring& text);		// Set data
	void	GetData(cdstring& name, cdstring& text);					// Get data
	void	SetRuler();													// Set current wrap length

private:
	cdstring*		mTextItem;								// Prefs field
};

#endif
