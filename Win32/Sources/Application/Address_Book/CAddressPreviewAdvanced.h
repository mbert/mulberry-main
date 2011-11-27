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

#ifndef __CADDRESSPREVIEW__MULBERRY__
#define __CADDRESSPREVIEW__MULBERRY__

#include "CAddressPreviewBase.h"

#include "CAddressFieldContainer.h"
#include "CGrayBackground.h"
#include "CIconWnd.h"
#include "CToolbarButton.h"

// Classes
class CAdbkAddress;

class CAddressPreviewAdvanced : public CAddressPreviewBase
{
public:
					CAddressPreviewAdvanced();
	virtual 		~CAddressPreviewAdvanced();

	virtual void	Close();
	virtual void	Focus();

	void			SetAddress(CAdbkAddress* addr);
	void			ClearAddress();
	CAdbkAddress*	GetAddress() const
		{ return mAddress; }

protected:
	CGrayBackground			mHeader;				// Header for parts table
	CIconWnd				mIcon;					// Icon
	CStatic					mDescriptor;			// Caption details
	CToolbarButton			mZoom;
	CAddressFieldContainer	mFields;

	CAdbkAddress*			mAddress;

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnZoomPane(void);


	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog

	DECLARE_MESSAGE_MAP()
};

#endif
