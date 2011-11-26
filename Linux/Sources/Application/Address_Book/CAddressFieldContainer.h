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


// Header for CAddressFieldContainer class

#ifndef __CADDRESSFIELDCONTAINER__MULBERRY__
#define __CADDRESSFIELDCONTAINER__MULBERRY__

#include <JXWidgetSet.h>

#include <vector>

// Constants

// Classes
class CAdbkAddress;
class CAddressFieldBase;
class CAddressFieldSubContainer;

class CAddressFieldContainer : public JXWidgetSet
{
	friend class CAddressPreviewAdvanced;
	friend class CAddressFieldBase;
    friend class CAddressFieldSubContainer;
    friend class CEditAddressAdvancedDialog;

public:
					CAddressFieldContainer(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressFieldContainer();

	unsigned long	GetCount() const
		{ return mFields.size(); }

	void	SetAddress(const CAdbkAddress* addr);
	bool	GetAddress(CAdbkAddress* addr);

protected:
	virtual void	OnCreate();

	CAddressFieldBase*          AddField(bool multi=false);
	CAddressFieldSubContainer*	AddContainer(int ctype);
	void               			AddView(JXWidget* view);
    
    void                Layout();

private:
	std::vector<JXWidget*>	mFields;
};

#endif
