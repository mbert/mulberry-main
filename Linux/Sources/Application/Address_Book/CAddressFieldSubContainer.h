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


// Header for CAddressFieldSubContainer class

#ifndef __CADDRESSFIELDSUBCONTAINER__MULBERRY__
#define __CADDRESSFIELDSUBCONTAINER__MULBERRY__

#include <JXWidgetSet.h>

#include <vector>

// Constants

// Classes
class CAdbkAddress;
class CAddressFieldBase;
class CStaticText;

class CAddressFieldSubContainer : public JXWidgetSet
{
	friend class CAddressFieldContainer;
	friend class CAddressFieldBase;

public:

    enum EContainerType
    {
        eTel = 0,
        eEmail,
        eAddress,
        eIM,
        eCalAddress,
        eURI
    };

					CAddressFieldSubContainer(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressFieldSubContainer();

	unsigned long	GetCount() const
		{ return mFields.size(); }

	void	SetContainerType(EContainerType ctype)
		{ mType = ctype; }
    
	void	SetAddress(const CAdbkAddress* addr);
	bool	GetAddress(CAdbkAddress* addr);

protected:
	// begin JXLayout1

		CStaticText*    mTitle;

	// end JXLayout1
	EContainerType	mType;
    bool            mDirty;

	virtual void	OnCreate(void);

	CAddressFieldBase*	AddField(bool multi=false);
	void                AddView(JXWidget* view);
	void                RemoveField(JXWidget* view);
    void                AppendField();
    
    void                Layout();

private:
	std::vector<JXWidget*>	mFields;
};

#endif
