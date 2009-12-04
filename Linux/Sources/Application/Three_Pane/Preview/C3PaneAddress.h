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


// Header for C3PaneAddress class

#ifndef __C3PANEADDRESS__MULBERRY__
#define __C3PANEADDRESS__MULBERRY__

#include "C3PaneViewPanel.h"

// Classes
class CAdbkAddress;
class CAddressBook;
class CAddressView;
class CGroup;

class C3PaneAddress: public C3PaneViewPanel
{
public:
					C3PaneAddress(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~C3PaneAddress();

	virtual void	OnCreate();

	virtual bool	TestClose();

	virtual bool	IsSpecified() const;

	virtual CBaseView*	GetBaseView() const;
	CAddressView*	GetAddressView() const
		{ return mAddressView; }

	void SetAddress(CAddressBook* adbk, CAdbkAddress* addr);
	void SetGroup(CAddressBook* adbk, CGroup* grp);

	virtual cdstring 		GetTitle() const;
	virtual unsigned int	GetIconID() const;

protected:
	CAddressView*	mAddressView;
};

#endif
