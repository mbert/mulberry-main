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


// Header for CMessagePartProp class

#ifndef __CMESSAGEPARTPROP__MULBERRY__
#define __CMESSAGEPARTPROP__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_MessagePart = 2011;
const	PaneIDT		paneid_MessagePartCancelAll = 'CANC';
const	PaneIDT		paneid_MessagePartName = 'NAME';
const	PaneIDT		paneid_MessagePartType = 'TYPE';
const	PaneIDT		paneid_MessagePartEncoding = 'ENCD';
const	PaneIDT		paneid_MessagePartID = 'IDID';
const	PaneIDT		paneid_MessagePartDisposition = 'DISP';
const	PaneIDT		paneid_MessagePartDescription = 'DESC';
const	PaneIDT		paneid_MessagePartParameters = 'PARA';
const	PaneIDT		paneid_MessagePartOpenWith = 'OPEN';

// Resources
class	CTextFieldX;
class	LPushButton;
class	CTextDisplay;
class	CAttachment;

class	CMessagePartProp : public LDialogBox {

private:
	LPushButton*		mCancelAll;
	CTextFieldX*		mName;
	CTextFieldX*		mType;
	CTextFieldX*		mEncoding;
	CTextFieldX*		mID;
	CTextFieldX*		mDisposition;
	CTextFieldX*		mDescription;
	CTextDisplay*		mParameters;
	CTextFieldX*		mOpenWith;
	
public:
	enum { class_ID = 'MPrt' };

					CMessagePartProp();
					CMessagePartProp(LStream *inStream);
	virtual 		~CMessagePartProp();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetFields(CAttachment& attach,
									bool multi);			// Set fields in dialog
};

#endif
