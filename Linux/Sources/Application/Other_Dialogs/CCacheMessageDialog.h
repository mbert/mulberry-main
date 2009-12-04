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


// CCacheMessageDialog.h : header file
//

#ifndef __CCACHEMESSAGEDIALOG__MULBERRY__
#define __CCACHEMESSAGEDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CCacheMessageDialog dialog

template <class T> class CInputField;
class JXTextButton;
class JXIntegerInput;

class CCacheMessageDialog : public CDialogDirector
{
// Construction
public:
	CCacheMessageDialog(JXDirector* supervisor);

	static bool PoseDialog(unsigned long& goto_num);

protected:
	virtual void OnCreate();

// begin JXLayout

    CInputField<JXIntegerInput>* mEnterCtrl;
    JXTextButton*                mOKBtn;
    JXTextButton*                mCancelBtn;

// end JXLayout

};

#endif
