/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Header for CEditAddressAdvancedDialog class

#ifndef __CEditAddressAdvancedDIALOG__MULBERRY__
#define __CEditAddressAdvancedDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_EditAddressAdvanced = 9003;
const	PaneIDT		paneid_EditAddressAdvancedFields = 'FLDS';

// Resources
const	ResIDT		STRx_EditAddressAdvancedHelp = 9003;

class	CAddressFieldContainer;
class	CAdbkAddress;

class	CEditAddressAdvancedDialog : public LDialogBox {

private:
	CAddressFieldContainer*	mFields;
	
public:
	enum { class_ID = 'EdAv' };

					CEditAddressAdvancedDialog();
					CEditAddressAdvancedDialog(LStream *inStream);
	virtual 		~CEditAddressAdvancedDialog();

	static	bool	PoseDialog(CAdbkAddress* addr, bool allow_edit = true);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetFields(const CAdbkAddress* addr,		// Set fields in dialog
								bool allow_edit = true);
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
};

#endif
