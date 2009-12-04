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

#ifndef __CGETSTRINGDIALOG__MULBERRY__
#define __CGETSTRINGDIALOG__MULBERRY__

#include "cdstring.h"
#include "CDialogDirector.h"

class cdstring;
class CStaticText;
class CTextInputField;
class JXTextButton;

class CGetStringDialog : public CDialogDirector
{
public:
	CGetStringDialog(JXDirector* supervisor);
	static bool	PoseDialog(int title, int prompt, cdstring& change);
	static bool PoseDialog(const char* window, const char* title, cdstring& name);
	static bool PoseDialog(const char* title, cdstring& name);

protected:
	void OnCreate();

// begin JXLayout

    CStaticText*     mPromptText;
    CTextInputField* mStringInput;
    JXTextButton*    mOkBtn;
    JXTextButton*    mCancelBtn;

// end JXLayout
private:
	static bool _PoseDialog(const char* window, const char* title, cdstring& name);

};
#endif
