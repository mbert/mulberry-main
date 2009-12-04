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


// Header for CCancelDialog class

#ifndef __CCANCELDIALOG__MULBERRY__
#define __CCANCELDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_CancelDialog = 10005;
const	PaneIDT		paneid_CancelSecs = 'SECS';
const	PaneIDT		paneid_CancelDesc = 'DESC';
const	PaneIDT		paneid_CancelProgress = 'PROG';

// Resources
const	PaneIDT		RidL_CCancelDialogBtns = 10005;

class LProgressBar;
class CStaticText;

class CCancelDialog : public LDialogBox
{
public:
	enum { class_ID = 'CDlg' };

					CCancelDialog();
					CCancelDialog(LStream *inStream);
	virtual 		~CCancelDialog();

			void	SetBusyDescriptor(const cdstring& desc);
			void	SetTime(unsigned long secs);

protected:
	CStaticText*	mTime;
	CStaticText*	mDesc;
	LProgressBar*	mBar;
	unsigned long	mLastSecs;

	virtual void	FinishCreateSelf();
};

#endif
