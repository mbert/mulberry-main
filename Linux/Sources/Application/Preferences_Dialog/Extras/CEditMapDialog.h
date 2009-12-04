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


// CEditMapDialog.h : header file
//

#ifndef __CEDITMAPDIALOG__MULBERRY__
#define __CEDITMAPDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CEditMapDialog dialog

class CMIMEMap;
class JXRadioGroup;
class JXTextButton;
class CTextInputField;

class CEditMapDialog : public CDialogDirector
{
// Construction
public:
	CEditMapDialog(JXDirector* supervisor);

	static bool PoseDialog(CMIMEMap& aMap);

protected:
// begin JXLayout

    JXTextButton*    mOkBtn;
    JXTextButton*    mCancelBtn;
    CTextInputField* mType;
    CTextInputField* mSubtype;
    CTextInputField* mSuffix;
    JXRadioGroup*    mAppLaunch;

// end JXLayout

	virtual void	OnCreate();								// Do odds & ends

			void	SetMap(const CMIMEMap& aMap);
			void	GetMap(CMIMEMap& aMap);
};

#endif
