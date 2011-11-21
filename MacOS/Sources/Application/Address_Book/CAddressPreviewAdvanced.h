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


// Header for CAddressPreviewAdvanced class

#ifndef __CADDRESSPREVIEWADVANCED__MULBERRY__
#define __CADDRESSPREVIEWADVANCED__MULBERRY__

#include "CAddressPreviewBase.h"

// Constants
const	PaneIDT		paneid_AddressPreviewAdvanced = 1739;
const	PaneIDT		paneid_AddressPreviewAdvancedDesc = 'DESC';
const	PaneIDT		paneid_AddressPreviewAdvancedZoom = 'ZOOM';
const	PaneIDT		paneid_AddressPreviewAdvancedFields = 'FLDS';

// Messages
const	MessageT	msg_AddressPreviewAdvancedZoom = 'ZOOM';

// Classes
class CAdbkAddress;
class CAddressFieldContainer;
class CTextDisplay;
class C3PaneWindow;
class CStaticText;

class CAddressPreviewAdvanced : public CAddressPreviewBase, public LListener
{
public:
	enum { class_ID = 'AprA' };
    
    CAddressPreviewAdvanced();
    CAddressPreviewAdvanced(LStream *inStream);
	virtual 		~CAddressPreviewAdvanced();
    
	virtual void	Close();
	virtual void	Focus();
    
	void			SetAddress(CAdbkAddress* addr);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const
        { return mAddress; }
    
protected:
	CAdbkAddress*	mAddress;
	CStaticText*	mDescriptor;
	C3PaneWindow*	m3PaneWindow;
	CAddressFieldContainer*	mFields;
    
	virtual void	FinishCreateSelf();					// Do odds & ends
    
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to clicks in the icon buttons
    
	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
    
    void	OnZoom();
};

#endif
