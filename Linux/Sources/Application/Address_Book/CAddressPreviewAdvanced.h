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


// Header for CAddressPreviewAdvanced class

#ifndef __CADDRESSPREVIEWADVANCED__MULBERRY__
#define __CADDRESSPREVIEWADVANCED__MULBERRY__

#include "CAddressPreviewBase.h"
#include "CAddressPanelBase.h"

// Classes
class CAdbkAddress;
class CAddressFieldContainer;
class CBetterScrollbarSet;
class CBlankScrollable;
class CStaticText;
class CToolbarButton;
class C3PaneWindow;

class CAddressPreviewAdvanced : public CAddressPreviewBase
{
public:
	CAddressPreviewAdvanced(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressPreviewAdvanced();

			void	OnCreate();

	virtual void	Close();
	virtual void	Focus();

	void			SetAddress(CAdbkAddress* addr);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const
		{ return mAddress; }

protected:
// begin JXLayout1

    CStaticText*            mDescriptor;
    CToolbarButton*         mZoomBtn;
    CBetterScrollbarSet*    mScroller;
    CBlankScrollable*       mScrollPane;
    CAddressFieldContainer* mFields;

// end JXLayout1
	CAdbkAddress*		mAddress;
	C3PaneWindow*		m3PaneWindow;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	InitTabs();

	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog

			void	OnZoom();
};

#endif
