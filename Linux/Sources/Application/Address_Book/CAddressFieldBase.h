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


// Header for CAddressFieldBase class

#ifndef __CADDRESSFIELD__MULBERRY__
#define __CADDRESSFIELD__MULBERRY__

#include <JXWidgetSet.h>

#include "HPopupMenu.h"
#include "cdstring.h"

// Constants

// Classes
class CStaticText;
class CToolbarButton;

class CAddressFieldBase : public JXWidgetSet
{
	friend class CAddressFieldContainer;
	friend class CAddressFieldSubContainer;

protected:
	CToolbarButton*		mAdd;
	CToolbarButton*		mRemove;
    CStaticText*        mTitle;
    HPopupMenu*      	mType;
    JXWidget*           mDataMove;
    
    bool                mUsesType;
    int                 mOriginalType;
    cdstring            mOriginalData;

public:
					CAddressFieldBase(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressFieldBase();

	void			SetSingleInstance();
	void			SetMultipleInstance();
	void			SetBottom();
	void			HideType();
	void			HideTitle();
    
    virtual void    SetDetails(const cdstring& title, int type, const cdstring& data) = 0;
    virtual bool    GetDetails(int& newtype, cdstring& newdata) = 0;
    void            SetMenu(bool typePopup);

protected:
	virtual void	OnCreate();					// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	void	OnAdd();
	void	OnRemove();
};

#endif
