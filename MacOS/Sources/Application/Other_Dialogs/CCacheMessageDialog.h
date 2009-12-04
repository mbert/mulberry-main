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


// Header for CCacheMessageDialog class

#ifndef __CCACHEMESSAGEDIALOG__MULBERRY__
#define __CCACHEMESSAGEDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_CacheMessageDialog = 1001;
const	PaneIDT		paneid_CacheMessageNumber = 'GOTN';

// Mesages

// Resources
const	ResIDT		RidL_CCacheMessageDialogBtns = 1001;

class	CTextFieldX;

class	CCacheMessageDialog : public LDialogBox
{
private:
	CTextFieldX*	mNumber;

public:
	enum { class_ID = 'CaMa' };

					CCacheMessageDialog();
					CCacheMessageDialog(LStream *inStream);
	virtual 		~CCacheMessageDialog();

	static bool PoseDialog(unsigned long& goto_num);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	GetDetails(unsigned long& goto_num);		// Get the dialogs return info

};

#endif
