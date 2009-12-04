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


// Header for CAboutDialog class

#ifndef __CABOUTDIALOG__MULBERRY__
#define __CABOUTDIALOG__MULBERRY__

#include <LDialogBox.h>

// Panes
const	ClassIDT	class_AboutDialog ='Abou';
const	PaneIDT		paneid_AboutDialog = 10000;
const	PaneIDT		paneid_VersionNumber = 'VERS';

// Messages

// Resources
const	ResIDT		RidL_CAboutDialogBtns = 10000;

// Classes
class LPushButton;

class CAboutDialog : public LDialogBox
{
public:
	enum { class_ID = class_AboutDialog };

					CAboutDialog();
					CAboutDialog(LStream *inStream);
	virtual 		~CAboutDialog();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

};

#endif
