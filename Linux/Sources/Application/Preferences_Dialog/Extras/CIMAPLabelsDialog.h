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


// Header for CIMAPLabelsDialog class

#ifndef __CIMAPLabelsDialog__MULBERRY__
#define __CIMAPLabelsDialog__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

#include "CMessageFwd.h"

// Classes

class CTextInputField;
class CStaticText;
class JXTextButton;

class CIMAPLabelsDialog : public CDialogDirector
{
public:
	CIMAPLabelsDialog(JXDirector* supervisor);

	static bool PoseDialog(const cdstrvect& names, cdstrvect& labels);

protected:
// begin JXLayout

    JXTextButton* mCancelBtn;
    JXTextButton* mOkBtn;

// end JXLayout

/*
// begin JXLayout2

    CStaticText*     mName[0];
    CTextInputField* mLabel[0];
    CStaticText*     mName[1];
    CTextInputField* mLabel[1];
    CStaticText*     mName[2];
    CTextInputField* mLabel[2];
    CStaticText*     mName[3];
    CTextInputField* mLabel[3];
    CStaticText*     mName[4];
    CTextInputField* mLabel[4];
    CStaticText*     mName[5];
    CTextInputField* mLabel[5];
    CStaticText*     mName[6];
    CTextInputField* mLabel[6];
    CStaticText*     mName[7];
    CTextInputField* mLabel[7];

// end JXLayout2
*/
    CStaticText*     mName[NMessage::eMaxLabels];
    CTextInputField* mLabel[NMessage::eMaxLabels];

	virtual void	OnCreate();								// Do odds & ends
	virtual JBoolean OKToDeactivate();

			bool	OnOK();

			void	SetDetails(const cdstrvect& names, const cdstrvect& labels);
			bool	GetDetails(cdstrvect& labels);
			bool	ValidLabel(const cdstring& label);

};

#endif
