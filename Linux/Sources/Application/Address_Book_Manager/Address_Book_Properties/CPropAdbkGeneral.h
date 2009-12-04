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


// Header for CPropAdbkGeneral class

#ifndef __CPROPADBKGENERAL__MULBERRY__
#define __CPROPADBKGENERAL__MULBERRY__

#include "CAdbkPropPanel.h"


// Classes
class CAddressBook;
class CStaticText;
class JXImageWidget;
class JXTextCheckbox;

class CPropAdbkGeneral : public CAdbkPropPanel
{
public:
		CPropAdbkGeneral(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CAdbkPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetAdbkList(CAddressBookList* adbk_list);		// Set adbk list
	virtual void	ApplyChanges(void);							// Force update of mboxes

protected:
// begin JXLayout1

    JXImageWidget*  mIconState;
    CStaticText*    mName;
    CStaticText*    mServer;
    CStaticText*    mTotal;
    CStaticText*    mSingle;
    CStaticText*    mGroup;
    JXTextCheckbox* mOpen;
    JXTextCheckbox* mNickName;
    JXTextCheckbox* mSearch;
    JXTextCheckbox* mAutoSync;

// end JXLayout1

	virtual void	Receive(JBroadcaster* sender, const Message& message);

private:
	virtual void	SetAddressBook(CAddressBook* adbk);			// Set adbk item

	virtual void	OnCheckOpenAtStart(bool set);
	virtual void	OnCheckNickName(bool set);
	virtual void	OnCheckSearch(bool set);
	virtual void	OnCheckAutoSync(bool set);
};

#endif
