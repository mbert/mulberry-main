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

#include <LDialogBox.h>

#include "CMessageFwd.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_IMAPLabelsDialog = 5113;
const	PaneIDT		paneid_IMAPLabelsNames[] = 
{
	'NAM1', 'NAM2', 'NAM3', 'NAM4', 'NAM5', 'NAM6', 'NAM7', 'NAM8'
};
const	PaneIDT		paneid_IMAPLabelsLabels[] = 
{
	'LBL1', 'LBL2', 'LBL3', 'LBL4', 'LBL5', 'LBL6', 'LBL7', 'LBL8'
};

// Resources

class CPreferences;
class CTextFieldX;
class CStaticText;

class CIMAPLabelsDialog : public LDialogBox
{
public:
	enum { class_ID = 'Labl' };

					CIMAPLabelsDialog();
					CIMAPLabelsDialog(LStream *inStream);
	virtual 		~CIMAPLabelsDialog();

	static bool		PoseDialog(const cdstrvect& names, cdstrvect& labels);


protected:
	CStaticText*	mNames[NMessage::eMaxLabels];
	CTextFieldX*	mLabels[NMessage::eMaxLabels];

	virtual void	FinishCreateSelf(void);

			void	SetDetails(const cdstrvect& names, const cdstrvect& labels);
			bool	GetDetails(cdstrvect& labels);
			bool	ValidLabel(const cdstring& label);
};

#endif
