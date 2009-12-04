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


// Header for CSynchroniseDialog class

#ifndef __CSYNCHRONISEDIALOG__MULBERRY__
#define __CSYNCHRONISEDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "templs.h"

// Constants

class CMboxList;
class CMbox;
class JXIntegerInput;
class JXRadioGroup;
class JXTextRadioButton;
template <class T> class CInputField;

class CSynchroniseDialog : public CDialogDirector
{
public:
	
	CSynchroniseDialog(JXDirector* supervisor);
	
	static  void	PoseDialog(CMboxList* mbox_list);
	static  void	PoseDialog(CMbox* mbox, ulvector& selection);
	static  bool	PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected);

protected:
	enum
	{
		eAll = 0,
		eNew,
		eSelected
	};
	enum
	{
		eFull = 0,
		eBelow,
		ePartial
	};
	
// begin JXLayout

    JXRadioGroup*                mMailboxGroup;
    JXTextRadioButton*           mSelected;
    JXRadioGroup*                mMessageGroup;
    CInputField<JXIntegerInput>* mSize;

// end JXLayout

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void	SetDetails(bool has_selection);								// Set the dialogs info
	virtual void	GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection);		// Get the dialogs return info

};

#endif
