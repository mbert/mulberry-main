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


// CMessagePartProp.h : header file
//

#ifndef __CMESSAGEPARTPROP__MULBERRY__
#define __CMESSAGEPARTPROP__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp dialog

class CAttachment;

class CStaticText;
class CTextDisplay;
class JXTextButton;

class CMessagePartProp : public CDialogDirector
{
// Construction
public:
	CMessagePartProp(JXDirector* supervisor);

	static bool PoseDialog(const CAttachment& attach, bool multi);

protected:
// begin JXLayout

    JXTextButton* mOKBtn;
    JXTextButton* mCancelBtn;
    CStaticText*  mName;
    CStaticText*  mType;
    CStaticText*  mEncoding;
    CStaticText*  mID;
    CStaticText*  mDisposition;
    CTextDisplay* mDescription;
    CTextDisplay* mParams;
    CStaticText*  mOpenWith;

// end JXLayout

	virtual void OnCreate();
			void SetFields(const CAttachment& attach, bool multi);	
};

#endif
