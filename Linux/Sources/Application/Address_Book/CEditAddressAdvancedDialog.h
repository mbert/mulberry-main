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


// CEditAddressAdvancedDialog.h : header file
//

#ifndef __CEditAddressAdvancedDialog__MULBERRY__
#define __CEditAddressAdvancedDialog__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CEditAddressAdvancedDialog dialog

class CAdbkAddress;
class CAddressFieldContainer;
class CBetterScrollbarSet;
class CBlankScrollable;
class JXTextButton;

class CEditAddressAdvancedDialog : public CDialogDirector
{
// Construction
public:
	CEditAddressAdvancedDialog(JXDirector* supervisor);

	static bool PoseDialog(CAdbkAddress* addr, bool allow_edit = true);

protected:
// begin JXLayout

    JXTextButton*           mCancelBtn;
    JXTextButton*           mOKBtn;
    CBetterScrollbarSet*    mScroller;
    CBlankScrollable*       mScrollPane;
    CAddressFieldContainer* mFields;

// end JXLayout

	virtual void OnCreate();

			bool GetFields(CAdbkAddress* addr);
			void SetFields(CAdbkAddress* addr, bool allow_edit);
};

#endif
