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


// Header for CAddressPreview class

#ifndef __CADDRESSPREVIEW__MULBERRY__
#define __CADDRESSPREVIEW__MULBERRY__

#include "CAddressPreviewBase.h"
#include "CAddressPanelBase.h"

// Classes
class CAdbkAddress;
class CStaticText;
class CTabController;
class C3PaneWindow;
class CToolbarButton;

class CAddressPreview : public CAddressPreviewBase
{
public:
					CAddressPreview(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressPreview();

			void	OnCreate();

	virtual void	Close();
	virtual void	Focus();

	void			SetAddress(CAdbkAddress* addr);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const
		{ return mAddress; }

protected:
// begin JXLayout1

    CStaticText*    mDescriptor;
    CToolbarButton* mZoomBtn;
    CTabController* mTabs;

// end JXLayout1
	CAdbkAddress*		mAddress;
	C3PaneWindow*		m3PaneWindow;
	CAddressPanelList	mPanels;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	InitTabs();

			void	OnZoom();
};

#endif
