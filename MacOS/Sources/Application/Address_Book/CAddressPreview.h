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

// Constants
const	PaneIDT		paneid_AddressPreview = 1733;
const	PaneIDT		paneid_AddressPreviewDesc = 'DESC';
const	PaneIDT		paneid_AddressPreviewZoom = 'ZOOM';

// Messages
const	MessageT	msg_AddressPreviewZoom = 'ZOOM';

// Classes
class CAdbkAddress;
class CTabController;
class C3PaneWindow;
class CStaticText;

class CAddressPreview : public CAddressPreviewBase, public LListener
{
public:
	enum { class_ID = 'Apre' };

					CAddressPreview();
					CAddressPreview(LStream *inStream);
	virtual 		~CAddressPreview();

	virtual void	Close();
	virtual void	Focus();

	void			SetAddress(CAdbkAddress* addr);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const
		{ return mAddress; }

protected:
	CAdbkAddress*		mAddress;
	C3PaneWindow*		m3PaneWindow;
	CStaticText*		mDescriptor;
	CTabController*		mTabs;
	CAddressPanelList	mPanels;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to clicks in the icon buttons

			void	InitTabs();

			void	OnZoom();
};

#endif
